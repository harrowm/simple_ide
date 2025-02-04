# simple_ide
A very simple IDE interface for the rosco_m68k using two atf22v10 GAL chips

With a lot of thanks to the following projects:
https://github.com/crmaykish/mackerel-68k/tree/master
https://github.com/markrvmurray/rosco-ide-ata

v1 hardware is in the v1 directory .. rough list of things to do in v2:  
/* TODO: I dont think we need A5 */
/* FC0-2 have to be in the other IC if we want to generate VPA there */
/* So one option is to bring in A4 then generate IDECS0/2 here */
/* move FC0-2 to the other IC */
/* Rotate the two GALs so they have the same roatation as the buffers */
/* Have managed to plug them in the wrong way around .. */
/* Spacing from IDE plug to edge of the board for right angle */
/* Note IC1 and IC2 on silkscreen where they can be seen with chip holders in place */
/* bring in DS */
/* fix connections for ~reset and ~idebuf that dont go anywhere !!! */
/* flag connections from rosco bus as not used when .. not used .. */
To build the rosco firmware with ATA enabled:

cd
git clone https://github.com/rosco-m68k/rosco_m68k.git
cd rosco_m68k/code/firmware/rosco_m68k_firmware
git submodule update --init --recursive
WITH_ATA=true ATA_DEBUG=true make clean all

# copy the ROM images to a SD Card named ROSCO32 on a Mac
cp rosco_m68k.rom /Volumes/ROSCO32 

.. mmm, had to edit the Makefiule to disable createint the rom filesystem
.. double mm, had to make the rom an even number of bytes using "truncate -s%2 rosco_m68k.rom"

# copy the update flash utility and rom images to a SD card named ROSCO32 on a Mac
cd
cd rosco_m68k/code/software
cp updateflash/updateflash.bin /Volumes/ROSCO32 