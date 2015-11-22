# BlueMSX-wii Installation instructions #

## Preparing your SD card ##

The BlueMSX-wii emulator can be started using the homebrew channel. For this purpose, the necessary files (main .dol file, icon.png and meta.xml) are released with this project. The best location for these files will be SD:/apps/bluemsx-wii/.

Except from the main .dol file, the BlueMSX-wii emulator needs some files on the SD-card.
Most of the files are embedded in the .dol file and will be placed on your SD-card when the emulator is first run. These files will be placed in the directory SD:/MSX/.

Additional to these files, there needs to be a gamepack. As distributing the rom files gamepack is not 100% legal, these will not be supplied through the google code project.

<a href='Hidden comment: TODO: supply info about structure of Gamepack<wiki:comment>

== Optional: BlueMSX-wii channel ==

Additional to starting the BlueMSX-wii emulator from the homebrew channel, it is possible (but not necessary) to install a BlueMSX-wii forwarder channel in the wii system menu. This channel can be installed using Waninkoko"s WAD Manager.

As this channel is a forwarder channel, it will automatically load SD:/apps/bluemsx-wii/boot.dol from the SD-card. This channel is optional, and if installed, it does not need to be re-installed everytime there is a new update of BlueMSX-wii.

*NOTE: Installing a custom WAD is dangerous and may brick your wii, we are not responsible for any damage you may do to your wii. U can use bootmii to make sure you can recover your wii!*