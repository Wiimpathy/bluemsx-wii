/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Wii/WiiSound.c,v $
**
** $Revision: 1.0 $
**
** $Date: 2008/12/07 00:00:00 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2008 Tim Brugman
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#include "ArchSound.h"
#include <stdlib.h>
#include <stdio.h>
#include <gccore.h>
#include <string.h>
#include <ogc/audio.h>
#include <ogc/cache.h>
#include <ogc/lwp.h>
#include <ogc/semaphore.h>

#define AUDIO_DEBUG 0

#define SAMPLE_SIZE 2
#define BUFFER_SIZE_BYTES (8*1024) /* 8 kB, average lag = 21ms */
#define BUFFER_SIZE_SAMPLES (BUFFER_SIZE_BYTES / SAMPLE_SIZE)
#define SAMPLERATE_IN  44100
#define SAMPLERATE_OUT 48000
#define TUNING_THRESHHOLD 100

typedef enum {
  AUDIOSTATE_UNINITIALIZED,
  AUDIOSTATE_PAUSED,
  AUDIOSTATE_RESUMING,
  AUDIOSTATE_PLAYING
} AUDIOSTATE;
  
static Int16 sample_buffer[BUFFER_SIZE_SAMPLES] __attribute__((aligned(32)));
static UInt32 buffer_offset = 0;

static AUDIOSTATE audio_state = AUDIOSTATE_UNINITIALIZED;
static Int32 tuning_sample_min, tuning_sample_max;
static Int32 samplerate_tuning_p = 0;
static Int32 samplerate_tuning_i = 0;
static Int32 samplerate_tuning = 0;
static Mixer *audio_mixer;
static Int32 resample_divider;
static Int16 prev_sample[2];
static Int32 audio_pause = 0;

static void soundDMACallback(void)
{
    // Restart DMA
    AUDIO_InitDMA((unsigned int)&sample_buffer, BUFFER_SIZE_BYTES);
    AUDIO_StartDMA();
}

static void soundClearBuffer(void)
{
    memset(sample_buffer, 0, sizeof(sample_buffer));
    DCFlushRange(sample_buffer, sizeof(sample_buffer));
}

static void StoreTuningSample(void)
{
    Int32 dma = BUFFER_SIZE_SAMPLES - (AUDIO_GetDMABytesLeft() / SAMPLE_SIZE);
    Int32 sample = dma - buffer_offset;
    if( sample < 0 ) {
        sample += BUFFER_SIZE_SAMPLES;
    }
    if( sample < tuning_sample_min ) {
        tuning_sample_min = sample;
    }
    if( sample > tuning_sample_max ) {
        tuning_sample_max = sample;
    }
}

void soundRestart(int time)
{
    if( audio_state == AUDIOSTATE_PLAYING ) {
        audio_pause = time;
        audio_state = AUDIOSTATE_RESUMING;
        soundClearBuffer();
    }
}

static void DoTuning(void)
{
    //       BUFFER_SIZE_SAMPLES   0
    // DMA output  ----------------|
    // Write ptr   ---|<-----|<-----
    //               max    min
    static Int32 prev_tuning = 0;
    Int32 percent_used = (tuning_sample_max - tuning_sample_min)*100/BUFFER_SIZE_SAMPLES;
    Int32 average = (tuning_sample_min + tuning_sample_max) / 2;
    Int32 buffer_tuning = average - BUFFER_SIZE_SAMPLES / 2;
    if( percent_used > 90 ) {
        prev_tuning = 0;
        samplerate_tuning_p = 0;
        soundRestart(10);
    }else{
        if( buffer_tuning <= -TUNING_THRESHHOLD ) {
            samplerate_tuning_p = (buffer_tuning + TUNING_THRESHHOLD) / 5;
            samplerate_tuning_i += (buffer_tuning + TUNING_THRESHHOLD) / 5;
        }else if( buffer_tuning >= TUNING_THRESHHOLD ) {
            samplerate_tuning_p = (buffer_tuning - TUNING_THRESHHOLD) / 5;
            samplerate_tuning_i += (buffer_tuning - TUNING_THRESHHOLD) / 5;
        }else{
            samplerate_tuning_p = 0; //buffer_tuning / 25;
        }
        samplerate_tuning_i += /*(buffer_tuning / 30) + */ buffer_tuning - buffer_tuning;
        samplerate_tuning = samplerate_tuning_p + (samplerate_tuning_i / 200);
        prev_tuning = buffer_tuning;
#if AUDIO_DEBUG
        printf("Audio: Tune %3d, %5d, %3d%%, %3d%%, %3d%%\n", samplerate_tuning, buffer_tuning,
               percent_used, tuning_sample_min*200/BUFFER_SIZE_SAMPLES,
               (BUFFER_SIZE_SAMPLES-tuning_sample_max)*200/BUFFER_SIZE_SAMPLES);
#endif
    }
    tuning_sample_min = BUFFER_SIZE_SAMPLES;
    tuning_sample_max = 0;
}

void soundCallibrate(void)
{
    if( audio_state == AUDIOSTATE_RESUMING ) {
        if( audio_pause ) {
            audio_pause--;
        }else{
#if AUDIO_DEBUG
            printf("Audio: Resuming\n");
#endif
            mixerSync(audio_mixer);
            resample_divider = 0;
            prev_sample[0] = 0;
            prev_sample[1] = 0;
            tuning_sample_min = BUFFER_SIZE_SAMPLES;
            tuning_sample_max = 0;
            buffer_offset = BUFFER_SIZE_SAMPLES - (AUDIO_GetDMABytesLeft() / SAMPLE_SIZE);
            buffer_offset += BUFFER_SIZE_SAMPLES / 4;
            if( buffer_offset > BUFFER_SIZE_SAMPLES ) {
                buffer_offset -= BUFFER_SIZE_SAMPLES;
            }
            audio_state = AUDIOSTATE_PLAYING;
        }
    }else
    if( audio_state == AUDIOSTATE_PLAYING ) {
        mixerSync(audio_mixer);
        DoTuning();
        StoreTuningSample();
    }
}

static Int32 soundWrite(void* dummy, Int16 *stream, UInt32 length)
{
    if( audio_state != AUDIOSTATE_PLAYING ) {
        return 0;
    }

    StoreTuningSample();

    // Copy to buffer
    // Includes resampling from 44k1 to 48kHz.
    while(length){
        Int32 samplerate_out = SAMPLERATE_OUT + samplerate_tuning;
        Int16 sample_in1 = *stream++;
        Int16 sample_in2 = *stream++;
        Int16 sample_out1, sample_out2;
        // Calculate samples
        sample_out1 = (resample_divider * sample_in1
                       + (samplerate_out - resample_divider) * prev_sample[0])
                      / samplerate_out;
        sample_out2 = (resample_divider * sample_in2
                       + (samplerate_out - resample_divider) * prev_sample[1])
                      / samplerate_out;
        // Store
        sample_buffer[buffer_offset++] = sample_out1;
        sample_buffer[buffer_offset++] = sample_out2;
        if( buffer_offset >= BUFFER_SIZE_SAMPLES ) {
            buffer_offset = 0;
        }
        // Increment resample divider
        resample_divider += SAMPLERATE_IN;
        // If still in samplerate_out range, need to produce another sample
        if( resample_divider < samplerate_out ) {
            // Calculate samples
            sample_out1 = (resample_divider * sample_in1
                           + (samplerate_out - resample_divider) * prev_sample[0])
                          / samplerate_out;
            sample_out2 = (resample_divider * sample_in2
                           + (samplerate_out - resample_divider) * prev_sample[1])
                          / samplerate_out;
            // Store
            sample_buffer[buffer_offset++] = sample_out1;
            sample_buffer[buffer_offset++] = sample_out2;
            if( buffer_offset >= BUFFER_SIZE_SAMPLES ) {
                buffer_offset = 0;
            }
            // Increment resample divider
            resample_divider += SAMPLERATE_IN;
        }
        resample_divider -= samplerate_out;
        prev_sample[0] = sample_in1;
        prev_sample[1] = sample_in2;
        length -= 2;
    }
    DCFlushRange(sample_buffer, sizeof(sample_buffer));

    StoreTuningSample();

    return 0;
}

void archSoundCreate(Mixer* mixer, UInt32 sampleRate, UInt32 bufferSize, Int16 channels)
{
    archSoundDestroy();
    // Check parameters
    if( sampleRate != SAMPLERATE_IN ) {
        fprintf(stderr, "Audio: Samplerate of %d not supported\n", sampleRate);
        exit(1);
    }
    if( channels != 2 ) {
        fprintf(stderr, "Audio: Only two channel stereo supported\n");
        exit(1);
    }
    // Init audio
    soundClearBuffer();
    AUDIO_Init(NULL);
    AUDIO_SetDSPSampleRate(AI_SAMPLERATE_48KHZ);
    AUDIO_RegisterDMACallback(soundDMACallback);
    // Initialize mixer
    audio_mixer = mixer;
    mixerSetStereo(mixer, channels == 2);
    mixerSetWriteCallback(mixer, soundWrite, NULL, 128);
#if AUDIO_DEBUG
    printf("Audio: Initialized\n");
#endif
    audio_state = AUDIOSTATE_PAUSED;
    // Start DMA
    AUDIO_InitDMA((unsigned int)&sample_buffer, BUFFER_SIZE_BYTES);
    AUDIO_StartDMA();
}

void archSoundDestroy(void)
{
    if( audio_state != AUDIOSTATE_UNINITIALIZED ) {
        audio_state = AUDIOSTATE_UNINITIALIZED;
        // De-register to mixer
        mixerSetWriteCallback(audio_mixer, NULL, NULL, 0);
        // Stop DMA
        AUDIO_StopDMA();
#if AUDIO_DEBUG
        printf("Audio: Destroyed\n");
#endif
    }
}

void archSoundSuspend(void)
{
    if( audio_state != AUDIOSTATE_UNINITIALIZED &&
        audio_state != AUDIOSTATE_PAUSED ) {
#if AUDIO_DEBUG
        printf("Audio: Pause\n");
#endif
        audio_state = AUDIOSTATE_PAUSED;
        soundClearBuffer();
    }
}

void archSoundResume(void)
{
    if( audio_state == AUDIOSTATE_PAUSED ) {
#if AUDIO_DEBUG
        printf("Audio: Prepare resume\n");
#endif
        audio_pause = 5;
        audio_state = AUDIOSTATE_RESUMING;
    }
}

