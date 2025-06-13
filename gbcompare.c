#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <direct.h>



// *** POKEBLUE *******************************************************************************
const char* ROM_FILE = "ao.gb";

const char* LAYOUT_FILE = "layout.link";

const char* INCLUDE_FILES[] = {
	"includes.asm"
};

char* MAIN_FILES[] = {
	"ram.asm",
	"text.asm",
	"gfx/pics.asm",
	"gfx/sprites.asm",
	"gfx/tilesets.asm",
	"audio.asm",
	"home.asm",
	"main.asm",
	"maps.asm",
};

char* DEF_SYMBOLS[] = { "_BLUE", 0 };
// *** POKEBLUE *******************************************************************************

bool debug = false;





FILE* asmfile;
char* asmpath;
bool asmfilebackedup = false;
unsigned int linenumber;

unsigned long mempos;

FILE* romfile;



#include "Symbols.c"
#include "Constants.c"
#include "TokenIdentifier.c"
#include "Errors.c"
#include "ExpressionProcessor.c"
#include "StatementProcessor.c"
#include "LayoutHandler.c"



int optionRegion(int argc, char* argv[]) {
	if(argc > 1) {
		// option to process a region before the others
		enum asmregiontype region = identifyRegiontype(argv[1]);
		unsigned char bank = 0;
		if(region == INVALID_REGION_TYPE) {
			unsigned int i;
			
			if(region == INVALID_REGION_TYPE) {
				const char romx[] = "ROM";
				region = ROMX;
				for(i = 0; i < strlen(romx); i++) if(argv[1][i] != romx[i] && argv[1][i] != romx[i] + 'a'-'A') {
					region = INVALID_REGION_TYPE;
					break;
				}
			}
			
			if(region == INVALID_REGION_TYPE) {
				const char wramx[] = "WRAM";
				region = WRAMX;
				for(i = 0; i < strlen(wramx); i++) if(argv[1][i] != wramx[i] && argv[1][i] != wramx[i] + 'a'-'A') {
					region = INVALID_REGION_TYPE;
					break;
				}
			}
			
			if(region == INVALID_REGION_TYPE) {
				const char sramx[] = "SRAM";
				region = SRAM;
				for(i = 0; i < strlen(sramx); i++) if(argv[1][i] != sramx[i] && argv[1][i] != sramx[i] + 'a'-'A') {
					region = INVALID_REGION_TYPE;
					break;
				}
			}
			
			if(region != INVALID_REGION_TYPE) {
				while(isHexadecimal(argv[1][i])) {
					bank *= 0x10;
					if(isNumber(argv[1][i])) bank += argv[1][i] - '0';
					else if(isUppercase(argv[1][i])) bank += argv[1][i] - 'A'+10;
					else if(isLowercase(argv[1][i])) bank += argv[1][i] - 'a'+10;
					i++;
				}
			}
		}
		
		if(region != INVALID_REGION_TYPE) {
			FILE* layoutfile = fopen(LAYOUT_FILE, "rb");
			if(layoutfile == NULL) {
				errorCannotFindFile(LAYOUT_FILE);
				return -1;
			}
			
			char* mainfiles[sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0]) + 1];
			for(unsigned int i = 0; i < sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0]); i++) mainfiles[i] = MAIN_FILES[i];
			mainfiles[sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0])] = 0;
			
			if(processRegion(layoutfile, region, bank, mainfiles) != 0) return -1;
		}
	}
}



int main(int argc, char* argv[]) {
	currentsection[0] = '\0';
	currentsection[1] = '\0';
	
	labellist[0] = malloc(1024 * sizeof(labellist[0][0]));
	assumptionlist[0] = malloc(1024 * sizeof(assumptionlist[0][0]));
	macrolist[0] = malloc(1024 * sizeof(macrolist[0][0]));
	constantlist[0] = malloc(1024 * sizeof(constantlist[0][0]));
	conststrlist[0] = malloc(1024 * sizeof(conststrlist[0][0]));
	variablelist[0] = malloc(1024 * sizeof(variablelist[0][0]));
	charmaplist[0] = malloc(1024 * sizeof(charmaplist[0][0]));
	
	unsigned char* topexpandedstrs[64];
	expandedstrs = topexpandedstrs;
	unsigned int topexpandedstrspos[64];
	expandedstrspos = topexpandedstrspos;
	
	
	
	for(unsigned int i = 0; DEF_SYMBOLS[i] != 0; i++) {
		consts(constantcount).name = DEF_SYMBOLS[i];
		incrementConstantlist;
	}
	
	
	
	mempos = 0;
	
	// open rom
	romfile = fopen(ROM_FILE, "rb");
	if(romfile == NULL) {
		errorCannotFindFile(ROM_FILE);
		return -1;
	}
	
	
	
	// includes
	for(unsigned int i = 0; i < sizeof(INCLUDE_FILES)/sizeof(INCLUDE_FILES[0]); i++) {
		// open
		asmfile = fopen(INCLUDE_FILES[i], "rb");
		char newasmpath[strlen(INCLUDE_FILES[i])+1];
		strcpy(newasmpath, INCLUDE_FILES[i]);
		asmpath = newasmpath;
		if(asmfile == NULL) {
			errorCannotFindFile(INCLUDE_FILES[i]);
			return -1;
		}
		linenumber = 1;
		
		// process
		int statementresult = 0;
		while(statementresult == 0) {
			statementresult = processNextStatement();
		}
		if(statementresult != 1) return statementresult;
		
		// close
		fclose(asmfile);
	}
	
	printf("\nProcessed include files\n");
	
	
	
	localmacrostart = macrocount;
	localvariablestart = variablecount;
	localconstantstart = constantcount;
	localconststrstart = conststrcount;
	
	
	// layout
	FILE* layoutfile = fopen(LAYOUT_FILE, "rb");
	if(layoutfile == NULL) {
		errorCannotFindFile(LAYOUT_FILE);
		return -1;
	}
	
	// main files
	char* mainfiles[sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0]) + 1];
	for(unsigned int i = 0; i < sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0]); i++) mainfiles[i] = MAIN_FILES[i];
	mainfiles[sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0])] = 0;
	
	
	
	
	// *** LAYOUT *****************************************************************************
	
	// ram
	mainfiles[0] = "ram.asm";
	if(processRegion(layoutfile, WRAM0,   0, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, VRAM,    0, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, SRAM, 0x00, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, SRAM, 0x01, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, SRAM, 0x02, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, SRAM, 0x03, mainfiles) != 0) return -1;
	mainfiles[1] = "main.asm";
	if(processRegion(layoutfile, HRAM,    0, mainfiles) != 0) return -1;
	
	
	// home
	mainfiles[0] = "home.asm";
	if(processRegion(layoutfile, ROM0,    0, mainfiles) != 0) return -1;
	
	
	// optional preemptive region
	if(optionRegion(argc, argv) != 0) return -1;
	
	
	// rom
	mainfiles[0] = "main.asm";
	mainfiles[1] = 0;
	if(processRegion(layoutfile, ROMX, 0x01, mainfiles) != 0) return -1;
	mainfiles[0] = "audio.asm";
	if(processRegion(layoutfile, ROMX, 0x02, mainfiles) != 0) return -1;
	mainfiles[0] = "main.asm";
	if(processRegion(layoutfile, ROMX, 0x03, mainfiles) != 0) return -1;
	mainfiles[1] = "gfx/sprites.asm";
	mainfiles[2] = "text.asm";
	mainfiles[3] = 0;
	if(processRegion(layoutfile, ROMX, 0x04, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x05, mainfiles) != 0) return -1;
	mainfiles[0] = "maps.asm";
	mainfiles[1] = "main.asm";
	mainfiles[2] = 0;
	if(processRegion(layoutfile, ROMX, 0x06, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x07, mainfiles) != 0) return -1;
	mainfiles[0] = "audio.asm";
	if(processRegion(layoutfile, ROMX, 0x08, mainfiles) != 0) return -1;
	mainfiles[0] = "gfx/pics.asm";
	if(processRegion(layoutfile, ROMX, 0x09, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x0A, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x0B, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x0C, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x0D, mainfiles) != 0) return -1;
	mainfiles[0] = "main.asm";
	mainfiles[1] = 0;
	if(processRegion(layoutfile, ROMX, 0x0E, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x0F, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x10, mainfiles) != 0) return -1;
	mainfiles[0] = "maps.asm";
	mainfiles[1] = "main.asm";
	if(processRegion(layoutfile, ROMX, 0x11, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x12, mainfiles) != 0) return -1;
	mainfiles[0] = "gfx/pics.asm";
	mainfiles[1] = "maps.asm";
	mainfiles[2] = "main.asm";
	if(processRegion(layoutfile, ROMX, 0x13, mainfiles) != 0) return -1;
	mainfiles[0] = "maps.asm";
	mainfiles[1] = "main.asm";
	mainfiles[2] = 0;
	if(processRegion(layoutfile, ROMX, 0x14, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x15, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x16, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x17, mainfiles) != 0) return -1;
	if(processRegion(layoutfile, ROMX, 0x18, mainfiles) != 0) return -1;
	mainfiles[0] = "gfx/tilesets.asm";
	mainfiles[1] = 0;
	if(processRegion(layoutfile, ROMX, 0x19, mainfiles) != 0) return -1;
	mainfiles[1] = "main.asm";
	if(processRegion(layoutfile, ROMX, 0x1A, mainfiles) != 0) return -1;
	mainfiles[1] = 0;
	if(processRegion(layoutfile, ROMX, 0x1B, mainfiles) != 0) return -1;
	mainfiles[0] = "main.asm";
	if(processRegion(layoutfile, ROMX, 0x1C, mainfiles) != 0) return -1;
	mainfiles[0] = "maps.asm";
	mainfiles[1] = "main.asm";
	if(processRegion(layoutfile, ROMX, 0x1D, mainfiles) != 0) return -1;
	mainfiles[0] = "main.asm";
	mainfiles[1] = 0;
	if(processRegion(layoutfile, ROMX, 0x1E, mainfiles) != 0) return -1;
	mainfiles[0] = "audio.asm";
	if(processRegion(layoutfile, ROMX, 0x1F, mainfiles) != 0) return -1;
	
	// *** LAYOUT *****************************************************************************
	
	
	
	
	
	// success!
	char projectname[256];
	getcwd(projectname, 256);
	unsigned int shortnameindex;
	for(unsigned int i = 0; projectname[i] != '\0'; i++) if(projectname[i] == '\\' || projectname[i] == '/') shortnameindex = i + 1;
	strcpy(projectname, &projectname[shortnameindex]);
	printf("\nSuccessfully validated %s with %s\n", projectname, ROM_FILE);
	
	return 0;
}