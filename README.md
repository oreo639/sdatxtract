# SDATxtract
[![Build Status](https://travis-ci.com/Oreo639/sdatxtract.svg?branch=master)](https://travis-ci.com/Oreo639/sdatxtract)

A comand line Nintendo ds Sound DATa extraction utility.

What is it?

It is a utility for dumping sdats from nds files, extracting contained files from sdats, and converting the contained files to standard formats.

## Building
You can use either cmake or autotools to build this program and I will cover both.
### Autotools
1. Run `./autogen.sh`. This will automaticly run all the nessary auto* tools that are needed to compile this program.
2. Type `make`
### Cmake
1. Run `mkdir build && cd build`
2. Then run `cmake ..`
3. Once it is done, type `make`

## TODO
Add SWAR extraction and swav conversion. Finish nsseq and use that instead of sseq2mid. More testing. This is unfinished and subject to change. If I used your code without crediting you, please let me know either by relevant social media, or by the issue tracker.

## Credits
Vgmtrans: Symb parser prety much riped from them, with minor changes being made to support this codebase ofc.

Loveemu: Sseq2mid and strm2wav are used in this project.

Ndssndext: Largely inspired by ndssndext, however do to the aplication being unlisenced, this application was rewriten.

SDAT documentation: Without the hard work of:
+ Crystal - the author of CrystalTile2.exe 
+ loveemu - the author of sseq2mid.exe, swave2wave.exe & strm2wave.exe
+ Nintendon - the author of ndssndext.exe
+ DJ Bouche - the author of sdattool.exe
+ VGMTrans - the author of VGMTrans.exe
+ And others on documenting the format. I would never have been able to finish this without them.

You can find them here:
+ http://www.feshrine.net/hacking/doc/nds-sdat.html
+ http://www.romhacking.net/documents/%5b469%5dnds_formats.htm#SDAT

## Copyright
Read LICENSE.md for information.
