# Introduction #

Compiling your own version of BlueMSX-wii will require a few easy steps.
  1. [Install\_devkitPro](Install_devkitPro.md)
  1. [Download\_source](Download_source.md)
  1. [Open\_and\_build](Open_and_build.md)

## Install devkitPro ##

If you have already installed devkitPro, continue to Compile\_project

Go to http://www.devkitpro.org/ and download the automated windows installer (devkitProUpdater).
Run the application and install devkitPPC.
It is possible to disable devkitARM and devkitPSP during installation. These components are not needed.

## Download source ##

Get your favorite SVN client (TortoiseSVN for example) and get the contents of this SVN repository.

## Open and build ##

Start Programmers Notepad (included in devkitPro) and open project.pnproj in the root of the bluemsx-wii project folder.
From the menu, choose Tools|clean and Tools|make
Wait a while, and your project should be compiled.

**Note: Due to bugs in devkitPPC and libogc (because they are in development too), we have decided to include a 'working' set of include files and libraries in our project**