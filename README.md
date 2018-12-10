# SDATxtract
A comand line Nintendo ds Sound DATa extraction utility.

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

## Copyright
Read LICENSE.md for information.
