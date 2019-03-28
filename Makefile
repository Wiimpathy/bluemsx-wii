#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#platform=WINDOWS
ifndef platform
	platform = UNIX
endif

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	boot
BUILD		:=	build
SOURCES		:=	source/Bios \
              source/Board \
              source/Debugger \
              source/Emulator \
              source/Expat \
              source/Gui \
              source/Input \
              source/IoDevice \
              source/Language \
              source/Media \
              source/Memory \
              source/Resource \
              source/SoundChips \
              source/TinyXML \
              source/Unzip \
              source/Utils \
              source/VideoChips \
              source/VideoRender \
              source/Wii \
              source/WiiSprite \
              source/WiiUsbKeyboard \
              source/Z80

DATA		:=	data
INCLUDES	:=	include/libpng \
              source/Arch \
              source/Board \
              source/Common \
              source/Debugger \
              source/Emulator \
              source/Expat \
              source/Gui \
              source/Input \
              source/IoDevice \
              source/Language \
              source/Media \
              source/Memory \
              source/Resource \
              source/SoundChips \
              source/TinyXML \
              source/Unzip \
              source/Utils \
              source/VideoChips \
              source/VideoRender \
              source/Wii \
              source/WiiSprite \
              source/WiiUsbKeyboard \
              source/Z80

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS	= -g -O2 -Wall $(MACHDEP) $(INCLUDE) -DNO_ASM -DWII -DDEVKITPPC_STDLIB_INCLUDE=\"$(DEVKITPPC)/powerpc-eabi/include/stdlib.h\"
CXXFLAGS	=	-Wno-narrowing $(CFLAGS)

LDFLAGS	=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS	:=	 -ldb -lfreetype -lpng -lz -lwiiuse -lbte -lfat -logc -lm -lmad

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

export GENFILES	:=	 sdcard.inc gamepack.inc $(PNGFILES:.png=.inc) $(TTFFILES:.ttf=.inc)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD) \
			-I$(LIBOGC_INC) -I$(LIBOGC_INC)/ogc -I$(PORTLIBS)/include

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib/wii) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
ifeq ($(platform),UNIX)
	@cd util && gcc raw2c.c -o raw2c
endif
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile -j6

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
ifeq ($(platform),UNIX)
	@rm -fr util/raw2c
endif
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

#---------------------------------------------------------------------------------
run:
	wiiload $(TARGET).dol

#---------------------------------------------------------------------------------
disasm:
	@echo Disassembling ...
	@$(DEVKITPPC)/bin/powerpc-eabi-objdump -S $(TARGET).elf >$(TARGET).txt

#---------------------------------------------------------------------------------
else

DEPENDS	:=	revision $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
blueMSXwii: $(GENFILES) $(OUTPUT).dol
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# SVN revision
#---------------------------------------------------------------------------------
revision:
#	@cmd /c ..\\util\\revision.bat

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o	:	%.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
# This rule creates the zip files for the sd-card contents and converts it to a .h
#---------------------------------------------------------------------------------
sdcard.inc: ../sdcard/MSX
	@echo Creating sdcard.zip ...
	@rm -f sdcard.zip
ifeq ($(platform),UNIX)
	7za a -r -xr!*.svn -xr!thumbs.* sdcard.zip ../sdcard/MSX
else
	@../util/7za a -r -xr!*.svn -xr!thumbs.* sdcard.zip ../sdcard/MSX
endif
	@echo Converting sdcard.zip to sdcard.inc ...
	@../util/raw2c sdcard.zip sdcard.inc sdcard

gamepack.inc: ../sdcard/Gamepack
	@echo Creating gamepack.zip ...
	@rm -f gamepack.zip
ifeq ($(platform),UNIX)
	7za a -r -xr!*.svn -xr!thumbs.* gamepack.zip ../sdcard/Gamepack/Games
else
	@../util/7za a -r -xr!*.svn -xr!thumbs.* gamepack.zip ../sdcard/Gamepack/Games
endif
	@echo Converting gamepack.zip to gamepack.inc ...
	@../util/raw2c gamepack.zip gamepack.inc gamepack

#---------------------------------------------------------------------------------
# This rule converts .png to .inc files
#---------------------------------------------------------------------------------
%.inc: %.png
	@echo Converting $(notdir $<) to $(notdir $@) ...
	@$(CURDIR)/../util/raw2c $< $(CURDIR)/$@ $(notdir $@)

#---------------------------------------------------------------------------------
# This rule converts .ttf to .inc files
#---------------------------------------------------------------------------------
%.inc: %.ttf
	@echo Converting $(notdir $<) to $(notdir $@) ...
	@../util/raw2c $< $(CURDIR)/$@ $(notdir $@)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
