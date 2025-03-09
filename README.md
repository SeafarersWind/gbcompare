# gbcompare
This project compares the assembly code of [pokered](https://github.com/pret/pokered) to the ROM data of the Japanese versions of Pokémon Red, Green, and Blue for the purpose of creating disassemblies for them. It is meant to read pokered's code accurately, so will not necessarily be able to read all projects written in [RGBASM](https://rgbds.gbdev.io/).

This project is a work in progress. It should be close to completion, but there are some parts I am considering reworking to cleaner designs, so it will have a while more to go.

When gbcompare.exe is run in the command line, it processes includes.asm, reads layout.link and searches the main files for the sections in WRAM0 and VRAM.