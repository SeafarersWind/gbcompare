# gbcompare
This program compares [RGBASM](https://rgbds.gbdev.io/) code with ROM data. Its purpose is to assist in the disassembly of GameBoy games by using the disassembly of a similar version of that game. For instance, it was used to develop [pokegreen](https://github.com/SeafarersWind/pokegreen) and [pokeblue](https://github.com/SeafarersWind/pokeblue) by using [pokered](https://github.com/pret/pokered) as a base.

This program performs the following tasks:
- Reports most errors in the code and any inconsistencies with the ROM data.
- Records labels, macros, variables, and constants, text constants, and charmaps.
- Assumes the value of undefined symbols (e.g. a label that is referenced before the region that it is defined in is processed).
- Suggests corrections based on the ROM data and the record of symbols.
- Automatically replaces `@`-terminated `db` strings, `text_far` macros, and `li` macros with the correct text. (based on pokered)
- Automatically corrects inaccurate `hlcoord` macros. (based on pokered)

---

This program is currently configured to Pokeblue. To configure gbcompare to a different project, it can be done manually in two locations of **gbcompare.c**:
- The area near the start of gbcompare.c currently marked `POKEBLUE` defines the relevant files and predefined symbols.
- The area near the end of gbcompare.c marked `LAYOUT` defines the regions to be processed and their order.

To make gbcompare process a region before the others without configuring `LAYOUT`, name a region (e.g. `SRAM2`, `ROM1E`, `VRAM`) as an option at the command line.

I developed this program for my personal use, but if anyone who wants to use it for their own projects runs into issues, feel free to contact me at seafarerswind@proton.me.
