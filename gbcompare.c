#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <direct.h>



// POKERED-JP-SPECIFIC
const char* INCLUDE_FILES[] = {
	"includes.asm"
};

const char* MAIN_FILES[] = {
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

const char* LAYOUT_FILE = "layout.link";

const char* ROM_FILE = "midori.gb";
// POKERED-JP-SPECIFIC

bool debug = false;



#define isWhitespace(c) (c==' ' || c=='\t' || c=='\v' || c=='\r')
#define isAlpha(c) ((c>='a' && c<= 'z') || (c>='A' && c<='Z'))
#define isUppercase(c) (c>='A' && c<='Z')
#define isLowercase(c) (c>='a' && c<='z')
#define isNumber(c) (c>='0' && c<='9')
#define isHexadecimal(c) (isNumber(c) || (c>='a' && c<='f') || (c>='A' && c<='F'))
#define isBinary(c) (c == '0' || c == '1')
#define isNum(c) (isNumber(c) || c=='_')
#define isHex(c) (isHexadecimal(c) || c=='_')
#define isBin(c) (isBinary(c) || c=='_')
#define isOct(c) ((c>='0' && c<='7') || c=='_')
#define isQua(c) ((c>='0' && c<='3') || c=='_')
#define isAlphanumeric(c) (isAlpha(c) || isNumber(c))
#define isValidSymbolFirstChar(c) (isAlpha(c) || c=='_' || c=='.')
#define isValidSymbolChar(c) (isAlphanumeric(c) || c=='_' || c=='#' || c=='@' || c=='.')

enum asmtoken {
	OPCODE, DIRECTIVE, PREDECLARED_SYMBOL, REGISTER, CONDITION, NUMBER, STRING, OPERATOR, COMMA, MEMORY_OPEN, MEMORY_CLOSE,
	CONSTANT, CONSTANT_STRING, VARIABLE, MACRO, LABEL, ASSUMPTION, UNRECORDED_SYMBOL, TEXT_MACRO, NEWLINE, REGION_TYPE, END_OF_FILE, UNKNOWN_TOKEN
};
const char* TOKENS[] = {
	"opcode", "directive", "predefined symbol", "register", "condition", "number", "string", "operator", ",", "[", "]",
	"constant", "constant string", "variable", "macro", "label", "assumption", "unrecorded symbol", "text macro", "newline", "region type", "end of file", "?????"
};
#define TOKENS_COUNT sizeof(TOKENS)/sizeof(TOKENS[0])

struct token {
	enum asmtoken type;
	unsigned int content;
};

struct section {
	char* name;
	char* path;
	long filepos;
};

struct label {
	char* name;
	long value;
};

struct assumption {
	char* name;
	unsigned char valuel;
	unsigned char valuem;
	unsigned char valueh;
	bool l;
	bool m;
	bool h;
};

struct macro {
	char* name;
	char* content;
};

struct variable {
	char* name;
	long value;
};

struct constant {
	char* name;
	long value;
};

struct constantstring {
	char* name;
	char* content;
};

struct mapchar {
	char* chars;
	long value;
};

enum asmopcode {
	NOP,
	LD, LDH, LDI, LDD,
	INC, DEC, ADD, ADC, SUB, SBC,
	SCF, OR, AND, XOR, CPL, CCF, CP,
	RRA, RRCA, RLA, RLCA,
	JP, JR, CALL, RST, RET, RETI,
	PUSH, POP,
	EI, DI, DAA, STOP, HALT,
	RLC, RRC, RL, RR, SLA, SRA, SWAP, SRL, BIT, RES, SET,
	INVALID_OPCODE
};
const char* OPCODES[] = {
	"nop",
	"ld", "ldh", "ldi", "ldd",
	"inc", "dec", "add", "adc", "sub", "sbc",
	"scf", "or", "and", "xor", "cpl", "ccf", "cp",
	"rra", "rrca", "rla", "rlca",
	"jp", "jr", "call", "rst", "ret", "reti",
	"push", "pop",
	"ei", "di", "daa", "stop", "halt",
	"rlc", "rrc", "rl", "rr", "sla", "sra", "swap", "srl", "bit", "res", "set"
};
#define OPCODES_COUNT sizeof(OPCODES)/sizeof(OPCODES[0])

enum asmarg {
	NO,
	A, B, C, D, E, H, L, AF, BC, DE, HL, SP, SPe8,
	n8, n16, e8,
	u0, u1, u2, u3, u4, u5, u6, u7,
	v00, v08, v10, v18, v20, v28, v30, v38,
	m_C, m_BC, m_DE, m_HL, m_HLI, m_HLD, m_a8, m_a16,
	ccZ, ccNZ, ccC, ccNC,
	INVALID_ARG
};
const char* OPCODEARGS[] = {
	"",
	"a", "b", "c", "d", "e", "h", "l", "af", "bc", "de", "hl", "sp", "sp + e8",
	"n8", "n16", "e8",
	"0", "1", "2", "3", "4", "5", "6", "7",
	"$00", "$08", "$10", "$18", "$20", "$28", "30", "$38",
	"[c]", "[bc]", "[de]", "[hl]", "[hli]", "[hld]", "[a8]", "[a16]",
	"z", "nz", "c", "nc"
};
#define OPCODEARGS_COUNT sizeof(OPCODEARGS)/sizeof(OPCODEARGS[0])

enum asmdirective {
	DIR_DB, DIR_DW, DIR_DL, DIR_DS, DIR_RSRESET, DIR_RSSET, DIR_RB, DIR_RW, DIR_RL, DIR_CHARMAP,
	DIR_BANK, DIR_HIGH, DIR_LOW, DIR_SIZEOF, DIR_STARTOF,
	DIR_STRLEN, DIR_STRCAT, DIR_STRCMP, DIR_STRIN, DIR_STRRIN, DIR_STRFIND, DIR_STRSUB, DIR_STRUPR, DIR_STRLWR, DIR_STRRPL, DIR_STRFMT, DIR_INCHARMAP, DIR_CHARLEN, DIR_CHARSUB,
	DIR_DEF, DIR_REDEF, DIR_EQU, DIR_EQUS, DIR_PURGE, DIR_EXPORT,
	DIR_IF, DIR_ELIF, DIR_ELSE, DIR_ENDC, DIR_REPT, DIR_FOR, DIR_ENDR,
	DIR_MACRO, DIR_ENDM, DIR_SHIFT,
	DIR_INCLUDE, DIR_INCBIN,
	DIR_SECTION, DIR_ENDSECTION, DIR_LOAD, DIR_ENDL, DIR_UNION, DIR_NEXTU, DIR_ENDU,
	DIR_FAIL, DIR_WARN, DIR_ASSERT,
	DIR_PUSHO, DIR_POPO, DIR_OPT,
	INVALID_DIRECTIVE
};
const char* DIRECTIVES[] = {
	"DB", "DW", "DL", "DS", "RSRESET", "RSSET", "RB", "RW", "RL", "CHARMAP",
	"BANK", "HIGH", "LOW", "SIZEOF", "STARTOF",
	"STRLEN", "STRCAT", "STRCMP", "STRIN", "STRRIN", "STRFIND", "STRSUB", "STRUPR", "STRLWR", "STRRPL", "STRFMT", "INCHARMAP", "CHARLEN", "CHARSUB",
	"DEF", "REDEF", "EQU", "EQUS", "PURGE", "EXPORT",
	"IF", "ELIF", "ELSE", "ENDC", "REPT", "FOR", "ENDR",
	"MACRO", "ENDM", "SHIFT",
	"INCLUDE", "INCBIN",
	"SECTION", "ENDSECTION", "LOAD", "ENDL", "UNION", "NEXTU", "ENDU",
	"FAIL", "WARN", "ASSERT",
	"PUSHO", "POPO", "OPT",
};
#define DIRECTIVES_COUNT sizeof(DIRECTIVES)/sizeof(DIRECTIVES[0])

enum asmregiontype {
	ROM0, ROMX, VRAM, SRAM, WRAM0, WRAMX, OAM, HRAM, INVALID_REGION_TYPE
};
const char* REGIONS[] = {
	"ROM0", "ROMX", "VRAM", "SRAM", "WRAM0", "WRAMX", "OAM", "HRAM"
};
#define REGIONS_COUNT sizeof(REGIONS)/sizeof(REGIONS[0])
const unsigned long REGION_ADDRESSES[] = {
	0x0000, 0x4000, 0x8000, 0xA000, 0xC000, 0xC000, 0xFE00, 0xFF80
};
const unsigned long REGION_SIZES[] = {
	0x4000, 0x4000, 0x2000, 0x2000, 0x2000, 0x2000, 0x00A0, 0x007F
};

enum asmpredef {
	NARG, RS, INVALID_PREDEF
};
const char* PREDEFS[] = {
	"_NARG", "_RS"
};
#define PREDEFS_COUNT sizeof(PREDEFS)/sizeof(PREDEFS[0])

enum asmregister {
	REG_A, REG_B, REG_C, REG_D, REG_E, REG_H, REG_L,
	REG_AF, REG_BC, REG_DE, REG_HL, REG_SP, REG_HLI, REG_HLD,
	INVALID_REGISTER
};
const char* REGISTERS[] = {
	"a", "b", "c", "d", "e", "h", "l", "af", "bc", "de", "hl", "sp", "hli", "hld"
};
#define REGISTERS_COUNT sizeof(REGISTERS)/sizeof(REGISTERS[0])

enum asmcondition {
	CC_Z, CC_NZ, CC_C, CC_NC, INVALID_CONDITION
};
const char* CCS[] = {
	"z", "nz", "c", "nc"
};
#define CCS_COUNT sizeof(CCS)/sizeof(CCS[0])

enum asmoperator {
	OPEN_PARENTHESIS, CLOSE_PARENTHESIS,
	EXPONENT,
	COMPLEMENT, BOOLEAN_NOT,
	MULTIPLY, DIVIDE, MODULO,
	SHIFT_LEFT, SHIFT_RIGHT, UNSIGNED_SHIFT_RIGHT, BINARY_AND, BINARY_OR, BINARY_XOR,
	OPERATION_ADD, SUBTRACT, 
	EQUAL_TO, NOT_EQUAL_TO, LESS_THAN_OR_EQUAL_TO, GREATER_THAN_OR_EQUAL_TO, LESS_THAN, GREATER_THAN, BOOLEAN_AND, BOOLEAN_OR,
	EQUALS, PLUS_EQUALS, MINUS_EQUALS, MULTIPLY_EQUALS, DIVIDE_EQUALS, MODULO_EQUALS, SHIFT_LEFT_EQUALS, SHIFT_RIGHT_EQUALS, AND_EQUALS, OR_EQUALS, XOR_EQUALS,
	END_OF_EXPRESSION
};
const char* OPERATORS[] = {
	"(", ")",
	"**",
	"~", "!",
	"*", "/", "%",
	"<<", ">>", ">>>", "&", "|", "^",
	"+", "-",
	"==", "!=", "<=", ">=", "<", ">", "&&", "||",
	"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "|=", "^=",
	"END"
};
#define OPERATORS_COUNT sizeof(OPERATORS)/sizeof(OPERATORS[0])

const unsigned int OPERATOR_PRIORITY[] = {
	13, 0,
	12,
	11, 11,
	10, 10, 10,
	9, 8, 7, 6, 6, 6,
	5, 5,
	4, 4, 4, 4, 4, 4, 3, 3,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1
};
#define OPERATOR_PRIORITY_COUNT sizeof(OPERATOR_PRIORITY)/sizeof(OPERATOR_PRIORITY[0])

enum textcommand {
	TEXT, NEXT, LINE, PARA, CONT, DONE, PROMPT, PAGE, DEX,
	TEXT_START, TEXT_RAM, TEXT_BCD, TEXT_MOVE, TEXT_BOX, TEXT_LOW,
	TEXT_PROMPTBUTTON, TEXT_SCROLL, TEXT_ASM, TEXT_DECIMAL, TEXT_PAUSE,
	SOUND_GET_ITEM_1, SOUND_LEVEL_UP, TEXT_DOTS, TEXT_WAITBUTTON,
	SOUND_POKEDEX_RATING, SOUND_GET_ITEM_1_DUPLICATE, SOUND_GET_ITEM_2,
	SOUND_GET_KEY_ITEM, SOUND_CAUGHT_MON, SOUND_DEX_PAGE_ADDED, SOUND_CRY_NIDORINA,
	SOUND_CRY_PIDGEOT, SOUND_CRY_DEWGONG, TEXT_FAR,
	TEXT_END,
	TX_DB
};
const char* TEXT_COMMANDS[] = {
	"text", "next", "line", "para", "cont", "done", "prompt", "page", "dex",
	"text_start", "text_ram", "text_bcd", "text_move", "text_box", "text_low",
	"text_promptbutton", "text_scroll", "text_asm", "text_decimal", "text_pause",
	"sound_get_item_1", "sound_level_up", "text_dots", "text_waitbutton",
	"sound_pokedex_rating", "sound_get_item_1_duplicate", "sound_get_item_2",
	"sound_get_key_item", "sound_caught_mon", "sound_dex_page_added", "sound_cry_nidorina",
	"sound_cry_pidgeot", "sound_cry_dewgong", "text_far",
	"text_end",
	"db  "
};
const unsigned int TEXT_COMMAND_IDS[] = {
	0x00, 0x4E, 0x4F, 0x51, 0x55, 0x57, 0x58, 0x49, 0x5F,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
	0x06, 0x07, 0x08, 0x09, 0x0A,
	0x0B, 0x0B, 0x0C, 0x0D,
	0x0E, 0x0F, 0x10,
	0x11, 0x12, 0x13, 0x14,
	0x15, 0x16, 0x17,
	0x50,
	0
};
const unsigned int TEXT_COMMAND_SIZES[] = {
	0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02,
	0x01, 0x03, 0x04, 0x03, 0x05, 0x01,
	0x01, 0x01, 0x01, 0x04, 0x01,
	0x01, 0x01, 0x02, 0x01,
	0x01, 0x01, 0x01,
	0x01, 0x01, 0x01, 0x01,
	0x01, 0x01, 0x04,
	0x01,
	0x01, 0x03, 0x01, 0x01,
	0x01, 0x01, 0x01,
	0x01,
	0
};
#define TEXT_COMMAND_COUNT (sizeof(TEXT_COMMANDS)/sizeof(TEXT_COMMANDS[0]) - 1)
#define newTextcommand(i) (i >= TEXT && i <= DEX)
#define terminatingTextcommand(i) (i == DONE || i == PROMPT || i == DEX || i == TEXT_END || i == TEXT_ASM)





struct instruction {
	enum asmopcode opcode;
	enum asmarg arg1;
	enum asmarg arg2;
	unsigned char byte;
};

const struct instruction INSTRUCTIONS[] = {
	{ NOP,  NO,  NO,  0x00 },
	{ LD,   BC,  n16, 0x01 },
	{ LD, m_BC,  A,   0x02 },
	{ INC,  BC,  NO,  0x03 },
	{ INC,  B,   NO,  0x04 },
	{ DEC,  B,   NO,  0x05 },
	{ LD,   B,   n8,  0x06 },
	{ RLCA, NO,  NO,  0x07 },
	{ LD, m_a16, SP,  0x08 },
	{ ADD,  HL,  BC,  0x09 },
	{ LD,   A, m_BC,  0x0A },
	{ DEC,  BC,  NO,  0x0B },
	{ INC,  C,   NO,  0x0C },
	{ DEC,  C,   NO,  0x0D },
	{ LD,   C,   n8,  0x0E },
	{ RRCA, NO,  NO,  0x0F },
	
	{ STOP, NO,  NO,  0x10 },
	{ LD,   DE,  n16, 0x11 },
	{ LD, m_DE,  A,   0x12 },
	{ INC,  DE,  NO,  0x13 },
	{ INC,  D,   NO,  0x14 },
	{ DEC,  D,   NO,  0x15 },
	{ LD,   D,   n8,  0x16 },
	{ RLA,  NO,  NO,  0x17 },
	{ JR,   e8,  NO,  0x18 },
	{ ADD,  HL,  DE,  0x19 },
	{ LD,   A, m_DE,  0x1A },
	{ DEC,  DE,  NO,  0x1B },
	{ INC,  E,   NO,  0x1C },
	{ DEC,  E,   NO,  0x1D },
	{ LD,   E,   n8,  0x1E },
	{ RRA,  NO,  NO,  0x1F },
	
	{ JR,   ccNZ,e8,  0x20 },
	{ LD,   HL,  n16, 0x21 },
	{ LD, m_HLI, A,   0x22 },
	{ INC,  HL,  NO,  0x23 },
	{ INC,  H,   NO,  0x24 },
	{ DEC,  H,   NO,  0x25 },
	{ LD,   H,   n8,  0x26 },
	{ DAA,  NO,  NO,  0x27 },
	{ JR,   ccZ, e8,  0x28 },
	{ ADD,  HL,  HL,  0x29 },
	{ LD,   A, m_HLI, 0x2A },
	{ DEC,  HL,  NO,  0x2B },
	{ INC,  L,   NO,  0x2C },
	{ DEC,  L,   NO,  0x2D },
	{ LD,   L,   n8,  0x2E },
	{ CPL,  NO,  NO,  0x2F },
	
	{ JR,   ccNC,e8,  0x30 },
	{ LD,   SP,  n16, 0x31 },
	{ LD, m_HLD, A,   0x32 },
	{ INC,  SP,  NO,  0x33 },
	{ INC,m_HL,  NO,  0x34 },
	{ DEC,m_HL,  NO,  0x35 },
	{ LD, m_HL,  n8,  0x36 },
	{ SCF,  NO,  NO,  0x37 },
	{ JR,   ccC, e8,  0x38 },
	{ ADD,  HL,  SP,  0x39 },
	{ LD,   A, m_HLD, 0x3A },
	{ DEC,  SP,  NO,  0x3B },
	{ INC,  A,   NO,  0x3C },
	{ DEC,  A,   NO,  0x3D },
	{ LD,   A,   n8,  0x3E },
	{ CCF,  NO,  NO,  0x3F },
	
	{ LD,   B,   B,   0x40 },
	{ LD,   B,   C,   0x41 },
	{ LD,   B,   D,   0x42 },
	{ LD,   B,   E,   0x43 },
	{ LD,   B,   H,   0x44 },
	{ LD,   B,   L,   0x45 },
	{ LD,   B, m_HL,  0x46 },
	{ LD,   B,   A,   0x47 },
	{ LD,   C,   B,   0x48 },
	{ LD,   C,   C,   0x49 },
	{ LD,   C,   D,   0x4A },
	{ LD,   C,   E,   0x4B },
	{ LD,   C,   H,   0x4C },
	{ LD,   C,   L,   0x4D },
	{ LD,   C, m_HL,  0x4E },
	{ LD,   C,   A,   0x4F },
	
	{ LD,   D,   B,   0x50 },
	{ LD,   D,   C,   0x51 },
	{ LD,   D,   D,   0x52 },
	{ LD,   D,   E,   0x53 },
	{ LD,   D,   H,   0x54 },
	{ LD,   D,   L,   0x55 },
	{ LD,   D, m_HL,  0x56 },
	{ LD,   D,   A,   0x57 },
	{ LD,   E,   B,   0x58 },
	{ LD,   E,   C,   0x59 },
	{ LD,   E,   D,   0x5A },
	{ LD,   E,   E,   0x5B },
	{ LD,   E,   H,   0x5C },
	{ LD,   E,   L,   0x5D },
	{ LD,   E, m_HL,  0x5E },
	{ LD,   E,   A,   0x5F },
	
	{ LD,   H,   B,   0x60 },
	{ LD,   H,   C,   0x61 },
	{ LD,   H,   D,   0x62 },
	{ LD,   H,   E,   0x63 },
	{ LD,   H,   H,   0x64 },
	{ LD,   H,   L,   0x65 },
	{ LD,   H, m_HL,  0x66 },
	{ LD,   H,   A,   0x67 },
	{ LD,   L,   B,   0x68 },
	{ LD,   L,   C,   0x69 },
	{ LD,   L,   D,   0x6A },
	{ LD,   L,   E,   0x6B },
	{ LD,   L,   H,   0x6C },
	{ LD,   L,   L,   0x6D },
	{ LD,   L, m_HL,  0x6E },
	{ LD,   L,   A,   0x6F },
	
	{ LD, m_HL,  B,   0x70 },
	{ LD, m_HL,  C,   0x71 },
	{ LD, m_HL,  D,   0x72 },
	{ LD, m_HL,  E,   0x73 },
	{ LD, m_HL,  H,   0x74 },
	{ LD, m_HL,  L,   0x75 },
	{ HALT, NO,  NO,  0x76 },
	{ LD, m_HL,  A,   0x77 },
	{ LD,   A,   B,   0x78 },
	{ LD,   A,   C,   0x79 },
	{ LD,   A,   D,   0x7A },
	{ LD,   A,   E,   0x7B },
	{ LD,   A,   H,   0x7C },
	{ LD,   A,   L,   0x7D },
	{ LD,   A, m_HL,  0x7E },
	{ LD,   A,   A,   0x7F },
	
	{ ADD,  A,   B,   0x80 },
	{ ADD,  A,   C,   0x81 },
	{ ADD,  A,   D,   0x82 },
	{ ADD,  A,   E,   0x83 },
	{ ADD,  A,   H,   0x84 },
	{ ADD,  A,   L,   0x85 },
	{ ADD,  A, m_HL,  0x86 },
	{ ADD,  A,   A,   0x87 },
	{ ADC,  A,   B,   0x88 },
	{ ADC,  A,   C,   0x89 },
	{ ADC,  A,   D,   0x8A },
	{ ADC,  A,   E,   0x8B },
	{ ADC,  A,   H,   0x8C },
	{ ADC,  A,   L,   0x8D },
	{ ADC,  A, m_HL,  0x8E },
	{ ADC,  A,   A,   0x8F },
	
	{ SUB,  A,   B,   0x90 },
	{ SUB,  A,   C,   0x91 },
	{ SUB,  A,   D,   0x92 },
	{ SUB,  A,   E,   0x93 },
	{ SUB,  A,   H,   0x94 },
	{ SUB,  A,   L,   0x95 },
	{ SUB,  A, m_HL,  0x96 },
	{ SUB,  A,   A,   0x97 },
	{ SBC,  A,   B,   0x98 },
	{ SBC,  A,   C,   0x99 },
	{ SBC,  A,   D,   0x9A },
	{ SBC,  A,   E,   0x9B },
	{ SBC,  A,   H,   0x9C },
	{ SBC,  A,   L,   0x9D },
	{ SBC,  A, m_HL,  0x9E },
	{ SBC,  A,   A,   0x9F },
	
	{ AND,  A,   B,   0xA0 },
	{ AND,  A,   C,   0xA1 },
	{ AND,  A,   D,   0xA2 },
	{ AND,  A,   E,   0xA3 },
	{ AND,  A,   H,   0xA4 },
	{ AND,  A,   L,   0xA5 },
	{ AND,  A, m_HL,  0xA6 },
	{ AND,  A,   A,   0xA7 },
	{ XOR,  A,   B,   0xA8 },
	{ XOR,  A,   C,   0xA9 },
	{ XOR,  A,   D,   0xAA },
	{ XOR,  A,   E,   0xAB },
	{ XOR,  A,   H,   0xAC },
	{ XOR,  A,   L,   0xAD },
	{ XOR,  A, m_HL,  0xAE },
	{ XOR,  A,   A,   0xAF },
	
	{ OR,   A,   B,   0xB0 },
	{ OR,   A,   C,   0xB1 },
	{ OR,   A,   D,   0xB2 },
	{ OR,   A,   E,   0xB3 },
	{ OR,   A,   H,   0xB4 },
	{ OR,   A,   L,   0xB5 },
	{ OR,   A, m_HL,  0xB6 },
	{ OR,   A,   A,   0xB7 },
	{ CP,   A,   B,   0xB8 },
	{ CP,   A,   C,   0xB9 },
	{ CP,   A,   D,   0xBA },
	{ CP,   A,   E,   0xBB },
	{ CP,   A,   H,   0xBC },
	{ CP,   A,   L,   0xBD },
	{ CP,   A, m_HL,  0xBE },
	{ CP,   A,   A,   0xBF },
	
	{ RET,  ccNZ,NO,  0xC0 },
	{ POP,  BC,  NO,  0xC1 },
	{ JP,   ccNZ,n16, 0xC2 },
	{ JP,   n16, NO,  0xC3 },
	{ CALL, ccNZ,n16, 0xC4 },
	{ PUSH, BC,  NO,  0xC5 },
	{ ADD,  A,   n8,  0xC6 },
	{ RST,  v00, NO,  0xC7 },
	{ RET,  ccZ, NO,  0xC8 },
	{ RET,  NO,  NO,  0xC9 },
	{ JP,   ccZ, n16, 0xCA },
	//  PREFIX        0xCB
	{ CALL, ccZ, n16, 0xCC },
	{ CALL, n16, NO,  0xCD },
	{ ADC,  A,   n8,  0xCE },
	{ RST,  v08, NO,  0xCF },
	
	{ RET,  ccNC,NO,  0xD0 },
	{ POP,  DE,  NO,  0xD1 },
	{ JP,   ccNC,n16, 0xD2 },
	//  ---           0xD3
	{ CALL, ccNC,n16, 0xD4 },
	{ PUSH, DE,  NO,  0xD5 },
	{ SUB,  A,   n8,  0xD6 },
	{ RST,  v10, NO,  0xD7 },
	{ RET,  ccC, NO,  0xD8 },
	{ RETI, NO,  NO,  0xD9 },
	{ JP,   ccC, n16, 0xDA },
	//  ---           0xDB
	{ CALL, ccC, n16, 0xDC },
	//  ---           0xDD
	{ SBC,  A,   n8,  0xDE },
	{ RST,  v18, NO,  0xDF },
	
	{ LDH,m_a8,  A,   0xE0 },
	{ POP,  HL,  NO,  0xE1 },
	{ LDH,m_C,   A,   0xE2 },
	//  ---           0xE3
	//  ---           0xE4
	{ PUSH, HL,  NO,  0xE5 },
	{ AND,  A,   n8,  0xE6 },
	{ RST,  v20, NO,  0xE7 },
	{ ADD,  SP,  e8,  0xE8 },
	{ JP,   HL,  NO,  0xE9 },
	{ LD, m_a16, A,   0xEA },
	//  ---           0xEB
	//  ---           0xEC
	//  ---           0xED
	{ XOR,  A,   n8,  0xEE },
	{ RST,  v28, NO,  0xEF },
	
	{ LDH,  A, m_a8,  0xF0 },
	{ POP,  AF,  NO,  0xF1 },
	{ LDH,  A, m_C,   0xF2 },
	{ DI,   NO,  NO,  0xF3 },
	//  ---           0xF4
	{ PUSH, AF,  NO,  0xF5 },
	{ OR,   A,   n8,  0xF6 },
	{ RST,  v30, NO,  0xF7 },
	{ LD,   HL,  SPe8,0xF8 },
	{ LD,   SP,  HL,  0xF9 },
	{ LD,   A, m_a16, 0xFA },
	{ EI,   NO,  NO,  0xFB },
	//  ---           0xFC
	//  ---           0xFD
	{ CP,   A,   n8,  0xFE },
	{ RST,  v38, NO,  0xFF }
};

const struct instruction PREFIXED_INSTRUCTIONS[] = {
	{ RLC,  B,   NO,  0x00 },
	{ RLC,  C,   NO,  0x01 },
	{ RLC,  D,   NO,  0x02 },
	{ RLC,  E,   NO,  0x03 },
	{ RLC,  H,   NO,  0x04 },
	{ RLC,  L,   NO,  0x05 },
	{ RLC,m_HL,  NO,  0x06 },
	{ RLC,  A,   NO,  0x07 },
	{ RRC,  B,   NO,  0x08 },
	{ RRC,  C,   NO,  0x09 },
	{ RRC,  D,   NO,  0x0A },
	{ RRC,  E,   NO,  0x0B },
	{ RRC,  H,   NO,  0x0C },
	{ RRC,  L,   NO,  0x0D },
	{ RRC,m_HL,  NO,  0x0E },
	{ RRC,  A,   NO,  0x0F },
	
	{ RL,   B,   NO,  0x10 },
	{ RL,   C,   NO,  0x11 },
	{ RL,   D,   NO,  0x12 },
	{ RL,   E,   NO,  0x13 },
	{ RL,   H,   NO,  0x14 },
	{ RL,   L,   NO,  0x15 },
	{ RL, m_HL,  NO,  0x16 },
	{ RL,   A,   NO,  0x17 },
	{ RR,   B,   NO,  0x18 },
	{ RR,   C,   NO,  0x19 },
	{ RR,   D,   NO,  0x1A },
	{ RR,   E,   NO,  0x1B },
	{ RR,   H,   NO,  0x1C },
	{ RR,   L,   NO,  0x1D },
	{ RR, m_HL,  NO,  0x1E },
	{ RR,   A,   NO,  0x1F },
	
	{ SLA,  B,   NO,  0x20 },
	{ SLA,  C,   NO,  0x21 },
	{ SLA,  D,   NO,  0x22 },
	{ SLA,  E,   NO,  0x23 },
	{ SLA,  H,   NO,  0x24 },
	{ SLA,  L,   NO,  0x25 },
	{ SLA,m_HL,  NO,  0x26 },
	{ SLA,  A,   NO,  0x27 },
	{ SRA,  B,   NO,  0x28 },
	{ SRA,  C,   NO,  0x29 },
	{ SRA,  D,   NO,  0x2A },
	{ SRA,  E,   NO,  0x2B },
	{ SRA,  H,   NO,  0x2C },
	{ SRA,  L,   NO,  0x2D },
	{ SRA,m_HL,  NO,  0x2E },
	{ SRA,  A,   NO,  0x2F },
	
	{ SWAP, B,   NO,  0x30 },
	{ SWAP, C,   NO,  0x31 },
	{ SWAP, D,   NO,  0x32 },
	{ SWAP, E,   NO,  0x33 },
	{ SWAP, H,   NO,  0x34 },
	{ SWAP, L,   NO,  0x35 },
	{ SWAP,m_HL, NO,  0x36 },
	{ SWAP, A,   NO,  0x37 },
	{ SRL,  B,   NO,  0x38 },
	{ SRL,  C,   NO,  0x39 },
	{ SRL,  D,   NO,  0x3A },
	{ SRL,  E,   NO,  0x3B },
	{ SRL,  H,   NO,  0x3C },
	{ SRL,  L,   NO,  0x3D },
	{ SRL,m_HL,  NO,  0x3E },
	{ SRL,  A,   NO,  0x3F },
	
	{ BIT,  u0,  B,   0x40 },
	{ BIT,  u0,  C,   0x41 },
	{ BIT,  u0,  D,   0x42 },
	{ BIT,  u0,  E,   0x43 },
	{ BIT,  u0,  H,   0x44 },
	{ BIT,  u0,  L,   0x45 },
	{ BIT,  u0,m_HL,  0x46 },
	{ BIT,  u0,  A,   0x47 },
	{ BIT,  u1,  B,   0x48 },
	{ BIT,  u1,  C,   0x49 },
	{ BIT,  u1,  D,   0x4A },
	{ BIT,  u1,  E,   0x4B },
	{ BIT,  u1,  H,   0x4C },
	{ BIT,  u1,  L,   0x4D },
	{ BIT,  u1,m_HL,  0x4E },
	{ BIT,  u1,  A,   0x4F },
	
	{ BIT,  u2,  B,   0x50 },
	{ BIT,  u2,  C,   0x51 },
	{ BIT,  u2,  D,   0x52 },
	{ BIT,  u2,  E,   0x53 },
	{ BIT,  u2,  H,   0x54 },
	{ BIT,  u2,  L,   0x55 },
	{ BIT,  u2,m_HL,  0x56 },
	{ BIT,  u2,  A,   0x57 },
	{ BIT,  u3,  B,   0x58 },
	{ BIT,  u3,  C,   0x59 },
	{ BIT,  u3,  D,   0x5A },
	{ BIT,  u3,  E,   0x5B },
	{ BIT,  u3,  H,   0x5C },
	{ BIT,  u3,  L,   0x5D },
	{ BIT,  u3,m_HL,  0x5E },
	{ BIT,  u3,  A,   0x5F },
	
	{ BIT,  u4,  B,   0x60 },
	{ BIT,  u4,  C,   0x61 },
	{ BIT,  u4,  D,   0x62 },
	{ BIT,  u4,  E,   0x63 },
	{ BIT,  u4,  H,   0x64 },
	{ BIT,  u4,  L,   0x65 },
	{ BIT,  u4,m_HL,  0x66 },
	{ BIT,  u4,  A,   0x67 },
	{ BIT,  u5,  B,   0x68 },
	{ BIT,  u5,  C,   0x69 },
	{ BIT,  u5,  D,   0x6A },
	{ BIT,  u5,  E,   0x6B },
	{ BIT,  u5,  H,   0x6C },
	{ BIT,  u5,  L,   0x6D },
	{ BIT,  u5,m_HL,  0x6E },
	{ BIT,  u5,  A,   0x6F },
	
	{ BIT,  u6,  B,   0x70 },
	{ BIT,  u6,  C,   0x71 },
	{ BIT,  u6,  D,   0x72 },
	{ BIT,  u6,  E,   0x73 },
	{ BIT,  u6,  H,   0x74 },
	{ BIT,  u6,  L,   0x75 },
	{ BIT,  u6,m_HL,  0x76 },
	{ BIT,  u6,  A,   0x77 },
	{ BIT,  u7,  B,   0x78 },
	{ BIT,  u7,  C,   0x79 },
	{ BIT,  u7,  D,   0x7A },
	{ BIT,  u7,  E,   0x7B },
	{ BIT,  u7,  H,   0x7C },
	{ BIT,  u7,  L,   0x7D },
	{ BIT,  u7,m_HL,  0x7E },
	{ BIT,  u7,  A,   0x7F },
	
	{ RES,  u0,  B,   0x80 },
	{ RES,  u0,  C,   0x81 },
	{ RES,  u0,  D,   0x82 },
	{ RES,  u0,  E,   0x83 },
	{ RES,  u0,  H,   0x84 },
	{ RES,  u0,  L,   0x85 },
	{ RES,  u0,m_HL,  0x86 },
	{ RES,  u0,  A,   0x87 },
	{ RES,  u1,  B,   0x88 },
	{ RES,  u1,  C,   0x89 },
	{ RES,  u1,  D,   0x8A },
	{ RES,  u1,  E,   0x8B },
	{ RES,  u1,  H,   0x8C },
	{ RES,  u1,  L,   0x8D },
	{ RES,  u1,m_HL,  0x8E },
	{ RES,  u1,  A,   0x8F },
	
	{ RES,  u2,  B,   0x90 },
	{ RES,  u2,  C,   0x91 },
	{ RES,  u2,  D,   0x92 },
	{ RES,  u2,  E,   0x93 },
	{ RES,  u2,  H,   0x94 },
	{ RES,  u2,  L,   0x95 },
	{ RES,  u2,m_HL,  0x96 },
	{ RES,  u2,  A,   0x97 },
	{ RES,  u3,  B,   0x98 },
	{ RES,  u3,  C,   0x99 },
	{ RES,  u3,  D,   0x9A },
	{ RES,  u3,  E,   0x9B },
	{ RES,  u3,  H,   0x9C },
	{ RES,  u3,  L,   0x9D },
	{ RES,  u3,m_HL,  0x9E },
	{ RES,  u3,  A,   0x9F },
	
	{ RES,  u4,  B,   0xA0 },
	{ RES,  u4,  C,   0xA1 },
	{ RES,  u4,  D,   0xA2 },
	{ RES,  u4,  E,   0xA3 },
	{ RES,  u4,  H,   0xA4 },
	{ RES,  u4,  L,   0xA5 },
	{ RES,  u4,m_HL,  0xA6 },
	{ RES,  u4,  A,   0xA7 },
	{ RES,  u5,  B,   0xA8 },
	{ RES,  u5,  C,   0xA9 },
	{ RES,  u5,  D,   0xAA },
	{ RES,  u5,  E,   0xAB },
	{ RES,  u5,  H,   0xAC },
	{ RES,  u5,  L,   0xAD },
	{ RES,  u5,m_HL,  0xAE },
	{ RES,  u5,  A,   0xAF },
	
	{ RES,  u6,  B,   0xB0 },
	{ RES,  u6,  C,   0xB1 },
	{ RES,  u6,  D,   0xB2 },
	{ RES,  u6,  E,   0xB3 },
	{ RES,  u6,  H,   0xB4 },
	{ RES,  u6,  L,   0xB5 },
	{ RES,  u6,m_HL,  0xB6 },
	{ RES,  u6,  A,   0xB7 },
	{ RES,  u7,  B,   0xB8 },
	{ RES,  u7,  C,   0xB9 },
	{ RES,  u7,  D,   0xBA },
	{ RES,  u7,  E,   0xBB },
	{ RES,  u7,  H,   0xBC },
	{ RES,  u7,  L,   0xBD },
	{ RES,  u7,m_HL,  0xBE },
	{ RES,  u7,  A,   0xBF },
	
	{ SET,  u0,  B,   0xC0 },
	{ SET,  u0,  C,   0xC1 },
	{ SET,  u0,  D,   0xC2 },
	{ SET,  u0,  E,   0xC3 },
	{ SET,  u0,  H,   0xC4 },
	{ SET,  u0,  L,   0xC5 },
	{ SET,  u0,m_HL,  0xC6 },
	{ SET,  u0,  A,   0xC7 },
	{ SET,  u1,  B,   0xC8 },
	{ SET,  u1,  C,   0xC9 },
	{ SET,  u1,  D,   0xCA },
	{ SET,  u1,  E,   0xCB },
	{ SET,  u1,  H,   0xCC },
	{ SET,  u1,  L,   0xCD },
	{ SET,  u1,m_HL,  0xCE },
	{ SET,  u1,  A,   0xCF },
	
	{ SET,  u2,  B,   0xD0 },
	{ SET,  u2,  C,   0xD1 },
	{ SET,  u2,  D,   0xD2 },
	{ SET,  u2,  E,   0xD3 },
	{ SET,  u2,  H,   0xD4 },
	{ SET,  u2,  L,   0xD5 },
	{ SET,  u2,m_HL,  0xD6 },
	{ SET,  u2,  A,   0xD7 },
	{ SET,  u3,  B,   0xD8 },
	{ SET,  u3,  C,   0xD9 },
	{ SET,  u3,  D,   0xDA },
	{ SET,  u3,  E,   0xDB },
	{ SET,  u3,  H,   0xDC },
	{ SET,  u3,  L,   0xDD },
	{ SET,  u3,m_HL,  0xDE },
	{ SET,  u3,  A,   0xDF },
	
	{ SET,  u4,  B,   0xE0 },
	{ SET,  u4,  C,   0xE1 },
	{ SET,  u4,  D,   0xE2 },
	{ SET,  u4,  E,   0xE3 },
	{ SET,  u4,  H,   0xE4 },
	{ SET,  u4,  L,   0xE5 },
	{ SET,  u4,m_HL,  0xE6 },
	{ SET,  u4,  A,   0xE7 },
	{ SET,  u5,  B,   0xE8 },
	{ SET,  u5,  C,   0xE9 },
	{ SET,  u5,  D,   0xEA },
	{ SET,  u5,  E,   0xEB },
	{ SET,  u5,  H,   0xEC },
	{ SET,  u5,  L,   0xED },
	{ SET,  u5,m_HL,  0xEE },
	{ SET,  u5,  A,   0xEF },
	
	{ SET,  u6,  B,   0xF0 },
	{ SET,  u6,  C,   0xF1 },
	{ SET,  u6,  D,   0xF2 },
	{ SET,  u6,  E,   0xF3 },
	{ SET,  u6,  H,   0xF4 },
	{ SET,  u6,  L,   0xF5 },
	{ SET,  u6,m_HL,  0xF6 },
	{ SET,  u6,  A,   0xF7 },
	{ SET,  u7,  B,   0xF8 },
	{ SET,  u7,  C,   0xF9 },
	{ SET,  u7,  D,   0xFA },
	{ SET,  u7,  E,   0xFB },
	{ SET,  u7,  H,   0xFC },
	{ SET,  u7,  L,   0xFD },
	{ SET,  u7,m_HL,  0xFE },
	{ SET,  u7,  A,   0xFF }
};





FILE* asmfile;
char* asmpath;
FILE* romfile;
unsigned int linenumber;
bool asmfilebackedup = false;

unsigned long mempos;
enum asmregiontype currentregion = INVALID_REGION_TYPE;
unsigned char currentsection[1024];
unsigned char metasection[1024];

struct label* labellist[64];
unsigned int labelcount = 0;
struct assumption* assumptionlist[64];
unsigned int assumptioncount = 0;
struct macro* macrolist[64];
unsigned int macrocount = 0;
struct variable* variablelist[64];
unsigned int variablecount = 0;
struct constant* constantlist[64];
unsigned int constantcount = 0;
struct constantstring* conststrlist[64];
unsigned int conststrcount = 0;
struct mapchar* charmaplist[64];
unsigned int charmapcount = 0;

unsigned int localmacrostart;
unsigned int localvariablestart;
unsigned int localconstantstart;
unsigned int localconststrstart;

#define labels(i) labellist[(i)/1024][(i)%1024]
#define asmpts(i) assumptionlist[(i)/1024][(i)%1024]
#define macros(i) macrolist[(i)/1024][(i)%1024]
#define varbls(i) variablelist[(i)/1024][(i)%1024]
#define consts(i) constantlist[(i)/1024][(i)%1024]
#define costrs(i) conststrlist[(i)/1024][(i)%1024]
#define charms(i) charmaplist[(i)/1024][(i)%1024]

#define incrementLabellist labelcount++; if(labelcount % 1024 == 0) labellist[labelcount/1024] = malloc(1024 * sizeof(labellist[labelcount/1024][0]))
#define incrementAssumptionlist assumptioncount++; if(assumptioncount % 1024 == 0) assumptionlist[assumptioncount/1024] = malloc(1024 * sizeof(assumptionlist[assumptioncount/1024][0]))
#define incrementMacrolist macrocount++; if(macrocount % 1024 == 0) macrolist[macrocount/1024] = malloc(1024 * sizeof(macrolist[macrocount/1024][0]))
#define incrementVariablelist variablecount++; if(variablecount % 1024 == 0) variablelist[variablecount/1024] = malloc(1024 * sizeof(variablelist[variablecount/1024][0]))
#define incrementConstantlist constantcount++; if(constantcount % 1024 == 0) constantlist[constantcount/1024] = malloc(1024 * sizeof(constantlist[constantcount/1024][0]))
#define incrementConststrlist conststrcount++; if(conststrcount % 1024 == 0) conststrlist[conststrcount/1024] = malloc(1024 * sizeof(conststrlist[conststrcount/1024][0]))
#define incrementCharmaplist charmapcount++; if(charmapcount % 1024 == 0) charmaplist[charmapcount/1024] = malloc(1024 * sizeof(charmaplist[charmapcount/1024][0]))

#define assertTokenType(token, TYPE) if(token.type != TYPE) { errorUnexpectedToken(token); return -1; }
#define assertLineBreak(token) if(token.type != NEWLINE && token.type != END_OF_FILE) { errorUnexpectedToken(token); return -1; }
#define assertToken(token, TYPE, CONTENT)  if(token.type != TYPE || token.content != CONTENT) { errorUnexpectedToken(token); return -1; }
#define assertNextTokenType(TYPE) identifyNextToken(); assertTokenType(token, TYPE)
#define assertNextToken(TYPE, CONTENT) identifyNextToken(); assertToken(token, TYPE, CONTENT)
#define assertNextLineBreak() identifyNextToken(); assertLineBreak(token)

long predeclaredsymbols[PREDEFS_COUNT];
#define _NARG predeclaredsymbols[NARG]
#define _RS predeclaredsymbols[RS]


bool charisliteral = false;
bool firsttoken = false;
bool indef = false;
bool newuniqueaffix = false;
bool loaddata = false;
bool loadlabels = false;
unsigned char** currentargs = 0;

unsigned char ungottenchars[1024];
unsigned int ungottencharcount = 0;
unsigned char** expandedstrs;
unsigned int expandedstrcount = 0;
unsigned int* expandedstrspos;

unsigned char* currentmacro = 0;
unsigned int currentmacropos;

char symbolstr[1024];
unsigned char labelscope[1024];
unsigned char* locallabelscope;

unsigned int ifdepth = 0;

unsigned long repeatcount = 0;
unsigned char repeatsymbol[32];
bool repeating = false;

unsigned long unionstart;
unsigned long unionend;
bool inunion = false;

char bin0 = '0';
char bin1 = '1';



// errors
	void printLocation() { printf("\n%02X:%04X %s (%d)", mempos>>16, mempos&0xFFFF, asmpath, linenumber); }
	
	void errorUnexpectedToken(struct token token) {
		printLocation();
		const char* content;
		switch(token.type) {
		case OPCODE:
			content = OPCODES[token.content];
			break;
		case DIRECTIVE:
			content = DIRECTIVES[token.content];
			break;
		case REGISTER:
			content = REGISTERS[token.content];
			break;
		case CONDITION:
			content = CCS[token.content];
			break;
		case OPERATOR:
			content = OPERATORS[token.content];
			break;
		case MACRO:
			content = macros(token.content).name;
			break;
		case LABEL:
			content = labels(token.content).name;
			break;
		case VARIABLE:
			content = varbls(token.content).name;
			break;
		case CONSTANT:
			content = consts(token.content).name;
			break;
		case CONSTANT_STRING:
			content = costrs(token.content).name;
			break;
		case ASSUMPTION:
			content = asmpts(token.content).name;
			break;
		case NUMBER:
			sprintf(symbolstr, "%d", token.content);
			content = symbolstr;
			break;
		case UNRECORDED_SYMBOL:
			content = (char*)token.content;
			break;
		case REGION_TYPE:
			content = REGIONS[token.content];
			break;
		default:
			content = TOKENS[token.type];
			break;
		}
		printf("\nError: Unexpected %s \"%s\"\n", TOKENS[token.type], content);
	}
	
	void errorCannotFindFile(const char* filepath) {
		printLocation();
		printf("\nError: Cannot find \"%s\"\n", filepath);
	}
	
	void errorValueTooLarge(unsigned int numbits) {
		printLocation();
		printf("\nError: Value must be %d-bit\n", numbits);
	}

	void errorTargetOutOfReach(long value) {
		printLocation();
		printf("\nError: Target out of reach %d\n", (signed char)value);
	}

	void errorInconsistentAssumption(char* name, int currentvalue, int previousvalue) {
		printLocation();
		printf("\nError: %s was previously assumed as $%04X, and is now assumed as $%04X.\n", "?", previousvalue, currentvalue);
	}

	void errorIncorrectAssumption(char* name, int realvalue, int assumedvalue) {
		printLocation();
		printf("\nError: %s was previously assumed as $%04X, and is now defined as $%04X.\n", name, assumedvalue, realvalue);
	}

	void errorLabelOverflow() {
		printLocation();
		printf("\nError: Too many labels!!\n");
	}

	void errorMacroOverflow() {
		printLocation();
		printf("\nError: Too many macros!!\n");
	}

	void errorNestedMacros() {
		printLocation();
		printf("\nError: Cannot nest macros\n");
	}
	
	void errorMacroWithoutEnd() {
		printLocation();
		printf("\nError: Macro started without end\n");
	}
	
	void errorCannotShiftOutsideMacro() {
		printLocation();
		printf("\nError: Shift cannot exist outside of a macro\n");
	}

	void errorArgRequired(unsigned int expectedarg, unsigned int givenargcount) {
		printLocation();
		printf("\nError: Argument %d is required in this maco, but only %d are giveen\n", expectedarg, givenargcount);
	}

	void errorLabelAlreadyDefined(char* name) {
		printLocation();
		printf("\nError: Second definition of %s\n", name);
	}

	void errorInconsistentData(int romdata, int asmdata) {
		printLocation();
		printf("\nError: Data ($%02X) is inconsistent with ROM data ($%02X)\n", asmdata, romdata);
	}
	
	void printArgument(char* buffer, enum asmarg arg, unsigned int argcontent) {
		switch(arg) {
		case NO:
			buffer[0] = '\0';
			break;
		case n8:
			sprintf(buffer, "$%02X", (unsigned char) argcontent);
			break;
		case e8:
			sprintf(buffer, "$%04X", ((signed char) argcontent) + ((mempos&0xFFFF)+2));
			break;
		case m_a8:
			sprintf(buffer, "[$FF%02X]", (unsigned char) argcontent);
			break;
		case SPe8:
			sprintf(buffer, "sp %s $%02X", 
				((signed char) argcontent) < 0 ? "-" : "+",
				((signed char) argcontent) < 0 ? ((unsigned char) argcontent) : -((unsigned char) argcontent));
			break;
		case n16:
			sprintf(buffer, "$%04X", argcontent);
			break;
		case m_a16:
			sprintf(buffer, "[$%04X]", argcontent);
			break;
		default:
			strcpy(buffer, OPCODEARGS[arg]);
			break;
		}
	}
	
	void suggestLabel(char* buffer, enum asmarg arg1, enum asmarg arg2, unsigned int argcontent) {
		unsigned int address = 0;
		
		switch(arg1) {
		case e8:
			address = ((signed char) argcontent) + ((mempos&0xFFFF)+2);
			break;
		case m_a8:
			address = (argcontent&0xFF) | 0xFF00;
			break;
		case n16:
			address = argcontent & 0xFFFF;
			break;
		case m_a16:
			address = argcontent & 0xFFFF;
			break;
		}
		
		switch(arg2) {
		case e8:
			address = ((signed char) argcontent) + ((mempos&0xFFFF)+2);
			break;
		case m_a8:
			address = (argcontent&0xFF) | 0xFF00;
			break;
		case n16:
			address = argcontent & 0xFFFF;
			break;
		case m_a16:
			address = argcontent & 0xFFFF;
			break;
		}
		
		if(address == 0) return;
		
		for(unsigned int i = 0; i < labelcount; i++) {
			if((address | (address < 0x4000 ? 0 : (mempos & 0xFF0000))) == labels(i).value & (address < 0x4000 ? 0xFFFF : 0xFFFFFF)) {
				sprintf(buffer, " (\"%s\"?)", labels(i).name);
				return;
			}
		}
		
		for(unsigned int i = 0; i < assumptioncount; i++) {
			if((address&0xFF) == asmpts(i).valuel && asmpts(i).l && ((address>>8)&0xFF) == asmpts(i).valuem && asmpts(i).m) {
				sprintf(buffer, " (\"%s\"?)", asmpts(i).name);
				return;
			}
		}
	}
	
	void errorInconsistentDataInstruction(enum asmopcode asmopcode, enum asmarg asmarg1, enum asmarg asmarg2, unsigned int asmargcontent,
	                                      enum asmopcode romopcode, enum asmarg romarg1, enum asmarg romarg2, unsigned int romargcontent) {
		printLocation();
		char asminstruction[32];
		strcpy(asminstruction, OPCODES[asmopcode]);
		if(asmarg1 != NO) strcat(asminstruction, " ");
		printArgument(&asminstruction[strlen(asminstruction)], asmarg1, asmargcontent);
		if(asmarg2 != NO) strcat(asminstruction, ", ");
		printArgument(&asminstruction[strlen(asminstruction)], asmarg2, asmargcontent);
		
		char rominstruction[32];
		char romsuggestion[256];
		romsuggestion[0] = '\0';
		strcpy(rominstruction, OPCODES[romopcode]);
		if(romarg1 != NO) strcat(rominstruction, " ");
		printArgument(&rominstruction[strlen(rominstruction)], romarg1, romargcontent);
		if(romarg2 != NO) strcat(rominstruction, ", ");
		printArgument(&rominstruction[strlen(rominstruction)], romarg2, romargcontent);
		suggestLabel(romsuggestion, romarg1, romarg2, romargcontent);
		
		printf("\nError: Instruction \'%s\' is inconsistent with ROM data \'%s\'%s\n", asminstruction, rominstruction, romsuggestion);
	}
	
	void errorInconsistentDataFirstarg(int romdata, int asmdata) {
		printLocation();
		printf("\nError: First argument ($%02X) is inconsistent with ROM data ($%02X)\n", asmdata, romdata);
	}

	void errorInconsistentDataSecondarg(int romdata, int asmdata) {
		printLocation();
		printf("\nError: Second argument ($%02X) is inconsistent with ROM data ($%02X)\n", asmdata, romdata);
	}

	void errorUnknownInstruction(enum asmopcode opcode, enum asmarg arg1, enum asmarg arg2) {
		printLocation();
		if(arg1 == NO) printf("\nError: Unknown instruction %s\n", OPCODES[opcode]);
		else if(arg2 == NO) printf("\nError: Unknown instruction %s %s\n", OPCODES[opcode], OPCODEARGS[arg1]);
		else printf("\nError: Unknown instruction %s %s, %s\n", OPCODES[opcode], OPCODEARGS[arg1], OPCODEARGS[arg2]);
	}
	
	void errorUnbalancedParentheses() {
		printLocation();
		printf("\nError: Unbalanced parentheses\n");
	}
	
	void errorFail(char* failure) {
		printLocation();
		printf("\nFailure: %s\n", failure);
	}
	
	void errorWarn(char* warning) {
		printLocation();
		printf("\nWarning: %s\n", warning);
	}
	
	void errorAssert() {
		printLocation();
		printf("\nFailure: Assertion failed\n");
	}
	
	void errorAssertWithMessage(char* message) {
		printf("\nFailure(%d)(%02X:%04X): %s\n", message);
	}
	
	void errorSectionCannotContainData() {
		printLocation();
		printf("\nError: Section cannot contain code or data\n");
	}
	
	void errorLabelCannotExistOutsideOfSection() {
		printLocation();
		printf("\nError: Label cannot exist outside of a section\n");
	}
	
	void errorDataCannotExistOutsideOfSection() {
		printLocation();
		printf("\nError: Code or data cannot exist outside of a section\n");
	}
	
	void errorIncorrectRegion(enum asmregiontype currentregion, enum asmregiontype expectedregion) {
		printLocation();
		printf("\nError: Section assigned to %s, expected %s\n", REGIONS[currentregion], REGIONS[expectedregion]);
	}
	
	void errorSectionNotFound(char* section) {
		printf("\nError: Section \"%s\" was not found\n", section);
	}
	
	void errorReptWithoutEndr() {
		printLocation();
		printf("\nError: REPT declared without matching ENDR\n");
	}
	
	void errorUnionWithoutEndu() {
		printLocation();
		printf("\nError: UNION declared without matching ENDU\n");
	}
	
	void errorUnableToCalculate() {
		printLocation();
		printf("\nError: Unable to calculate expression\n");
	}
	
	void errorRedefinitionOfConstant(struct constant constant) {
		printLocation();
		printf("\nError: Redefinition of constant %s\n", constant.name);
	}



long savedasmfilepos;
unsigned int savedlinenumber;
unsigned int savedcurrentmacropos;
unsigned int savedexpandedstrcount;
unsigned int savedexpandedstrspos[64];
char savedungottenchars[1024];
unsigned int savedungottencharcount = 0;

void saveAsmPos() {
	savedasmfilepos = ftell(asmfile);
	savedlinenumber = linenumber;
	savedcurrentmacropos = currentmacropos;
	for(unsigned int i = 0; i < expandedstrcount; i++) savedexpandedstrspos[i] = expandedstrspos[i];
	savedexpandedstrcount = expandedstrcount;
	for(unsigned int i = 0; i < ungottencharcount; i++) savedungottenchars[i] = ungottenchars[i];
	savedungottencharcount = ungottencharcount;
}

void restoreAsmPos() {
	fseek(asmfile, savedasmfilepos, SEEK_SET);
	linenumber = savedlinenumber;
	currentmacropos = savedcurrentmacropos;
	for(unsigned int i = 0; i < savedexpandedstrcount; i++) expandedstrspos[i] = savedexpandedstrspos[i];
	expandedstrcount = savedexpandedstrcount;
	for(unsigned int i = 0; i < savedungottencharcount; i++) ungottenchars[i] = savedungottenchars[i];
	ungottencharcount = savedungottencharcount;
}

void numToBracestr(unsigned long value, char sign, bool exact, bool align, bool pad, unsigned char width, unsigned char frac, unsigned char prec, char type) {
	char bracestr[1024];
	unsigned int i = 0;
	
	if(value >= 0 && sign != 0) bracestr[i++] = sign;
	if((signed long) value < 0 && type == 'd') { bracestr[i++] = '-'; value = -((signed long) value); }
	
	if(exact) {
		switch(type) {
		case 'x':
		case 'X':
			bracestr[i++] = '$';
			break;
		case 'b':
			bracestr[i++] = '%';
			break;
		case 'o':
			bracestr[i++] = '&';
			break;
		}
	}
	
	char padchar = pad ? '0' : ' ';
	
	unsigned int base;
	switch(type) {
	case 'd':
	case 'u':
	case 'f':
		base = 10;
		break;
	case 'x':
	case 'X':
		base = 0x10;
		break;
	case 'b':
		base = 0b10;
		break;
	case 'o':
		base = 010;
		break;
	}
	
	unsigned long digit = 1;
	bool invalue = false;
	if(!align) for(unsigned int ii = 1; ii < width; ii++) digit *= base;
	else if(value == 0) {
		bracestr[i++] == '0';
		if(width != 0) width--;
	}
	while(value >= digit * base) {
		if(digit * base == 0) break;
		digit *= base;
	}
	while(width > 0 || value != 0 || digit != 0) {
		if(value >= digit) invalue = true;
		if(invalue) {
			bracestr[i++] = (value / digit) < 10 ? ((value / digit) + '0') : ((value / digit) + 'A'-10);
			value = value % digit;
			if(value == 0 && digit == 0) invalue = false;
		} else if(!align && value == 0 && digit <= 1) bracestr[i++] = '0';
		else bracestr[i++] = padchar;
		digit /= base;
		if(width != 0) width--;
	}
	
	// frac, prec, and fixed-point types are unimplemented
	
	bracestr[i] = '\0';
	
	expandedstrs[expandedstrcount] = malloc(strlen(bracestr)+1 + ungottencharcount);
	strcpy(expandedstrs[expandedstrcount], bracestr);
	strncat(expandedstrs[expandedstrcount], ungottenchars, ungottencharcount);
	ungottencharcount = 0;
	expandedstrspos[expandedstrcount] = 0;
	expandedstrcount++;
}

void ungetChar(char ungotten) {
	ungottenchars[ungottencharcount] = ungotten;
	ungottencharcount++;
}

char getNextChar() {
	unsigned char c;
	if(ungottencharcount > 0) c = ungottenchars[--ungottencharcount];
	
	else if(expandedstrcount > 0) {
		c = expandedstrs[expandedstrcount-1][expandedstrspos[expandedstrcount-1]];
		expandedstrspos[expandedstrcount-1]++;
		if(expandedstrs[expandedstrcount-1][expandedstrspos[expandedstrcount-1]] == '\0') {
			free(expandedstrs[expandedstrcount-1]);
			expandedstrcount--;
		}
	}
	
	else if(currentmacro != 0) c = currentmacro[currentmacropos++];
	
	else c = fgetc(asmfile);
	
	
	if(!charisliteral) {
		if(c == '{') {
		// interpolate symbol
			char symbol[1024];
			unsigned int i = 0;
			char sign = 0;
			bool exact = true;
			bool align = false;
			bool pad = false;
			unsigned char width = 1;
			unsigned char frac = 5;
			unsigned char prec = 8;
			char type = 'X';
			
			long savedasmfilepos = ftell(asmfile);
			unsigned int savedcurrentmacropos = currentmacropos;
			unsigned int savedexpandedstrcount = expandedstrcount;
			unsigned int savedexpandedstrspos[1024];
			for(unsigned int ii = 0; ii < expandedstrcount; ii++) savedexpandedstrspos[ii] = expandedstrspos[ii];
			
			unsigned int uniqueaffixpos = 256;
			unsigned int uniqueaffixorig = 256;
			symbol[i] = getNextChar();
			while(symbol[i] != '}') {
				if(symbol[i] == '\n' || symbol[i] == ';') {
					symbol[0] = '\0';
					break;
				}
				if(symbol[i] == ':') {
					i = 0;
					if(symbol[i] == '+' | symbol[i] == ' ') { sign = symbol[i++]; }
					if(symbol[i] == '#') { exact = true; i++; } else { exact = false; }
					if(symbol[i] == '-') { align = true; i++; }
					if(symbol[i] == '0') { pad = true; i++; }
					if(symbol[i] >= '0' && symbol[i] <= '9') {
						width = 0;
						while(symbol[i] >= '0' && symbol[i] <= '9') { width *= 10; width += symbol[i++] - '0'; }
					}
					if(symbol[i] == '.') { frac = 0; i++; while(symbol[i] >= '0' && symbol[i] <= '9') { frac *= 10; frac += symbol[i++] - '0'; } }
					if(symbol[i] == 'q') { prec = 0; i++; while(symbol[i] >= '0' && symbol[i] <= '9') { prec *= 10; prec += symbol[i++] - '0'; } }
					if(symbol[i] == 'd' || symbol[i] == 'u' || symbol[i] == 'x' || symbol[i] == 'X'
					|| symbol[i] == 'b' || symbol[i] == 'o' || symbol[i] == 'f' || symbol[i] == 's') { type = symbol[i]; i++; }
					i = 0;
					symbol[i] = getNextChar();
					continue;
				}
				if(symbol[i] == '\\') {
					symbol[i] = getNextChar();
					if(symbol[i] == '@') {
						uniqueaffixorig = i;
						symbol[i]   = '2';
						symbol[++i] = '5';
						symbol[++i] = '6';
						uniqueaffixpos = i;
					} else {
						ungetChar(symbol[i]);
						symbol[i] = '\\';
					}
				}
				symbol[++i] = getNextChar();
			}
			symbol[i] = '\0';
			
			if(strcmp(repeatsymbol, symbol) == 0) {
				numToBracestr(repeatcount, sign, exact, align, pad, width, frac, prec, type);
				c = getNextChar();
				return c;
			}
			
			if(strcmp(symbol, ".") == 0) {
				expandedstrs[expandedstrcount] = malloc(strlen(labelscope)+1);
				strcpy(expandedstrs[expandedstrcount], labelscope);
				expandedstrspos[expandedstrcount] = 0;
				expandedstrcount++;
				return getNextChar();
			}
			
			if(strcmp(symbol, "..") == 0) {
				expandedstrs[expandedstrcount] = malloc(strlen(locallabelscope)+1);
				strcpy(expandedstrs[expandedstrcount], locallabelscope);
				expandedstrspos[expandedstrcount] = 0;
				expandedstrcount++;
				return getNextChar();
			}
			
			while(true) {
				if(uniqueaffixpos < 256) {
					symbol[uniqueaffixpos]--;
					unsigned int underflowto = uniqueaffixpos;
					while(symbol[underflowto] == '0'-1) {
						symbol[underflowto] = '9';
						symbol[--underflowto]--;
						if(underflowto == uniqueaffixorig && symbol[underflowto] == '0') {
							for(unsigned int ii = uniqueaffixorig; ii < i; ii++) symbol[ii] = symbol[ii+1];
							uniqueaffixpos--;
							i--;
						}
					}
					if(uniqueaffixpos < uniqueaffixorig) break;
				}
				
				for(unsigned int ii = 0; ii < labelcount; ii++) if(strcmp(symbol, labels(ii).name) == 0) {
					numToBracestr(labels(ii).value & 0xFFFF, sign, exact, align, pad, width, frac, prec, type);
					c = getNextChar();
					return c;
				}
				
				for(unsigned int ii = 0; ii < constantcount; ii++) if(strcmp(symbol, consts(ii).name) == 0) {
					numToBracestr(consts(ii).value, sign, exact, align, pad, width, frac, prec, type);
					c = getNextChar();
					return c;
				}
				
				for(unsigned int ii = 0; ii < variablecount; ii++) if(strcmp(symbol, varbls(ii).name) == 0) {
					numToBracestr(varbls(ii).value, sign, exact, align, pad, width, frac, prec, type);
					c = getNextChar();
					return c;
				}
				
				for(unsigned int ii = 0; ii < conststrcount; ii++) if(strcmp(symbol, costrs(ii).name) == 0) {
					expandedstrs[expandedstrcount] = malloc(strlen(costrs(ii).content)+1);
					strcpy(expandedstrs[expandedstrcount], costrs(ii).content);
					expandedstrspos[expandedstrcount] = 0;
					expandedstrcount++;
					c = getNextChar();
					return c;
				}
				
				if(uniqueaffixpos >= 256 || (uniqueaffixpos == uniqueaffixorig && symbol[uniqueaffixpos] == '0')) break;
			}
			
			fseek(asmfile, savedasmfilepos, SEEK_SET);
			currentmacropos = savedcurrentmacropos;
			expandedstrcount = savedexpandedstrcount;
			for(unsigned int ii = 0; ii < savedexpandedstrcount; ii++) expandedstrspos[ii] = savedexpandedstrspos[ii];
			return '{';
		}
		
		
		else if(c == '\\') {
			long savedasmfilepos = ftell(asmfile);
			unsigned int savedcurrentmacropos = currentmacropos;
			unsigned int savedexpandedstrcount = expandedstrcount;
			unsigned int savedexpandedstrspos[64];
			for(unsigned int ii = 0; ii < expandedstrcount; ii++) savedexpandedstrspos[ii] = expandedstrspos[ii];
			
			char n = getNextChar();
			
			if(n == '#') {
				if(_NARG == 0) return 18;
				unsigned int ii = 0;
				for(unsigned int i = 0; i < _NARG; i++) ii += strlen(currentargs[i]) + 1;
				expandedstrs[expandedstrcount] = malloc(ii);
				ii = 0;
				for(unsigned int i = 0; i < _NARG; i++) {
					strcpy(&expandedstrs[expandedstrcount][ii], currentargs[i]);
					ii += strlen(currentargs[i]);
					expandedstrs[expandedstrcount][ii++] = ',';
				}
				expandedstrs[expandedstrcount][ii-1] = '\0';
				expandedstrspos[expandedstrcount] = 0;
				expandedstrcount++;
				return getNextChar();
			}
			
			// if(n == '@') {
			// 	expandedstrs[expandedstrcount] = malloc(6);
			// 	sprintf(expandedstrs[expandedstrcount], "%d", uniquesymbolaffix);
			// 	expandedstrspos[expandedstrcount] = 0;
			// 	expandedstrcount++;
			// 	return getNextChar();
			// }
			
			unsigned int argnum = 0;
			if(n >= '1' && n <= '9') argnum = n - '0';
			else if(n == '<') {
				char argnumstr[64];
				unsigned int i = 0;
				argnumstr[i] = getNextChar();
				while(argnumstr[i] != '>') argnumstr[++i] = getNextChar();
				argnumstr[i] = '\0';
				if(argnumstr[0] >= '0' && argnumstr[0] <= '9') {
					for(i = 0; argnumstr[i] != '\0'; i++) {
						argnum *= 10;
						argnum += argnumstr[i] - '0';
					}
				} else {
					if(strcmp(argnumstr, repeatsymbol) == 0) { argnum = repeatcount; goto argnumfound; }
					for(i = 0; i < PREDEFS_COUNT; i++) if(strcmp(argnumstr, PREDEFS[i]) == 0) { argnum = predeclaredsymbols[i]; goto argnumfound; }
					for(i = 0; i < variablecount; i++)   if(strcmp(argnumstr, varbls(i).name) == 0) { argnum = varbls(i).value; goto argnumfound; }
					for(i = 0; i < labelcount; i++)      if(strcmp(argnumstr, labels(i).name) == 0) { argnum = labels(i).value; goto argnumfound; }
					for(i = 0; i < constantcount; i++)   if(strcmp(argnumstr, consts(i).name) == 0) { argnum = consts(i).value; goto argnumfound; }
					for(i = 0; i < assumptioncount; i++) if(strcmp(argnumstr, asmpts(i).name) == 0) { argnum = asmpts(i).valuel | asmpts(i).valuem | asmpts(i).valueh; goto argnumfound; }
				}
				
			} argnumfound:
			if(argnum > 0 && _NARG >= argnum) {
				expandedstrs[expandedstrcount] = malloc(strlen(currentargs[argnum-1])+1 + ungottencharcount);
				strcpy(expandedstrs[expandedstrcount], currentargs[argnum-1]);
				strncat(expandedstrs[expandedstrcount], ungottenchars, ungottencharcount);
				ungottencharcount = 0;
				expandedstrspos[expandedstrcount] = 0;
				expandedstrcount++;
				return getNextChar();
			}
			
			while(isWhitespace(n)) n = getNextChar();
			if(n == ';') {
				bool wascharliteral = charisliteral;
				charisliteral = true;
				while(getNextChar() != '\n') continue;
				charisliteral = wascharliteral;
				if(currentmacro == 0) linenumber++;
				return getNextChar();
			}
			if(n == '\n') {
				if(currentmacro == 0) linenumber++;
				char cc = getNextChar();
				return cc;
			}
			
			fseek(asmfile, savedasmfilepos, SEEK_SET);
			currentmacropos = savedcurrentmacropos;
			expandedstrcount = savedexpandedstrcount;
			for(unsigned int ii = 0; ii < savedexpandedstrcount; ii++) expandedstrspos[ii] = savedexpandedstrspos[ii];
			return '\\';
		}
	}
	
	
	return c;
}

enum asmopcode identifyOpcode(char* token) {
	enum asmopcode opcode;
	for(opcode = 0; opcode < OPCODES_COUNT; opcode++) {
		int i;
		for(i = 0; token[i] != '\0' && OPCODES[opcode][i] != '\0'; i++) {
			if(token[i] != OPCODES[opcode][i] && token[i]-'A'+'a' != OPCODES[opcode][i]) break;
		}
		if(token[i] == '\0' && OPCODES[opcode][i] == '\0') return opcode;
	}
	
	return INVALID_OPCODE;
}

enum asmdirective identifyDirective(char* token) {
	enum asmdirective directive;
	for(directive = 0; directive < DIRECTIVES_COUNT; directive++) {
		int i;
		for(i = 0; token[i] != '\0' && DIRECTIVES[directive][i] != '\0'; i++) {
			if(token[i] != DIRECTIVES[directive][i] && token[i]-'a'+'A' != DIRECTIVES[directive][i]) break;
		}
		if(token[i] == '\0' && DIRECTIVES[directive][i] == '\0') return directive;
	}
	
	return INVALID_DIRECTIVE;
}

enum asmregiontype identifyRegiontype(char* token) {
	enum asmregiontype type;
	for(type = 0; type < REGIONS_COUNT; type++) {
		int i;
		for(i = 0; token[i] != '\0' && REGIONS[type][i] != '\0'; i++) {
			if(token[i] != REGIONS[type][i] && token[i]-'a'+'A' != REGIONS[type][i]) break;
		}
		if(token[i] == '\0' && REGIONS[type][i] == '\0') return type;
	}
	
	return INVALID_REGION_TYPE;
}

enum asmdirective identifyPredef(char* token) {
	enum asmpredef predef;
	for(predef = 0; predef < PREDEFS_COUNT; predef++) {
		int i;
		for(i = 0; token[i] != '\0' && PREDEFS[predef][i] != '\0'; i++) {
			if(token[i] != PREDEFS[predef][i]) break;
		}
		if(token[i] == '\0' && PREDEFS[predef][i] == '\0') return predef;
	}
	
	return INVALID_PREDEF;
}

enum asmregister identifyRegister(char* token) {
	enum asmregister reg;
	for(reg = 0; reg < REGISTERS_COUNT; reg++) {
		int i;
		for(i = 0; token[i] != '\0' && REGISTERS[reg][i] != '\0'; i++) {
			if(token[i] != REGISTERS[reg][i] && token[i]-'A'+'a' != REGISTERS[reg][i]) break;
		}
		if(token[i] == '\0' && REGISTERS[reg][i] == '\0') return reg;
	}
	
	return INVALID_REGISTER;
}

enum asmcondition identifyCondition(char* token) {
	enum asmcondition condition;
	for(condition = 0; condition < CCS_COUNT; condition++) {
		int i;
		for(i = 0; token[i] != '\0' && CCS[condition][i] != '\0'; i++) {
			if(token[i] != CCS[condition][i] && token[i]-'A'+'a' != CCS[condition][i]) break;
		}
		if(token[i] == '\0' && CCS[condition][i] == '\0') return condition;
	}
	
	return INVALID_CONDITION;
}

struct token identifyNextToken() {
  if(debug) printf(".");
	char token[256];
	int i = 0;
	token[i] = getNextChar();
	
	// cross whitespace
	while(isWhitespace(token[i])) token[i] = getNextChar();
	
	// identify end
	if(token[i] == '\0') return (struct token) { .type = END_OF_FILE };
	
	// identify newline
	if(token[i] == ';') {
		bool wascharliteral = charisliteral;
		charisliteral = true;
		while(getNextChar() != '\n') continue;
		charisliteral = wascharliteral;
		if(currentmacro == 0) linenumber++;
		return (struct token) { .type = NEWLINE };
	}
	if(token[i] == '\n') {
		if(currentmacro == 0) linenumber++;
		return (struct token) { .type = NEWLINE };
	}
	
	if(isAlpha(token[i])) {
		while(isAlpha(token[i])) {
			token[++i] = getNextChar();
		}
		if(!isValidSymbolChar(token[i])) {
			ungetChar(token[i--]);
			token[i+1] = '\0';
			
			// identify opcode
			enum asmopcode opcode = identifyOpcode(token);
			if(opcode != INVALID_OPCODE) return (struct token) { .type = OPCODE, .content = opcode };
			
			// identify directive
			enum asmdirective directive = identifyDirective(token);
			if(directive != INVALID_DIRECTIVE) return (struct token) { .type = DIRECTIVE, .content = directive };
			
			// identify register
			enum asmregister reg = identifyRegister(token);
			if(reg != INVALID_REGISTER) return (struct token) { .type = REGISTER, .content = reg };
			
			// identify condition
			enum asmcondition condition = identifyCondition(token);
			if(condition != INVALID_CONDITION) return (struct token) { .type = CONDITION, .content = condition };
			
		}
	}
	if(isValidSymbolFirstChar(token[0])) {
		if(token[i] == '.' && !charisliteral) {
			token[++i] = getNextChar();
			if(token[i] == '.') {
				strcpy(&token[--i], locallabelscope);
				i += strlen(locallabelscope);
			} else {
				ungetChar(token[i--]);
				strcpy(&token[i], labelscope);
				i += strlen(labelscope);
			}
			token[i] = getNextChar();
			if(isValidSymbolChar(token[i])) {
				token[i+1] = token[i];
				token[i++] = '.';
			} else ungetChar(token[i--]);
		}
		unsigned int uniqueaffixpos = 256;
		while(isValidSymbolChar(token[i])) {
			token[++i] = getNextChar();
			if(token[i] == '\\') {
				token[i] = getNextChar();
				if(token[i] == '@') {
					uniqueaffixpos = i;
					continue;
				}
				else {
					ungetChar(token[i]);
					token[i] = '\\';
				}
			}
		}
		ungetChar(token[i--]);
		token[++i] = '\0';
		
		unsigned int uniqueaffixorig = uniqueaffixpos;
		if(uniqueaffixpos < 256) {
			if(firsttoken || indef) {
				token[uniqueaffixpos] = '0' - 1;
				newuniqueaffix = false;
			} else if(newuniqueaffix) {
				token[uniqueaffixpos] = '0' - 1;
			} else {
				uniqueaffixpos += 2;
				i += 2;
				for(unsigned int ii = i; ii > uniqueaffixpos; ii--) token[ii] = token[ii - 2];
				token[uniqueaffixpos-2] = '2';
				token[uniqueaffixpos-1] = '5';
				token[uniqueaffixpos]   = '6';
			}
		}
			
		// identify repeatsymbol
		if(strcmp(token, repeatsymbol) == 0) return (struct token) { .type = NUMBER, .content = repeatcount };
		
		// identify region type
		enum asmregiontype type = identifyRegiontype(token);
		if(type != INVALID_REGION_TYPE) return (struct token) { .type = REGION_TYPE, .content = type };
		
		// identify predefined symbol
		enum asmpredef predef = identifyPredef(token);
		if(predef != INVALID_PREDEF) return (struct token) { .type = NUMBER, .content = predeclaredsymbols[predef] };
		
		// identify text macro
		if(strcmp(token, "text_far") == 0) return (struct token) { .type = TEXT_MACRO };
		
		if(!charisliteral) {
			while(true) {
				if(uniqueaffixpos < 256) {
					if(firsttoken || indef || newuniqueaffix) {
						token[uniqueaffixpos]++;
						unsigned int overflowto = uniqueaffixpos;
						while(token[overflowto] == '9'+1) {
							token[overflowto] = '0';
							if(overflowto == uniqueaffixorig) {
								uniqueaffixpos++;
								i++;
								for(unsigned int ii = i; ii > uniqueaffixorig; ii--) token[ii] = token[ii-1];
								token[uniqueaffixorig] = '1';
							} else token[--overflowto]++;
						}
					} else {
						token[uniqueaffixpos]--;
						unsigned int underflowto = uniqueaffixpos;
						while(token[underflowto] == '0'-1) {
							token[underflowto] = '9';
							token[--underflowto]--;
							if(underflowto == uniqueaffixorig && token[underflowto] == '0') {
								for(unsigned int ii = uniqueaffixorig; ii < i; ii++) token[ii] = token[ii+1];
								uniqueaffixpos--;
								i--;
							}
						}
					}
				}
				
				unsigned int ii;
				
				// identify variable
				for(ii = 0; ii < variablecount; ii++) if(strcmp(token, varbls(ii).name) == 0) {
					if(!(firsttoken || indef || newuniqueaffix) || uniqueaffixpos >= 256) return (struct token) { .type = VARIABLE, .content = ii };
					else break;
				} if(ii < variablecount) continue;
				
				// identify label
				for(ii = 0; ii < labelcount; ii++) if(strcmp(token, labels(ii).name) == 0) {
					if(!(firsttoken || indef || newuniqueaffix) || uniqueaffixpos >= 256) return (struct token) { .type = LABEL, .content = ii };
					else break;
				} if(ii < labelcount) continue;
				
				// identify constant
				for(ii = 0; ii < constantcount; ii++) if(strcmp(token, consts(ii).name) == 0) {
					if(!(firsttoken || indef || newuniqueaffix) || uniqueaffixpos >= 256) return (struct token) { .type = CONSTANT, .content = ii };
					else break;
				} if(ii < constantcount) continue;
				
				// identify assumption
				for(ii = 0; ii < assumptioncount; ii++) if(strcmp(token, asmpts(ii).name) == 0) {
					if(!(firsttoken || indef || newuniqueaffix) || uniqueaffixpos >= 256) return (struct token) { .type = ASSUMPTION, .content = ii };
					else break;
				} if(ii < assumptioncount) continue;
				
				// identify macro
				for(ii = 0; ii < macrocount; ii++) if(strcmp(token, macros(ii).name) == 0) {
					if(!(firsttoken || indef || newuniqueaffix) || uniqueaffixpos >= 256) return (struct token) { .type = MACRO, .content = ii };
					else break;
				} if(ii < macrocount) continue;
			
				// identify constant string
				for(ii = 0; ii < conststrcount; ii++) if(strcmp(token, costrs(ii).name) == 0) {
					if(!(firsttoken || indef || newuniqueaffix) || uniqueaffixpos >= 256) {
						if(charisliteral || indef) return (struct token) { .type = CONSTANT_STRING, .content = ii };
						
						expandedstrs[expandedstrcount] = malloc(strlen(costrs(ii).content)+1 + ungottencharcount);
						strcpy(expandedstrs[expandedstrcount], costrs(ii).content);
						strncat(expandedstrs[expandedstrcount], ungottenchars, ungottencharcount);
						ungottencharcount = 0;
						expandedstrspos[expandedstrcount] = 0;
						expandedstrcount++;
						struct token returntoken = identifyNextToken();
						return returntoken;
					}
					else break;
				} if(ii < conststrcount) continue;
				
				if(uniqueaffixpos >= 256 || firsttoken || indef || newuniqueaffix || (uniqueaffixpos == uniqueaffixorig && token[uniqueaffixpos] == '0')) break;
			}
		}
		
		// identify new symbol
		strcpy(symbolstr, token);
		return (struct token) { .type = UNRECORDED_SYMBOL, .content = (unsigned int) symbolstr };
	}
	
	// identify number
	if(isNumber(token[i])) {
		// decimal number
		unsigned long value = 0;
		while(isNum(token[i])) {
			if(token[i] != '_') {
				value *= 10;
				value += token[i] - '0';
			}
			token[++i] = getNextChar();
		}
		ungetChar(token[i--]);
		token[i+1] = '\0';
		return (struct token) { .type = NUMBER, .content = value };
	}
	if(token[i] == '$') {
		// hexadecimal number
		token[++i] = getNextChar();
		if(isHex(token[i])) {
			unsigned long value = 0;
			while(isHex(token[i])) {
				if(token[i] != '_') {
					value *= 0x10;
					if(isNumber(token[i]))         value += token[i] - '0';
					else if(isUppercase(token[i])) value += token[i] - 'A' + 0xA;
					else if(isLowercase(token[i])) value += token[i] - 'a' + 0xa;
				}
				token[++i] = getNextChar();
			}
			ungetChar(token[i--]);
			token[i+1] = '\0';
			return (struct token) { .type = NUMBER, .content = value };
		}
		ungetChar(token[i--]);
	}
	if(token[i] == '%') {
		// binary number
		token[++i] = getNextChar();
		if(token[i] == bin0 || token[i] == bin1 || token[i] == '_') {
			unsigned long value = 0;
			while(true) {
				if(token[i] == bin0) {
					value <<= 1;
				} else if(token[i] == bin1) {
					value <<= 1;
					value += 1;
				} else if(token[i] != '_') break;
				token[++i] = getNextChar();
			}
			ungetChar(token[i--]);
			token[i+1] = '\0';
			return (struct token) { .type = NUMBER, .content = value };
		}
		ungetChar(token[i--]);
	}
	if(token[i] == '&') {
		// octal number
		token[++i] = getNextChar();
		if(isOct(token[i])) {
			unsigned long value = 0;
			while(isOct(token[i])) {
				if(token[i] != '_') {
					value *= 010;
					value += token[i] - '0';
				}
				token[++i] = getNextChar();
			}
			ungetChar(token[i--]);
			token[i+1] = '\0';
			return (struct token) { .type = NUMBER, .content = value };
		}
		ungetChar(token[i--]);
	}
	if(token[i] == '`') {
		// game boy graphics number
		token[++i] = getNextChar();
		if(isQua(token[i])) {
			unsigned long valuel = 0;
			unsigned long valueh = 0;
			while(isQua(token[i-1])) {
				if(token[i] != '_') {
					valuel << 1;
					valuel += (token[i] - '0') & 0x1;
					valueh << 1;
					valueh += (token[i] - '0') >> 1;
				}
				token[++i] = getNextChar();
			}
			ungetChar(token[i--]);
			token[i+1] = '\0';
			return (struct token) { .type = NUMBER, .content = (valuel) | (valueh << 4)};
		}
		ungetChar(token[i--]);
	}
	
	// identify string
	if(token[i] == '\"') {
		symbolstr[0] = getNextChar();
		unsigned int ii = 0;
		while(symbolstr[ii] != '\"') {
			if(symbolstr[ii] == '\\') {
				bool wascharliteral = charisliteral;
				charisliteral = true;
				symbolstr[ii] = getNextChar();
				if(symbolstr[ii] == '@') {
					symbolstr[++ii] = '@';
					symbolstr[ii-1] = '\\';
				}
				charisliteral = wascharliteral;
			}
			symbolstr[++ii] = getNextChar();
		}
		symbolstr[ii] = '\0';
		return (struct token) { .type = STRING, .content = (int) symbolstr };
	}
	// identify at
	if(token[i] == '@') return (struct token) { .type = NUMBER, .content = mempos&0xFFFF };
	
	// identify comma
	if(token[i] == ',') return (struct token) { .type = COMMA };
	
	// identify memory
	if(token[i] == '[') return (struct token) { .type = MEMORY_OPEN };
	if(token[i] == ']') return (struct token) { .type = MEMORY_CLOSE };
	
	// identify operator
	unsigned int matchcount = 0;
	unsigned int lastmatch;
	for(unsigned int ii = 0; ii < OPERATORS_COUNT; ii++) {
		if(token[i] == OPERATORS[ii][0]) {
			matchcount++;
			if(strlen(OPERATORS[ii]) == 1) lastmatch = ii;
		}
	}
	if(matchcount > 0) {
		while(matchcount > 1) {
			matchcount = 0;
			token[++i] = getNextChar();
			for(unsigned int ii = 0; ii < OPERATORS_COUNT; ii++) {
				unsigned int iii;
				for(iii = 0; iii <= i; iii++) {
					if(token[iii] != OPERATORS[ii][iii]) {
						break;
					}
				}
				if(iii > i) {
					matchcount++;
					if(iii == strlen(OPERATORS[ii])) lastmatch = ii;
				}
			}
		}
		if(matchcount == 0) {
			while(i >= strlen(OPERATORS[lastmatch])) ungetChar(token[i--]);
		}
		return (struct token) { .type = OPERATOR, .content = lastmatch };
	}
	
	if(feof(asmfile) != 0) return (struct token) { .type = END_OF_FILE };
	
	
	
  if(debug) printf("?%c ", token[0]);
	return (struct token) { .type = UNKNOWN_TOKEN };
}



int recordLabel(char* name) {
// if(strcmp(name, "SFX_Caught_Mon") == 0) {
// 	printLocation();
// 	printf("\n%s is defined as %0X\n", name, mempos);
// 	return -1;
// }
	labels(labelcount) = (struct label) { .name = name, .value = mempos};
	incrementLabellist;
}

// int recordAssumptionL(char* name, unsigned char value) {
// 	unsigned int i;
// 	for(i = 0; i < assumptionCount; i++) if(strcmp(asmpts(i).name, name) == 0) break;
	
// 	if(asmpts[i].l && asmpts[i].valuel != value) {
// 		errorInconsistentAssumption(name, value, asmpts[i].value);
// 		return -1;
// 	}
	
// 	asmpts[i].l = true;
// 	asmpts[i].valuel = value;
	
// 	if(i == assumptionCount) {
// 		asmpts[i].name = malloc(strlen(name)+1);
// 		strcpy(asmpts[i].name, name);
// 		assumptioncount++;
// 		if(assumptioncount % 1024 == 0) assumptionlist[assumptioncount/1024] = malloc(1024 * sizeof(assumptionlist[assumptioncount/1024][0]));
// 	}
	
// 	return 0;
// }

// int recordAssumptionH(char* name, unsigned char value) {
// 	unsigned int i;
// 	for(i = 0; i < assumptionCount; i++) if(strcmp(asmpts(i).name, name) == 0) break;
	
// 	if(asmpts[i].h && asmpts[i].valueh != value) {
// 		errorInconsistentAssumption(name, value, asmpts[i].value);
// 		return -1;
// 	}
	
// 	asmpts[i].h = true;
// 	asmpts[i].valueh = value;
	
// 	if(i == assumptionCount) {
// 		asmpts[i].name = malloc(strlen(name)+1);
// 		strcpy(asmpts[i].name, name);
// 		assumptioncount++;
// 		if(assumptioncount % 1024 == 0) assumptionlist[assumptioncount/1024] = malloc(1024 * sizeof(assumptionlist[assumptioncount/1024][0]));
// 	}
	
// 	return 0;
// }

// int recordAssumptionB(char* name, unsigned char value) {
// 	unsigned int i;
// 	for(i = 0; i < assumptionCount; i++) if(strcmp(asmpts(i).name, name) == 0) break;
	
// 	if(asmpts[i].b && asmpts[i].valueb != value) {
// 		errorInconsistentAssumption(name, value, asmpts[i].value);
// 		return -1;
// 	}
	
// 	asmpts[i].b = true;
// 	asmpts[i].valueb = value;
	
// 	if(i == assumptionCount) {
// 		asmpts[i].name = malloc(strlen(name)+1);
// 		strcpy(asmpts[i].name, name);
// 		assumptioncount++;
// 		if(assumptioncount % 1024 == 0) assumptionlist[assumptioncount/1024] = malloc(1024 * sizeof(assumptionlist[assumptioncount/1024][0]));
// 	}
	
// 	return 0;
// }




unsigned char* mapstring(char* string) {
	unsigned char data[1024];
	unsigned int datalength = 0;
	for(unsigned int i = 0; string[i] != '\0';) {
		unsigned int longestmatch;
		unsigned int longestmatchlength = 0;
		for(unsigned int ii = 0; ii < charmapcount; ii++) {
			for(unsigned int iii = 0; string[i+iii] == charms(ii).chars[iii]; iii++) {
				if(charms(ii).chars[iii+1] == '\0') {
					if(iii+1 > longestmatchlength) {
						longestmatch = ii;
						longestmatchlength = iii+1;
					}
					break;
				}
			}
		}
		if(longestmatchlength == 0) data[datalength++] = string[i++];
		else {
			data[datalength++] = charms(longestmatch).value;
			i += longestmatchlength;
		}
	}
	unsigned char* returndata = malloc(datalength+1);
	for(unsigned int i = 0; i < datalength; i++) returndata[i] = data[i];
	returndata[datalength] = '\0';
	return returndata;
}



struct expressionpart {
	bool unknown;
	long value;
	bool negative;
	bool complemented;
	bool not;
	enum asmoperator operation;
};

int processExpression(struct expressionpart** returnexpression, unsigned int numbits) {
	struct expressionpart expression[1024];
	unsigned int expressionsize;
	unsigned int parenthesisdepth = 0;
	int uncalculable = 0;
	
	for(unsigned int i = 0; i==0?true : expression[i-1].operation != END_OF_EXPRESSION; i++) {
		expression[i].unknown = false;
		expression[i].negative = false;
		expression[i].complemented = false;
		expression[i].not = false;
		
		struct token token = identifyNextToken();
		
		// determine negative and complemented
		while(token.type == OPERATOR) {
			if(token.content == OPEN_PARENTHESIS) break;
			else if(token.content == COMPLEMENT) expression[i].complemented ^= true;
			else if(token.content == OPERATION_ADD) expression[i].negative ^= false;
			else if(token.content == SUBTRACT) expression[i].negative ^= true;
			else if(token.content == BOOLEAN_NOT) expression[i].not ^= true;
			else {
				errorUnexpectedToken(token);
				return -1;
			}
			token = identifyNextToken();
		}
		
		// determine OPEN_PARENTHESIS operation
		if(token.type == OPERATOR && token.content == OPEN_PARENTHESIS) {
			parenthesisdepth++;
			expression[i].operation = OPEN_PARENTHESIS;
			continue;
		}
		
		
		switch(token.type) {
		case STRING:
			expression[i].value = 0;
			unsigned char* strdata = mapstring((char*)token.content);
			if(strlen(strdata) > numbits/8) {
				errorValueTooLarge(numbits);
				return -1;
			}
			for(int ii = strlen(strdata)-1; ii >= 0; ii--) {
				expression[i].value <<= 8;
				expression[i].value |= strdata[ii];
			}
			free(strdata);
			break;
		
		
		case LABEL:
			expression[i].value = labels(token.content).value & 0xFFFF;
			break;
		case CONSTANT:
			expression[i].value = consts(token.content).value;
			break;
		case VARIABLE:
			expression[i].value = varbls(token.content).value;
			break;
		
		case ASSUMPTION:
			if(numbits <= 8 && asmpts(token.content).l) {
				expression[i].value = asmpts(token.content).valuel;
				break;
			} else if(numbits <= 16 && asmpts(token.content).l && asmpts(token.content).m) {
				expression[i].value = asmpts(token.content).valuel | (asmpts(token.content).valuem << 8);
				break;
			} else if(asmpts(token.content).l && asmpts(token.content).m && asmpts(token.content).h) {
				expression[i].value = asmpts(token.content).valuel | (asmpts(token.content).valuem << 8) | (asmpts(token.content).h << 16);
				break;
			}
			for(int ii = 0; ii < i; ii++) if(expression[ii].unknown = true) uncalculable = 1;
			expression[i].unknown = true;
			expression[i].value = token.content;
			break;
		
		case UNRECORDED_SYMBOL:
		// determine count of unknowns
			for(int ii = 0; ii < i; ii++) if(expression[ii].unknown = true) uncalculable = 1;
			expression[i].unknown = true;
			expression[i].value = token.content;
			break;
		
		
		case PREDECLARED_SYMBOL:
			expression[i].value = predeclaredsymbols[token.content];
			break;
		
		
		case DIRECTIVE: {
			unsigned int dirtype = token.content;
			token = assertNextToken(OPERATOR, OPEN_PARENTHESIS);
			
			switch(dirtype) {
			
			case DIR_BANK:
			// determine bank
				token = identifyNextToken();
				switch(token.type) {
				case LABEL:
					expression[i].value = labels(token.content).value >> 16 & 0xFF;
					break;
				case ASSUMPTION:
					if(asmpts(token.content).h) {
						expression[i].value = asmpts(token.content).valueh;
						break;
					}
				case UNRECORDED_SYMBOL:
				case STRING:
					uncalculable = 1;
					break;
					
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			
			case DIR_HIGH: {
			// insert (((n) & 0xFF00) >> 8)
				struct expressionpart* insertion = 0;
				int procresult = processExpression(&insertion, 24);
				if(procresult == 1) uncalculable = 1;
				else if(procresult != 0) return -1;
				
				unsigned int highstart = i;
				expression[i++].operation = OPEN_PARENTHESIS;
				expression[i++].operation = OPEN_PARENTHESIS;
				expression[i++].operation = OPEN_PARENTHESIS;
				for(unsigned int ii = 0; expression[i-1].operation != END_OF_EXPRESSION; ii++) expression[i++] = insertion[ii];
				free(insertion);
				expression[i-1].operation = CLOSE_PARENTHESIS;
				expression[i++].operation = BINARY_AND;
				expression[i].value = 0xFF00;
				expression[i++].operation = CLOSE_PARENTHESIS;
				expression[i++].operation = SHIFT_RIGHT;
				expression[i].value = 8;
				expression[i++].operation = CLOSE_PARENTHESIS;
				for(unsigned int ii = highstart+1; ii < i; ii++) {
					expression[ii].unknown = false;
					expression[ii].negative = false;
					expression[ii].complemented = false;
					expression[ii].not = false;
					if(ii - highstart == 2) ii = i - 4;
				}
				break;}
			
			case DIR_LOW: {
			// insert ((n) & 0xFF)
				struct expressionpart* insertion = 0;
				int procresult = processExpression(&insertion, 24);
				if(procresult == 1) uncalculable = 1;
				else if(procresult != 0) return -1;
				
				unsigned int lowstart = i;
				expression[i++].operation = OPEN_PARENTHESIS;
				expression[i++].operation = OPEN_PARENTHESIS;
				for(unsigned int ii = 0; expression[i-1].operation != END_OF_EXPRESSION; ii++) expression[i++] = insertion[ii];
				free(insertion);
				expression[i-1].operation = CLOSE_PARENTHESIS;
				expression[i++].operation = BINARY_AND;
				expression[i].value = 0xFF;
				expression[i++].operation = CLOSE_PARENTHESIS;
				for(unsigned int ii = lowstart+1; ii < i; ii++) {
					expression[ii].unknown = false;
					expression[ii].negative = false;
					expression[ii].complemented = false;
					expression[ii].not = false;
					if(ii - lowstart == 1) ii = i - 2;
				}
				break;}
			
			case DIR_SIZEOF:
				token = identifyNextToken();
				switch(token.type) {
				case STRING:
					uncalculable = 1;
					break;
				case REGION_TYPE:
					expression[i].value = REGION_SIZES[token.content];
					break;
				}
				break;
			
			case DIR_STARTOF:
				token = identifyNextToken();
				switch(token.type) {
				case STRING:
					uncalculable = 1;
					break;
				case REGION_TYPE:
					expression[i].value = REGION_ADDRESSES[token.content];
					break;
				}
				break;
				
			case DIR_DEF:
			// determine definition
				indef = true;
				token = identifyNextToken();
				indef = false;
				switch(token.type) {
				case NUMBER:
				case CONSTANT:
				case CONSTANT_STRING:
				case VARIABLE:
				case MACRO:
				case LABEL:
					expression[i].value = 1;
					break;
				case ASSUMPTION:
				case UNRECORDED_SYMBOL:
					expression[i].value = 0;
					break;
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			
			case DIR_STRIN: {
				token = assertNextTokenType(STRING);
				char str1[strlen((char*)token.content)+1];
				strcpy(str1, (char*)token.content);
				
				token = assertNextTokenType(COMMA);
				
				token = assertNextTokenType(STRING);
				char str2[strlen((char*)token.content)+1];
				strcpy(str1, (char*)token.content);
				
				unsigned int strinpos = 0;
				for(unsigned int i = 0; str1[i] != '\0'; i++) {
					unsigned int ii = 0;
					while(str2[ii] == str1[i+ii]) ii++;
					if(str2[ii] == '\0') {
						strinpos = i;
						break;
					}
				}
				
				expression[i].value = strinpos;
				
				break;}
			
			case DIR_STRFIND: {
				token = assertNextTokenType(STRING);
				char str1[strlen((char*)token.content)+1];
				strcpy(str1, (char*)token.content);
				
				token = assertNextTokenType(COMMA);
				
				token = assertNextTokenType(STRING);
				char str2[strlen((char*)token.content)+1];
				strcpy(str1, (char*)token.content);
				
				int strinpos = -1;
				for(unsigned int i = 0; str1[i] != '\0'; i++) {
					unsigned int ii = 0;
					while(str2[ii] == str1[i+ii]) ii++;
					if(str2[ii] == '\0') {
						strinpos = i;
						break;
					}
				}
				
				expression[i].value = strinpos;
				
				break;}
			
			case DIR_STRCMP: {
				token = assertNextTokenType(STRING);
				char str1[strlen((char*)token.content)+1];
				strcpy(str1, (char*)token.content);
				
				token = assertNextTokenType(COMMA);
				
				token = assertNextTokenType(STRING);
				
				int value = strcmp(str1, (char*)token.content);
				if(value > 0) expression[i].value = 1;
				else if(value < 0) expression[i].value = -1;
				else expression[i].value = 0;
				
				break;}
			
			default:
				errorUnexpectedToken((struct token) { .type = DIRECTIVE, .content = dirtype });
				return -1;
			}
			
			token = assertNextToken(OPERATOR, CLOSE_PARENTHESIS);
			
			break;}
		
		
		case NUMBER:
		// determine value
			expression[i].value = token.content;
			break;
		
		
		default:
			errorUnexpectedToken(token);
			return -1;
		}
		
		// determine operation
		while(true) {
			saveAsmPos();
			token = identifyNextToken();
			if(token.type == OPERATOR) {
				expression[i].operation = token.content;
				if(token.content == CLOSE_PARENTHESIS) {
					if(parenthesisdepth == 0) {
						expression[i].operation = END_OF_EXPRESSION;
						restoreAsmPos();
						expressionsize = i + 1;
						break;
					}
					parenthesisdepth--;
					i++;
					continue;
				}
			} else {
				expression[i].operation = END_OF_EXPRESSION;
				restoreAsmPos();
				expressionsize = i + 1;
			}
			break;
		}
	}
	
	if(parenthesisdepth != 0) {
		errorUnbalancedParentheses();
		return -1;
	}
	
	*returnexpression = malloc(expressionsize * sizeof(struct expressionpart));
	for(int i = 0; i < expressionsize; i++) {
		(*returnexpression)[i] = expression[i];
	}
	
	return uncalculable;
}

int calculateExpression(long* result) {
	struct expressionpart* expression = 0;
	
	// build the expression from asmfile
	int procresult = processExpression(&expression, 24);
	if(procresult == 1) {
		if(expression != 0) free(expression);
		return 1;
	} else if(procresult != 0) {
		if(expression != 0) free(expression);
		return -1;
	}
	
// if(debug) {
// 	printf("\n\n");
// 	unsigned int ii;
// 	for(ii = 0; expression[ii].operation != END_OF_EXPRESSION; ii++) {
// 		if(expression[ii].operation == OPEN_PARENTHESIS) printf("%s ", OPERATORS[expression[ii].operation]);
// 		else if(ii > 0 && expression[ii-1].operation == CLOSE_PARENTHESIS) printf("%s ", OPERATORS[expression[ii].operation]);
// 		else if(expression[ii].unknown) printf("? %s ", OPERATORS[expression[ii].operation]);
// 		else printf("%0X %s ", expression[ii].value, OPERATORS[expression[ii].operation]);
// 	}
// 	if(!(ii > 0 && expression[ii-1].operation == CLOSE_PARENTHESIS)) printf("%0X", expression[ii].value);
// 	printf("\n\n");
// }
	// calculate
	unsigned int i = 0;
	while(true) {
		if(expression[i].unknown || (expression[i].operation != END_OF_EXPRESSION && expression[i+1].unknown)) return 1;
		
		
		if(expression[i].operation == OPEN_PARENTHESIS) {
			if(expression[i+1].operation == CLOSE_PARENTHESIS) {
				expression[i].unknown = expression[i+1].unknown;
				expression[i].value = expression[i+1].value;
				expression[i].negative ^= expression[i+1].negative;
				expression[i].complemented ^= expression[i+1].complemented;
				expression[i].not ^= expression[i+1].not;
				expression[i].operation = expression[i+2].operation;
				for(unsigned int ii = i+1; expression[ii].operation != END_OF_EXPRESSION; ii++) {
					expression[ii] = expression[ii+2];
				}
				i = 0;
			}
			
			else i++;
			continue;
		}
		
		
		if(expression[i].operation == CLOSE_PARENTHESIS) {
			i++;
			if(expression[i].operation != END_OF_EXPRESSION) i++;
			continue;
		}
		
		
		if(expression[i].operation == END_OF_EXPRESSION) {
			if(i == 0) {
				*result = expression[i].value;
				if(expression[0].negative) *result = -(*result);
				if(expression[0].complemented) *result = ~(*result);
				if(expression[0].not) {
					*result = (*result) == 0 ? 1 : 0;
				}
				free(expression);
				return 0;
			}
			i = 0;
			continue;
		}
		
		
		else if(OPERATOR_PRIORITY[expression[i].operation] >= OPERATOR_PRIORITY[expression[i+1].operation]) {
			long operand1 = expression[i].value;
			if(expression[i].negative) operand1 = -operand1;
			if(expression[i].complemented) operand1 = ~operand1;
			if(expression[i].not) operand1 = operand1 == 0 ? 1 : 0;
			
			long operand2 = expression[i+1].value;
			if(expression[i+1].negative) operand2 = -operand2;
			if(expression[i+1].complemented) operand2 = ~operand2;
			if(expression[i+1].not) operand2 = operand2 == 0 ? 1 : 0;
			
			switch(expression[i].operation) {
			case EXPONENT:
				expression[i].value = pow(expression[i].value, operand2);
				goto retainsign;
			case MULTIPLY:
				expression[i].value = operand1 * operand2;
				break;
			case DIVIDE:
				expression[i].value = operand1 / operand2;
				break;
			case MODULO:
				expression[i].value = operand1 % operand2;
				break;
			case SHIFT_LEFT:
				expression[i].value = operand1 << operand2;
				break;
			case SHIFT_RIGHT:
				expression[i].value = operand1 >> operand2;
				break;
			case UNSIGNED_SHIFT_RIGHT:
				expression[i].value = (unsigned long) operand1 >> operand2;
				break;
			case BINARY_AND:
				expression[i].value = operand1 & operand2;
				break;
			case BINARY_OR:
				expression[i].value = operand1 | operand2;
				break;
			case BINARY_XOR:
				expression[i].value = operand1 ^ operand2;
				break;
			case OPERATION_ADD:
				expression[i].value = operand1 + operand2;
				break;
			case SUBTRACT:
				expression[i].value = operand1 - operand2;
				break;
			case EQUAL_TO:
				expression[i].value = operand1 == operand2 ? 1 : 0;
				break;
			case NOT_EQUAL_TO:
				expression[i].value = operand1 != operand2 ? 1 : 0;
				break;
			case LESS_THAN_OR_EQUAL_TO:
				expression[i].value = operand1 <= operand2 ? 1 : 0;
				break;
			case GREATER_THAN_OR_EQUAL_TO:
				expression[i].value = operand1 >= operand2 ? 1 : 0;
				break;
			case LESS_THAN:
				expression[i].value = operand1 < operand2 ? 1 : 0;
				break;
			case GREATER_THAN:
				expression[i].value = operand1 > operand2 ? 1 : 0;
				break;
			case BOOLEAN_AND:
				expression[i].value = operand1 && operand2 ? 1 : 0;
				break;
			case BOOLEAN_OR:
				expression[i].value = operand1 || operand2 ? 1 : 0;
				break;
			default:
				printf("unexpected operator %s", OPERATORS[expression[i].operation]);
				free(expression);
				return -1;
			}
			expression[i].negative = false;
			expression[i].complemented = false;
			expression[i].not = false;
			
			retainsign:
			expression[i].operation = expression[i+1].operation;
			for(int ii = i+1; expression[ii].operation != END_OF_EXPRESSION; ii++) {
				expression[ii] = expression[ii+1];
			}
			
			i = 0;
		}
		
		
		else i++;
	}
}

int calculateExpressionWithAssumption(unsigned long romdata, unsigned int numbits, long* result) {
	struct expressionpart* expression = 0;
	
	// build the expression from asmfile
	int procresult = processExpression(&expression, numbits);
	if(procresult == 1) {
		if(expression != 0) free(expression);
		*result = romdata;
		return 0;
	} else if(procresult != 0) {
		if(expression != 0) free(expression);
		return -1;
	}
	
// if(debug) {
// 	printf("\n\n");
// 	unsigned int ii;
// 	for(ii = 0; expression[ii].operation != END_OF_EXPRESSION; ii++) {
// 		if(expression[ii].operation == OPEN_PARENTHESIS) printf("%s ", OPERATORS[expression[ii].operation]);
// 		else if(ii > 0 && expression[ii-1].operation == CLOSE_PARENTHESIS) printf("%s ", OPERATORS[expression[ii].operation]);
// 		else printf("%0X %s ", expression[ii].value, OPERATORS[expression[ii].operation]);
// 	}
// 	if(!(ii > 0 && expression[ii-1].operation == CLOSE_PARENTHESIS)) printf("%0X", expression[ii].value);
// 	printf("\n\n");
// }
	// resolve all calculable parts
	unsigned int i = 0;
	unsigned int expressionend = 1024;
	while(true) {
		if(expression[i].operation == OPEN_PARENTHESIS) {
			if(expression[i+1].operation == CLOSE_PARENTHESIS) {
				expression[i].unknown = expression[i+1].unknown;
				expression[i].value = expression[i+1].value;
				expression[i].negative ^= expression[i+1].negative;
				expression[i].complemented ^= expression[i+1].complemented;
				expression[i].not ^= expression[i+1].not;
				expression[i].operation = expression[i+2].operation;
				for(unsigned int ii = i+1; expression[ii].operation != END_OF_EXPRESSION; ii++) {
					expression[ii] = expression[ii+2];
				}
				i = 0;
			}
			
			else i++;
			continue;
		}
		
		
		if(expression[i].operation == END_OF_EXPRESSION) {
			if(expressionend == i) {
				if(expressionend == 0 && !expression[i].unknown) {
					*result = expression[i].value;
					if(expression[0].negative) *result = -(*result);
					if(expression[0].complemented) *result = ~(*result) & (~(~0 << numbits));
					if(expression[0].not) *result = (*result) == 0 ? 1 : 0;
					free(expression);
					return 0;
				}
				break;
			}
			expressionend = i;
			i = 0;
			continue;
		}
		
		
		if(expression[i].operation == CLOSE_PARENTHESIS) {
			i++;
			if(expression[i].operation != END_OF_EXPRESSION) i++;
			continue;
		}
		if(expression[i].unknown || expression[i+1].unknown) {
			i++;
			continue;
		}
			
		
		else if(OPERATOR_PRIORITY[expression[i].operation] >= OPERATOR_PRIORITY[expression[i+1].operation]) {
			long operand1 = expression[i].value;
			if(expression[i].negative) operand1 = -operand1;
			if(expression[i].complemented) operand1 = ~operand1 & (~(~0 << numbits));
			if(expression[i].not) operand1 = operand1 == 0 ? 1 : 0;
			
			long operand2 = expression[i+1].value;
			if(expression[i+1].negative) operand2 = -operand2;
			if(expression[i+1].complemented) operand2 = ~operand2 & (~(~0 << numbits));
			if(expression[i+1].not) operand2 = operand2 == 0 ? 1 : 0;
			
			switch(expression[i].operation) {
			case EXPONENT:
				expression[i].value = pow(expression[i].value, operand2);
				goto retainsign;
			case MULTIPLY:
				expression[i].value = operand1 * operand2;
				break;
			case DIVIDE:
				expression[i].value = operand1 / operand2;
				break;
			case MODULO:
				expression[i].value = operand1 % operand2;
				break;
			case SHIFT_LEFT:
				expression[i].value = operand1 << operand2;
				break;
			case SHIFT_RIGHT:
				expression[i].value = operand1 >> operand2;
				break;
			case UNSIGNED_SHIFT_RIGHT:
				expression[i].value = (unsigned long) operand1 >> operand2;
				break;
			case BINARY_AND:
				expression[i].value = operand1 & operand2;
				break;
			case BINARY_OR:
				expression[i].value = operand1 | operand2;
				break;
			case BINARY_XOR:
				expression[i].value = operand1 ^ operand2;
				break;
			case OPERATION_ADD:
				expression[i].value = operand1 + operand2;
				break;
			case SUBTRACT:
				expression[i].value = operand1 - operand2;
				break;
			case EQUAL_TO:
				expression[i].value = operand1 == operand2 ? 1 : 0;
				break;
			case NOT_EQUAL_TO:
				expression[i].value = operand1 != operand2 ? 1 : 0;
				break;
			case LESS_THAN_OR_EQUAL_TO:
				expression[i].value = operand1 <= operand2 ? 1 : 0;
				break;
			case GREATER_THAN_OR_EQUAL_TO:
				expression[i].value = operand1 >= operand2 ? 1 : 0;
				break;
			case LESS_THAN:
				expression[i].value = operand1 < operand2 ? 1 : 0;
				break;
			case GREATER_THAN:
				expression[i].value = operand1 > operand2 ? 1 : 0;
				break;
			case BOOLEAN_AND:
				expression[i].value = operand1 && operand2 ? 1 : 0;
				break;
			case BOOLEAN_OR:
				expression[i].value = operand1 || operand2 ? 1 : 0;
				break;
			default:
				printf("unexpected operator %s", OPERATORS[expression[i].operation]);
				free(expression);
				return -1;
			}
			expression[i].negative = false;
			expression[i].complemented = false;
			expression[i].not = false;
			
			retainsign:
			expression[i].operation = expression[i+1].operation;
			for(int ii = i+1; expression[ii].operation != END_OF_EXPRESSION; ii++) {
				expression[ii] = expression[ii+1];
			}
			
			i = 0;
		}
		
		
		else i++;
	}
	
	
	
	long assumption = romdata;
	
	while(!expression[0].unknown || expression[0].operation != END_OF_EXPRESSION) {
		
		unsigned int expressionlength;
		for(expressionlength = 0; expression[expressionlength].operation != END_OF_EXPRESSION; expressionlength++) {}
		if(expression[0].operation == OPEN_PARENTHESIS && expressionlength > 0 && expression[expressionlength-1].operation == CLOSE_PARENTHESIS) {
			if(expression[0].negative) assumption = -assumption;
			if(expression[0].complemented) assumption = ~assumption & (~(~0 << numbits));
			if(expression[0].not) assumption = assumption == 0 ? 1 : 0;
			for(int ii = 0; expression[ii].operation != END_OF_EXPRESSION; ii++) {
				expression[ii] = expression[ii+1];
			}
			expression[expressionlength-2].operation = expression[expressionlength-1].operation;
			continue;
		}
		
		unsigned int operationpos = 0;
		unsigned int valuepos = 0;
		unsigned int parenthesisdepth = 0;
		bool rightside = false;
		unsigned int ii;
		for(ii = 0; expression[ii].operation != END_OF_EXPRESSION; ii++) {
			if(expression[ii].unknown) rightside = true;
			if(expression[ii].operation == OPEN_PARENTHESIS) parenthesisdepth++;
			else if(expression[ii].operation == CLOSE_PARENTHESIS) {
				if(parenthesisdepth == 0) {
					free(expression);
					printf("Unbalanced parentheses\n");
					return -1;
				}
				parenthesisdepth--;
			} else if(parenthesisdepth == 0 && OPERATOR_PRIORITY[expression[ii].operation] <= OPERATOR_PRIORITY[expression[ii].operation]) {
				operationpos = ii;
				valuepos = ii;
				if(rightside) valuepos++;
			}
		}
		
		long operand = expression[valuepos].value;
		if(expression[valuepos].negative) operand = -operand;
		if(expression[valuepos].complemented) operand = ~operand & (~(~0 << numbits));
		if(expression[valuepos].not) operand == 0 ? 1 : 0;
		
		switch(expression[operationpos].operation) {
		case EXPONENT:
			assumption = round(pow(assumption, 1./operand));
			goto retainsign;
		case MULTIPLY:
			assumption /= operand;
			break;
		case DIVIDE:
			// if(valuepos > operationpos) {
			// 	minassumption *= operand;
			// 	maxassumption = maxassumption * operand + operand - 1;
			// } else {
			// 	minassumption = operand / minassumption;
			// 	maxassumption = operand / maxassumption;
			// 	if(maxassumption == 0) {
			// 		free(expression);
			// 		printf("Cannot divide by 0\n");
			// 		return -1;
			// 	} if(minassumption == 0) maxassumption > 0 ? minassumption++ : minassumption--;
			// }
			// break;
		case MODULO:
		case SHIFT_LEFT:
		case SHIFT_RIGHT:
		case UNSIGNED_SHIFT_RIGHT:
		case BINARY_AND:
		case BINARY_OR:
			// the label can't be guessed with this operation, so just return the expected data
			free(expression);
			*result = romdata;
			return 0;
		case BINARY_XOR:
			assumption ^= operand;
			break;
		case OPERATION_ADD:
			assumption -= operand;
			break;
		case SUBTRACT:
			if(valuepos > operationpos) {
				assumption += operand;
			} else {
				assumption -= operand;
				expression[operationpos+1].negative ^= 1;
			}
			break;
		default:
			free(expression);
			printf("unexpected operator %s", OPERATORS[expression[operationpos].operation]);
			return -1;
		}
		
		expression[operationpos].operation = expression[valuepos].operation;
		for(int ii = valuepos; expression[ii].operation != END_OF_EXPRESSION; ii++) {
			expression[ii] = expression[ii+1];
		}
	}
	
	if(expression[0].negative) assumption = -assumption;
	if(expression[0].complemented) assumption = ~assumption & (~(~0 << numbits));
	if(expression[0].not) assumption = assumption == 0 ? 1 : 0;
	
	
	// record the new label assumtion
	if((char*)expression[0].value == symbolstr) {
		asmpts(assumptioncount).name = malloc(strlen((char*)expression[0].value)+1);
		strcpy(asmpts(assumptioncount).name, (char*)expression[0].value);
		if(numbits >= 8) {
			asmpts(assumptioncount).valuel = assumption & 0xFF;
			asmpts(assumptioncount).l = true;
		} else asmpts(assumptioncount).l = false;
		if(numbits >= 16) {
			asmpts(assumptioncount).valuem = (assumption >> 8) & 0xFF;
			asmpts(assumptioncount).m = true;
		} else asmpts(assumptioncount).m = false;
		if(numbits >= 24) {
		asmpts(assumptioncount).valueh = (assumption >> 16) & 0xFF;
		asmpts(assumptioncount).h = true;
		} else asmpts(assumptioncount).h = false;
if(strcmp(asmpts(assumptioncount).name, "SFX_Caught_Mon") == 0) {
	printLocation();
	printf("\n%s is assumed as %0X\n", asmpts(assumptioncount).name, asmpts(assumptioncount).valuel | (asmpts(assumptioncount).valuem << 8) | (asmpts(assumptioncount).valueh << 16));
	return -1;
}
		incrementAssumptionlist;
	} else {
		if(numbits >= 8) {
			if(asmpts(expression[0].value).l) {
				if((assumption & 0xFF) != asmpts(expression[0].value).valuel) {
					errorInconsistentAssumption(asmpts(expression[0].value).name, assumption, 0
						| (asmpts(expression[0].value).l ? asmpts(expression[0].value).valuel : 0)
						| (asmpts(expression[0].value).m ? asmpts(expression[0].value).valuem << 8 : 0)
						| (asmpts(expression[0].value).h ? asmpts(expression[0].value).valueh << 16 : 0));
					return -1;
				}
			} else asmpts(expression[0].value).valuel = assumption & 0xFF;
		}
		if(numbits >= 16) {
			if(asmpts(expression[0].value).m) {
				if(((assumption >> 8) & 0xFF) != asmpts(expression[0].value).valuem) {
					errorInconsistentAssumption(asmpts(expression[0].value).name, assumption, 0
						| (asmpts(expression[0].value).l ? asmpts(expression[0].value).valuel : 0)
						| (asmpts(expression[0].value).m ? asmpts(expression[0].value).valuem << 8 : 0)
						| (asmpts(expression[0].value).h ? asmpts(expression[0].value).valueh << 16 : 0));
					return -1;
				}
			} else asmpts(expression[0].value).valuem = (assumption >> 8) & 0xFF;
		}
		if(numbits >= 24) {
			if(asmpts(expression[0].value).h) {
				if(((assumption >> 16) & 0xFF) != asmpts(expression[0].value).valueh) {
					errorInconsistentAssumption(asmpts(expression[0].value).name, assumption, 0
						| (asmpts(expression[0].value).l ? asmpts(expression[0].value).valuel : 0)
						| (asmpts(expression[0].value).m ? asmpts(expression[0].value).valuem << 8 : 0)
						| (asmpts(expression[0].value).h ? asmpts(expression[0].value).valueh << 16 : 0));
					return -1;
				}
			} else asmpts(expression[0].value).valueh = (assumption >> 16) & 0xFF;
		}
	}
	
	free(expression);
	*result = romdata;
	return 0;
}





int processNextStatement() {
// if(strcmp(asmpath, "data/maps/objects/SeafoamIslands1F.asm") == 0) {
// 	debug = true;
// 	printf("!");
// }
if(debug && currentmacro == 0) printf("%0X ", mempos);
if(debug && mempos > 0x0149D0 + 6) return -1;
	firsttoken = true;
	struct token token = identifyNextToken();
	firsttoken = false;
	
  if(debug) {
	switch(token.type) {
	case OPCODE:
		printf("%s ", OPCODES[token.content]);
		break;
	case DIRECTIVE:
		printf("%s ", DIRECTIVES[token.content]);
		break;
	case PREDECLARED_SYMBOL:
		printf("%s ", PREDEFS[token.content]);
		break;
	case REGISTER:
		printf("%s ", REGISTERS[token.content]);
		break;
	case CONDITION:
		printf("%s ", CCS[token.content]);
		break;
	case OPERATOR:
		printf("%s ", OPERATORS[token.content]);
		break;
	case MACRO:
		printf("%s ", macros(token.content).name);
		break;
	case LABEL:
		printf("%s ", labels(token.content).name);
		break;
	case VARIABLE:
		printf("%s ", varbls(token.content).name);
		break;
	case CONSTANT:
		printf("%s ", consts(token.content).name);
		break;
	case ASSUMPTION:
		printf("%s ", asmpts(token.content).name);
		break;
	case NUMBER:
		printf("%d", token.content);
		break;
	case STRING:
		printf("\"%s\" ", (char*)token.content);
	case UNRECORDED_SYMBOL:
		printf("%s ", (char*)token.content);
		break;
	case NEWLINE:
		break;
	default:
		printf("%s ", TOKENS[token.type]);
		break;
	}
  }
	
	
	
	if(token.type == ASSUMPTION || token.type == UNRECORDED_SYMBOL || token.type == LABEL) {
		if(!loaddata) {
			if(currentsection[0] == '\0') {
				errorLabelCannotExistOutsideOfSection();
				return -1;
			}
			
			unsigned int labelindex;
			
			if(token.type == LABEL) {
				if(labels(token.content).value != mempos) {
					errorLabelAlreadyDefined(labels(token.content).name);
					return -1;
				}
				labelindex = token.content;
			}
			
			else if(token.type == ASSUMPTION) {
				if((asmpts(token.content).l && (mempos & 0xFF) != asmpts(token.content).valuel) ||
					(asmpts(token.content).m && ((mempos >> 8) & 0xFF) != asmpts(token.content).valuem)) {
					errorIncorrectAssumption((char*)asmpts(token.content).name, mempos&0xFFFF, 0 | (asmpts(token.content).l ? asmpts(token.content).valuel : 0)
						| (asmpts(token.content).m ? (asmpts(token.content).valuem << 8) : 0));
					return -1;
				}
				
				labelindex = labelcount;
				recordLabel(asmpts(token.content).name);
				if(assumptioncount % 1024 == 0) free(assumptionlist[assumptioncount/1024]);
				assumptioncount--;
				for(unsigned int i = token.content; i < assumptioncount; i++) asmpts(i) = asmpts(i+1);
			}
			
			else if(token.type == UNRECORDED_SYMBOL) {
				
				labelindex = labelcount;
				recordLabel(strcpy(malloc(strlen((char*)token.content)+1), (char*)token.content));
			}
			
			char newlabelscope[1024];
			unsigned int i;
			for(i = 0; labels(labelindex).name[i] != '.' && i < strlen(labels(labelindex).name); i++) newlabelscope[i] = labels(labelindex).name[i];
			newlabelscope[i] = '\0';
			if(strcmp(labelscope, newlabelscope) != 0) strcpy(labelscope, newlabelscope);
			locallabelscope = labels(labelindex).name;
		}
		
		char colon = getNextChar();
		if(colon == ':') {
			colon = getNextChar();
			if(colon != ':') ungetChar(colon);
		} else ungetChar(colon);
		
		token = identifyNextToken();
	}
	
	
	
	switch(token.type) {
		
	case OPCODE: {
		if((mempos & 0xFFFF) >= 0x8000 && !loadlabels) {
			errorSectionCannotContainData();
			return -1;
		}
		if(currentsection[0] == '\0') {
			errorDataCannotExistOutsideOfSection();
			return -1;
		}
		
		enum asmopcode opcode;
		enum asmarg arg1 = NO;
		enum asmarg arg2 = NO;
		unsigned int argcontent = -1;
		
		// identify opcode
		opcode = token.content;
		
		saveAsmPos();
		token = identifyNextToken();
		
		// identify first argument
		switch(token.type) {
		case REGISTER:
			if(token.content == REG_C && (opcode == JP || opcode == JR || opcode == CALL || opcode == RET)) {
				// identifyNextToken will always identify c as a register before a condition, so it is caught here
				arg1 = ccC;
				break;
			}
			if(token.content == REG_HLI || token.content == REG_HLD || token.content == INVALID_REGISTER) {
				errorUnexpectedToken(token);
				return -1;
			}
			arg1 = token.content - REG_A + A;
			break;
		
		case STRING:
		case DIRECTIVE:
		case OPERATOR:
		case LABEL:
		case VARIABLE:
		case CONSTANT:
		case ASSUMPTION:
		case UNRECORDED_SYMBOL:
		case NUMBER:
			restoreAsmPos();
			switch(opcode) {
				long romfilepos;
				unsigned long romdata;
				long result;
			case CALL:
			case JP:
				if(!loadlabels) {
					romfilepos = ftell(romfile);
					fgetc(romfile);
					romdata = fgetc(romfile) | (fgetc(romfile) << 8);
					fseek(romfile, romfilepos, SEEK_SET);
					if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
					if(result > 0xFFFF) {
						errorValueTooLarge(16);
						return -1;
					}
					if(result < 0) {
						result *= -1;
						if(result > 0xFFFF) {
							errorValueTooLarge(16);
							return -1;
						}
						result = 0x10000 - result;
					}
					argcontent = result;
				} else {
					struct expressionpart* expression = 0;
					int procresult = processExpression(&expression, 16);
					if(procresult != 0 && procresult != 1) return -1;
				}
				arg1 = n16;
				break;
			case JR:
				if(!loadlabels) {
					romfilepos = ftell(romfile);
					fgetc(romfile);
					romdata = fgetc(romfile);
					fseek(romfile, romfilepos, SEEK_SET);
					if(!loaddata) {
						romdata += ((mempos&0xFFFF)+2);
						fseek(romfile, romfilepos, SEEK_SET);
						if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
						result -= ((mempos&0xFFFF)+2);
					} else {
						struct expressionpart* expression = 0;
						int procresult = processExpression(&expression, 16);
						if(procresult != 0 && procresult != 1) return -1;
						result = romdata;
					}
					if((signed char)result > 128 || (signed char)result < -129) {
						errorTargetOutOfReach(result);
						return -1;
					}
					argcontent = (unsigned char) result;
				} else {
					struct expressionpart* expression = 0;
					int procresult = processExpression(&expression, 16);
					if(procresult != 0 && procresult != 1) return -1;
				}
				arg1 = e8;
				break;
			case ADD:
			case ADC:
			case SUB:
			case SBC:
			case AND:
			case XOR:
			case OR:
			case CP:
				if(!loadlabels) {
					romfilepos = ftell(romfile);
					fgetc(romfile);
					romdata = fgetc(romfile);
					fseek(romfile, romfilepos, SEEK_SET);
					if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
					if(result > 0xFF) {
						errorValueTooLarge(8);
						return -1;
					}
					if(result < 0) {
						result *= -1;
						if(result > 0xFF) {
							errorValueTooLarge(8);
							return -1;
						}
						result = 0x100 - result;
					}
					argcontent = result;
				} else {
					struct expressionpart* expression = 0;
					int procresult = processExpression(&expression, 8);
					if(procresult != 0 && procresult != 1) return -1;
				}
				arg1 = A;
				arg2 = n8;
				break;
			case RST:
				if(!loadlabels) {
					romfilepos = ftell(romfile);
					romdata = fgetc(romfile) & 0x38;
					fseek(romfile, romfilepos, SEEK_SET);
					if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
					if((result & 0x7) != 0 || result > 0x38) {
						errorUnexpectedToken(token);
						return -1;
					}
				} else {
					struct expressionpart* expression = 0;
					int procresult = processExpression(&expression, 8);
					if(procresult != 0 && procresult != 1) return -1;
					result = 0;
				}
				arg1 = v00 + (result >> 3);
				break;
			case BIT:
			case RES:
			case SET:
				if(!loadlabels) {
					romfilepos = ftell(romfile);
					fgetc(romfile);
					romdata = (fgetc(romfile) >> 3) & 0x7;
					fseek(romfile, romfilepos, SEEK_SET);
					if(calculateExpressionWithAssumption(romdata, 3, &result) != 0) return -1;
					if(result < 0 || result > 7) {
						errorValueTooLarge(3);
						return -1;
					}
				} else {
					struct expressionpart* expression = 0;
					int procresult = processExpression(&expression, 3);
					if(procresult != 0 && procresult != 1) return -1;
					result = 0;
				}
				arg1 = u0 + result;
				break;
			}
			break;
		
		
		case MEMORY_OPEN:
			saveAsmPos();
			token = identifyNextToken();
			switch(token.type) {
			long romfilepos;
			unsigned long romdata;
			long result;
			case REGISTER:
				switch(token.content) {
				case REG_C:
					arg1 = m_C;
					break;
				case REG_BC:
					arg1 = m_BC;
					break;
				case REG_DE:
					arg1 = m_DE;
					break;
				case REG_HL:
					if(opcode == LDI) {
						opcode = LD;
						arg1 = m_HLI;
					} else if(opcode == LDD) {
						opcode = LD;
						arg1 = m_HLD;
					} else {
						arg1 = m_HL;
					}
					break;
				case REG_HLI:
					arg1 = m_HLI;
					break;
				case REG_HLD:
					arg1 = m_HLD;
					break;
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			
			case STRING:
			case DIRECTIVE:
			case OPERATOR:
			case LABEL:
			case VARIABLE:
			case CONSTANT:
			case ASSUMPTION:
			case UNRECORDED_SYMBOL:
			case NUMBER:
				restoreAsmPos();
				if(opcode == LD) {
					if(!loadlabels) {
						romfilepos = ftell(romfile);
						fgetc(romfile);
						romdata = fgetc(romfile) | (fgetc(romfile) << 8);
						fseek(romfile, romfilepos, SEEK_SET);
						if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
						if(result > 0xFFFF) {
							errorValueTooLarge(16);
							return -1;
						}
						argcontent = result;
					} else {
						struct expressionpart* expression = 0;
						int procresult = processExpression(&expression, 16);
						if(procresult != 0 && procresult != 1) return -1;
					}
					arg1 = m_a16;
				} else if(opcode == LDH) {
					if(!loadlabels) {
						romfilepos = ftell(romfile);
						fgetc(romfile);
						romdata = fgetc(romfile) + 0xFF00;
						fseek(romfile, romfilepos, SEEK_SET);
						if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
						if(result < 0xFF00 || result > 0xFFFF) {
							errorTargetOutOfReach(result);
							return -1;
						}
						argcontent = result - 0xFF00;
					} else {
						struct expressionpart* expression = 0;
						int procresult = processExpression(&expression, 16);
						if(procresult != 0 && procresult != 1) return -1;
					}
					arg1 = m_a8;
				} else {
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			}
			
			token = assertNextTokenType(MEMORY_CLOSE);
			break;
		
		
		case CONDITION:
			arg1 = token.content - CC_Z + ccZ;
			break;
		
		
		case NEWLINE:
		case END_OF_FILE:
			restoreAsmPos();
			arg1 = NO;
			break;
		
		
		default:
			errorUnexpectedToken(token);
			return -1;
		}
		
		
		// identify second argument
		if(arg1 != NO && arg2 == NO) {
			saveAsmPos();
			token = identifyNextToken();
			if(token.type == COMMA) {
				saveAsmPos();
				token = identifyNextToken();
				switch(token.type) {
					long romfilepos;
					unsigned long romdata;
					long result;
				case REGISTER:
					if(token.content == REG_HLI || token.content == REG_HLD) {
						errorUnexpectedToken(token);
						return -1;
					}
					arg2 = token.content - REG_A + A;
					if(token.content == REG_SP) {
						saveAsmPos();
						token = identifyNextToken();
						if(token.type == OPERATOR && token.content == OPERATION_ADD) {
							romfilepos = ftell(romfile);
							fgetc(romfile);
							romdata = fgetc(romfile);
							fseek(romfile, romfilepos, SEEK_SET);
							if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
							arg2 = SPe8;
							argcontent = result;
							break;
						} else restoreAsmPos();
					}
					break;
					
					
				case STRING:
				case DIRECTIVE:
				case OPERATOR:
				case LABEL:
				case VARIABLE:
				case CONSTANT:
				case ASSUMPTION:
				case UNRECORDED_SYMBOL:
				case NUMBER:
					restoreAsmPos();
					switch(opcode) {
					case LD:
						if((arg1 >= A && arg1 < A+7) || arg1 == m_HL) goto opcodearg2n8;
						else if(!(arg1 >= BC && arg1 < BC+4)) {
							errorUnexpectedToken(token);
							return -1;
						}
					case JP:
					case CALL:
						if(!loadlabels) {
							romfilepos = ftell(romfile);
							fgetc(romfile);
							romdata = fgetc(romfile) | (fgetc(romfile) << 8);
							fseek(romfile, romfilepos, SEEK_SET);
							if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
							if(result > 0xFFFF) {
								errorValueTooLarge(16);
								return -1;
							}
							if(result < 0) {
								result *= -1;
								if(result > 0xFFFF) {
									errorValueTooLarge(16);
									return -1;
								}
								result = 0x10000 - result;
							}
							argcontent = result;
						} else {
							struct expressionpart* expression = 0;
							int procresult = processExpression(&expression, 16);
							if(procresult != 0 && procresult != 1) return -1;
						}
						arg2 = n16;
						break;
					case ADD:
						if (arg1 == SP) goto opcdoearg2e8;
						else if(arg1 != A) {
							errorUnexpectedToken(token);
							return -1;
						}
					case ADC:
					case SUB:
					case SBC:
					case AND:
					case XOR:
					case OR:
					case CP:
					opcodearg2n8: // case LD:
						if(!loadlabels) {
							romfilepos = ftell(romfile);
							fgetc(romfile);
							romdata = fgetc(romfile);
							fseek(romfile, romfilepos, SEEK_SET);
							if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
							if(result > 0xFF) {
								errorValueTooLarge(8);
								return -1;
							}
							if(result < 0) {
								result *= -1;
								if(result > 0xFF) {
									errorValueTooLarge(8);
									return -1;
								}
								result = 0x100 - result;
							}
							argcontent = result;
						} else {
							struct expressionpart* expression = 0;
							int procresult = processExpression(&expression, 8);
							if(procresult != 0 && procresult != 1) return -1;
						}
						arg2 = n8;
						break;
					case JR:
					opcdoearg2e8: // ADD:
						if(!loadlabels) {
							romfilepos = ftell(romfile);
							fgetc(romfile);
							romdata = fgetc(romfile);
							fseek(romfile, romfilepos, SEEK_SET);
							if(!loaddata) {
								if(romdata >= 128) romdata = -((~romdata & 0xFF) + 1);
								romdata += ((mempos&0xFFFF)+2);
								if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
								result -= ((mempos&0xFFFF)+2);
							} else {
								struct expressionpart* expression = 0;
								int procresult = processExpression(&expression, 16);
								if(procresult != 0 && procresult != 1) return -1;
								result = romdata;
							}
							if((signed char)result > 128 && (signed char)result < -129) {
								errorTargetOutOfReach(result);
								return -1;
							}
							argcontent = (unsigned char) result;
						} else {
							struct expressionpart* expression = 0;
							int procresult = processExpression(&expression, 16);
							if(procresult != 0 && procresult != 1) return -1;
						}
						arg2 = e8;
						break;
					}
					break;
					
					
				case MEMORY_OPEN:
					saveAsmPos();
					token = identifyNextToken();
					switch(token.type) {
						long romfilepos;
						unsigned long romdata;
						long result;
					case REGISTER:
						switch(token.content) {
						case REG_C:
							arg2 = m_C;
							break;
						case REG_BC:
							arg2 = m_BC;
							break;
						case REG_DE:
							arg2 = m_DE;
							break;
						case REG_HL:
							if(opcode == LDI) {
								opcode = LD;
								arg2 = m_HLI;
							} else if(opcode == LDD) {
								opcode = LD;
								arg2 = m_HLD;
							} else {
								arg2 = m_HL;
							}
							break;
						case REG_HLI:
							arg2 = m_HLI;
							break;
						case REG_HLD:
							arg2 = m_HLD;
							break;
						default:
							errorUnexpectedToken(token);
							return -1;
						}
						break;
					
					case STRING:
					case DIRECTIVE:
					case OPERATOR:
					case LABEL:
					case VARIABLE:
					case CONSTANT:
					case ASSUMPTION:
					case UNRECORDED_SYMBOL:
					case NUMBER:
						restoreAsmPos();
						if(opcode == LD) {
							if(!loadlabels) {
								romfilepos = ftell(romfile);
								fgetc(romfile);
								romdata = fgetc(romfile) | (fgetc(romfile) << 8);
								fseek(romfile, romfilepos, SEEK_SET);
								if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
								if(result > 0xFFFF) {
									errorValueTooLarge(16);
									return -1;
								}
								argcontent = result;
							} else {
								struct expressionpart* expression = 0;
								int procresult = processExpression(&expression, 16);
								if(procresult != 0 && procresult != 1) return -1;
							}
							arg2 = m_a16;
						} else if(opcode == LDH) {
							if(!loadlabels) {
								romfilepos = ftell(romfile);
								fgetc(romfile);
								romdata = fgetc(romfile) + 0xFF00;
								fseek(romfile, romfilepos, SEEK_SET);
								if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
								if(result < 0xFF00 || result > 0xFFFF) {
									errorTargetOutOfReach(result);
									return -1;
								}
								argcontent = result - 0xFF00;
							} else {
								struct expressionpart* expression = 0;
								int procresult = processExpression(&expression, 16);
								if(procresult != 0 && procresult != 1) return -1;
							}
							arg2 = m_a8;
						} else {
							errorUnexpectedToken(token);
							return -1;
						}
						break;
					}
					
					token = assertNextTokenType(MEMORY_CLOSE);
					break;
				
				default:
					errorUnexpectedToken(token);
					return -1;
				}
			} else {
				restoreAsmPos();
				if(opcode == ADD || opcode == ADC || opcode == SUB || opcode == SBC || opcode == AND || opcode == XOR || opcode == OR || opcode == CP) {
					arg2 = arg1;
					arg1 = A;
				} else arg2 = NO;
			}
		}
		
		// identify instruction
		if(!loadlabels) {
			unsigned char romdata = fgetc(romfile);
			if(romdata != 0xCB) {
				for(unsigned int i = 0; i < sizeof(INSTRUCTIONS)/sizeof(struct instruction); i++) if(INSTRUCTIONS[i].byte == romdata) {
					enum asmopcode romopcode = INSTRUCTIONS[i].opcode;
					enum asmarg romarg1 = INSTRUCTIONS[i].arg1;
					enum asmarg romarg2 = INSTRUCTIONS[i].arg2;
					int romargcontent = -1;
					
					if(romarg1 == n8 || romarg1 == e8 || romarg1 == m_a8 || romarg2 == n8 || romarg2 == e8 || romarg2 == m_a8 || arg2 == SPe8) {
						romargcontent = fgetc(romfile);
					} else if(romarg1 == n16 || romarg1 == m_a16 || romarg2 == n16 || romarg2 == m_a16) {
						romargcontent = fgetc(romfile) | (fgetc(romfile) << 8);
					}
					
					if(opcode != romopcode || arg1 != romarg1 || arg2 != romarg2) {
						errorInconsistentDataInstruction(opcode, arg1, arg2, argcontent, romopcode, romarg1, romarg2, romargcontent);
						return -1;
					}
					
					if(arg1 == n8 || arg1 == e8 || arg1 == m_a8 || arg2 == n8 || arg2 == e8 || arg2 == m_a8 || arg2 == SPe8) {
						if(argcontent != romargcontent) {
							errorInconsistentDataInstruction(opcode, arg1, arg2, argcontent, romopcode, romarg1, romarg2, romargcontent);
							return -1;
						}
					}
					
					else if(arg1 == n16 || arg1 == m_a16 || arg2 == n16 || arg2 == m_a16) {
						if(argcontent != romargcontent) {
							errorInconsistentDataInstruction(opcode, arg1, arg2, argcontent, romopcode, romarg1, romarg2, romargcontent);
							return -1;
						}
					}
					
				}
			} else {
				romdata = fgetc(romfile);
				for(unsigned int i = 0; i < sizeof(PREFIXED_INSTRUCTIONS)/sizeof(struct instruction); i++) if(PREFIXED_INSTRUCTIONS[i].byte == romdata)  {
					enum asmopcode romopcode = PREFIXED_INSTRUCTIONS[i].opcode;
					enum asmarg romarg1 = PREFIXED_INSTRUCTIONS[i].arg1;
					enum asmarg romarg2 = PREFIXED_INSTRUCTIONS[i].arg2;
					
					if(opcode != romopcode || arg1 != romarg1 || arg2 != romarg2) {
						errorInconsistentDataInstruction(opcode, arg1, arg2, 0, romopcode, romarg1, romarg2, 0);
						return -1;
					}
				}
			}
		}
		
		mempos++;
		if(arg1 == n8 || arg1 == e8 || arg1 == m_a8 || arg2 == n8 || arg2 == e8 || arg2 == m_a8 || arg2 == SPe8) mempos++;
		else if(arg1 == n16 || arg1 == m_a16 || arg2 == n16 || arg2 == m_a16) mempos += 2;
		if(opcode >= RLC && opcode <= SET) mempos++;
		
		token = assertNextLineBreak();
		
		break;
	}
	
	
	
	case DIRECTIVE:
		switch(token.content) {
		case DIR_DB:
			if(currentsection[0] == '\0') {
				errorDataCannotExistOutsideOfSection();
				return -1;
			}
			while(true) {
				saveAsmPos();
				token = identifyNextToken();
				if(token.type == STRING) {
					if((mempos & 0xFFFF) >= 0x8000 && !loadlabels) {
						errorSectionCannotContainData();
						return -1;
					}
					unsigned char* data = mapstring((char*)token.content);
					long romfilepos = ftell(romfile);
					for(unsigned int i = 0; data[i] != '\0'; i++) {
						if(!loadlabels) {
							unsigned char romdata = fgetc(romfile);
							if(data[i] != romdata) {
								if(data[strlen(data)-1] == 0x50 && currentmacro == 0) {
									
									// edit asm file to match rom file
									// create the new asmfile
									FILE* newasmfile = fopen("edit", "wb");
									if(newasmfile == NULL) {
										printf("Failed to create %s.\n", asmpath);
										return -1;
									}
									
									
									// copy the code up to 'db "'
									saveAsmPos();
									rewind(asmfile);
									while(ftell(newasmfile) < savedasmfilepos - strlen((char*)token.content)-1) {
										char c = fgetc(asmfile);
										fputc(c, newasmfile);
									}
									restoreAsmPos();
									
									
									// insert the new text
									fseek(romfile, romfilepos, SEEK_SET);
									unsigned char romdata = 0;
									while(romdata != 0x50) {
										romdata = fgetc(romfile);
										mempos++;
										unsigned int mapchar;
										for(mapchar = charmapcount-1; mapchar >= 0; mapchar--) if(charms(mapchar).value == romdata) break;
										fputs(charms(mapchar).chars, newasmfile);
									}
									fputc('\"', newasmfile);
									long resumeasmfile = ftell(newasmfile);
									
									
									// copy the rest of the code
									char c = getNextChar();
									while(feof(asmfile) == 0) {
										fputc(c, newasmfile);
										c = getNextChar();
									}
									
									
									// back up the old asmfile
									char bakpath[strlen(asmpath)+strlen(".bak")+1];
									strcpy(bakpath, asmpath);
									strcat(bakpath, ".bak");
									FILE* bakfile = fopen(bakpath, "rb");
									if(bakfile == NULL) {
										fclose(asmfile);
										rename(asmpath, bakpath);
									} else {
										fclose(bakfile);
										fclose(asmfile);
										remove(asmpath);
									}
		
		
									// replace the old asmfile with the new asmfile
									fclose(newasmfile);
									rename("edit", asmpath);
									asmfile = fopen(asmpath, "rb");
									fseek(asmfile, resumeasmfile, SEEK_SET);
									
									
									break;
									
								} else {
									errorInconsistentData(romdata, data[i]);
									return -1;
								}
							}
						}
						mempos++;
					}
				} else if(token.type == NEWLINE) {
					if(mempos < 0x8000 && !loadlabels) {
						unsigned long romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
					}
					mempos++;
					break;
				} else {
					if(!loadlabels) {
						if((mempos & 0xFFFF) >= 0x8000) {
							errorSectionCannotContainData();
							return -1;
						}
						long result;
						unsigned long romdata = fgetc(romfile);
						restoreAsmPos();
						long asmfilepos = ftell(asmfile);
						if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
						if(result < 0) result = (~(-result) & 0xFF) + 1;
						if(result != romdata) {
							if(currentmacro == 0 && false) {
								
								// edit asm file to match rom file
								// create the new asmfile
								FILE* newasmfile = fopen("edit", "wb");
								if(newasmfile == NULL) {
									printf("Failed to create %s.\n", asmpath);
									return -1;
								}
								
								
								// copy the code up to 'db '
								rewind(asmfile);
								while(ftell(newasmfile) < asmfilepos) {
									char c = fgetc(asmfile);
									fputc(c, newasmfile);
								}
								restoreAsmPos();
								
								
								// insert the new data
								char decimalnum[8];
								sprintf(decimalnum, "%d", romdata);
								fputs(decimalnum, newasmfile);
								long resumeasmfile = ftell(newasmfile);
								
								
								// copy the rest of the code
								char c = getNextChar();
								while(feof(asmfile) == 0) {
									fputc(c, newasmfile);
									c = getNextChar();
								}
								
								
								// back up the old asmfile
								char bakpath[strlen(asmpath)+strlen(".bak")+1];
								strcpy(bakpath, asmpath);
								strcat(bakpath, ".bak");
								FILE* bakfile = fopen(bakpath, "rb");
								if(bakfile == NULL) {
									fclose(asmfile);
									rename(asmpath, bakpath);
								} else {
									fclose(bakfile);
									fclose(asmfile);
									remove(asmpath);
								}


								// replace the old asmfile with the new asmfile
								fclose(newasmfile);
								rename("edit", asmpath);
								asmfile = fopen(asmpath, "rb");
								fseek(asmfile, resumeasmfile, SEEK_SET);
								
							} else {
								errorInconsistentData(romdata, result);
								return -1;
							}
						}
					}
					mempos++;
				}
				saveAsmPos();
				token = identifyNextToken();
				if(token.type != COMMA) break;
			}
			restoreAsmPos();
			break;
			
		case DIR_DW:
			if(currentsection[0] == '\0') {
				errorDataCannotExistOutsideOfSection();
				return -1;
			}
			while(true) {
				saveAsmPos();
				token = identifyNextToken();
				if(token.type == STRING) {
					if((mempos & 0xFFFF) >= 0x8000 && !loadlabels) {
						errorSectionCannotContainData();
						return -1;
					}
					unsigned char* data = mapstring((char*)token.content);
					for(unsigned int i = 0; data[i] != '\0'; i++) {
						if(!loadlabels) {
							unsigned int romdata = fgetc(romfile) | (fgetc(romfile) << 8);
							if((unsigned int)data[i] != romdata) {
								errorInconsistentData(romdata, data[i]);
								return -1;
							}
						}
						mempos++;
						mempos++;
					}
				} else if(token.type == NEWLINE) {
					if(mempos < 0x8000 && !loadlabels) {
						unsigned long romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
						romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
					}
					mempos++;
					mempos++;
					break;
				} else {
					if(!loadlabels) {
						if((mempos & 0xFFFF) >= 0x8000) {
							errorSectionCannotContainData();
							return -1;
						}
						long result;
						unsigned long romdata = fgetc(romfile) | (fgetc(romfile) << 8);
						restoreAsmPos();
						long asmfilepos = ftell(asmfile);
						if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
						if(result < 0) result = (~(-result) & 0xFFFF) + 1;
						if(result != romdata) {
							if(currentmacro == 0 && false) {
								
								// edit asm file to match rom file
								// create the new asmfile
								FILE* newasmfile = fopen("edit", "wb");
								if(newasmfile == NULL) {
									printf("Failed to create %s.\n", asmpath);
									return -1;
								}
								
								
								// copy the code up to 'db '
								rewind(asmfile);
								while(ftell(newasmfile) < asmfilepos) {
									char c = fgetc(asmfile);
									fputc(c, newasmfile);
								}
								restoreAsmPos();
								
								
								// insert the new data
								char decimalnum[8];
								sprintf(decimalnum, "%d", romdata);
								fputs(decimalnum, newasmfile);
								long resumeasmfile = ftell(newasmfile);
								
								
								// copy the rest of the code
								char c = getNextChar();
								while(c == ',' || isNumber(c)) c = getNextChar();
								while(feof(asmfile) == 0) {
									fputc(c, newasmfile);
									c = getNextChar();
								}
								
								
								// back up the old asmfile
								char bakpath[strlen(asmpath)+strlen(".bak")+1];
								strcpy(bakpath, asmpath);
								strcat(bakpath, ".bak");
								FILE* bakfile = fopen(bakpath, "rb");
								if(bakfile == NULL) {
									fclose(asmfile);
									rename(asmpath, bakpath);
								} else {
									fclose(bakfile);
									fclose(asmfile);
									remove(asmpath);
								}


								// replace the old asmfile with the new asmfile
								fclose(newasmfile);
								rename("edit", asmpath);
								asmfile = fopen(asmpath, "rb");
								fseek(asmfile, resumeasmfile, SEEK_SET);
								
							} else {
								errorInconsistentData(romdata, result);
								return -1;
							}
						}
					}
					mempos++;
					mempos++;
				}
				saveAsmPos();
				token = identifyNextToken();
				if(token.type != COMMA) break;
			}
			restoreAsmPos();
			break;
			
		case DIR_DL:
			if(currentsection[0] == '\0') {
				errorDataCannotExistOutsideOfSection();
				return -1;
			}
			while(true) {
				saveAsmPos();
				token = identifyNextToken();
				if(token.type == STRING) {
					if((mempos & 0xFFFF) >= 0x8000 && !loadlabels) {
						errorSectionCannotContainData();
						return -1;
					}
					unsigned char* data = mapstring((char*)token.content);
					for(unsigned int i = 0; data[i] != '\0'; i++) {
						if(!loadlabels) {
							unsigned long romdata = fgetc(romfile) | (fgetc(romfile) << 8) | (fgetc(romfile) << 16) | (fgetc(romfile) | 24);
							if((unsigned long)data[i] != romdata) {
								errorInconsistentData(romdata, data[i]);
								return -1;
							}
						}
						mempos++;
						mempos++;
						mempos++;
						mempos++;
					}
				} else if(token.type == NEWLINE) {
					if(mempos < 0x8000 && !loadlabels) {
						unsigned long romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
						romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
						romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
						romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
					}
					mempos++;
					mempos++;
					mempos++;
					mempos++;
					break;
				} else {
					if(!loadlabels) {
						if((mempos & 0xFFFF) >= 0x8000) {
							errorSectionCannotContainData();
							return -1;
						}
						long result;
						unsigned long romdata = fgetc(romfile) | (fgetc(romfile) << 8) | (fgetc(romfile) << 16) | (fgetc(romfile) | 24);
						restoreAsmPos();
						if(calculateExpressionWithAssumption(romdata, 32, &result) != 0) return -1;
						if(result < 0) result = (~(-result) & 0xFFFFFFFF) + 1;
						if(result != romdata) {
							errorInconsistentData(romdata, result);
							return -1;
						}
					}
					mempos++;
					mempos++;
					mempos++;
					mempos++;
				}
				saveAsmPos();
				token = identifyNextToken();
				if(token.type != COMMA) break;
			}
			restoreAsmPos();
			break;
		
		case DIR_DS: {
			if(currentsection[0] == '\0') {
				errorDataCannotExistOutsideOfSection();
				return -1;
			}
			long numfill;
			unsigned char fillbytes[1024];
			unsigned int fillbytecount = 0;
			int calcresult = calculateExpression(&numfill);
			if(calcresult == 1) {
				errorUnableToCalculate();
				return -1;
			} if(calcresult != 0) return -1;
			
			saveAsmPos();
			token = identifyNextToken();
			if(token.type == COMMA) {
				if((mempos&0xFFFF) >= 0x8000 && !loadlabels) {
					errorSectionCannotContainData();
					return -1;
				}
				
				long romfilepos = ftell(romfile);
				while(token.type == COMMA) {
					saveAsmPos();
					token = identifyNextToken();
					if(token.type == STRING) {
						unsigned char* data = mapstring((char*)token.content);
						for(unsigned int i = 0; data[i] != '\0'; i++) fillbytes[fillbytecount++] = data[i];
						free(data);
					} else {
						restoreAsmPos();
						if(!loadlabels) {
							long result;
							unsigned long romdata = fgetc(romfile);
							if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
							if(result < 0) result = (~(-result) & 0xFF) + 1;
							fillbytes[fillbytecount++] = (unsigned char) (result & 0xFF);
						}
					}
					saveAsmPos();
					token = identifyNextToken();
				}
				restoreAsmPos();
				fseek(romfile, romfilepos, SEEK_SET);
				
				unsigned int ii = 0;
				for(unsigned int i = 0; i < numfill; i++) {
					if((mempos&0xFFFF) < 0x8000 && !loadlabels) {
						unsigned char romdata = fgetc(romfile);
						if(fillbytes[ii++] != romdata) {
							errorInconsistentData(romdata, fillbytes[ii-1]);
							return -1;
						}
						ii %= fillbytecount;
					}
					mempos++;
				}
			}
			
			else {
				restoreAsmPos();
				for(unsigned int i = 0; i < numfill; i++) {
					if((mempos&0xFFFF) < 0x8000 && !loadlabels && !(mempos >= 0x000104 && mempos <= 0x000150)) {
						unsigned char romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
					}
					mempos++;
				}
			}
			
			break;}
		
		
		
		case DIR_CHARMAP: {
			token = assertNextTokenType(STRING);
			charms(charmapcount).chars = malloc(strlen((char*)token.content)+1);
			strcpy(charms(charmapcount).chars, (char*)token.content);
			token = assertNextTokenType(COMMA);
			long result;
			int calcresult = calculateExpression(&result);
			if(calcresult == 1) break;
			if(calcresult != 0) return -1;
			charms(charmapcount).value = result;
			incrementCharmaplist;
			break;}
		
		
		
		case DIR_RSSET:{
			long result;
			int calcresult = calculateExpression(&result);
			if(calcresult == 1) {
				errorUnableToCalculate();
				return -1;
			}
			if(calcresult != 0) return -1;
			_RS = result;
			break;}
		
		case DIR_RSRESET:
			_RS = 0;
			break;
		
		
		
		case DIR_MACRO:
			// macro name
			token = identifyNextToken();
			if(token.type == MACRO) {
				charisliteral = true;
				while(token.type != DIRECTIVE || token.content != DIR_ENDM) token = identifyNextToken();
				charisliteral = false;
				break;
			} else if(token.type == TEXT_MACRO) {
				token.type = UNRECORDED_SYMBOL;
				strcpy(symbolstr, "text_far");
				token.content = (unsigned int) symbolstr;
			}
			assertTokenType(token, UNRECORDED_SYMBOL);
			
			macros(macrocount).name = malloc(strlen((char*)token.content)+1);
			strcpy(macros(macrocount).name, (char*)token.content);
			macros(macrocount).content = malloc(1024);
			
			token = assertNextLineBreak();
			
			// macro content
			unsigned int i = 0;
			unsigned char c;
			unsigned char dirword[6];
			unsigned int dirwordi = 0;
			charisliteral = true;
			while(true) {
				c = getNextChar(asmfile);
				if(c == '\n' && currentmacro == 0) linenumber++;
				if(c == ';') {
					while(c != '\n') c = getNextChar(asmfile);
					if(currentmacro == 0) linenumber++;
				}
				if(isWhitespace(c) && (isWhitespace(i == 0 || macros(macrocount).content[i-1]) || macros(macrocount).content[i-1] == '\n')) continue;
				if(c == '\n' && i > 0 && (isWhitespace(macros(macrocount).content[i-1]) || macros(macrocount).content[i-1] == '\n')) i--;
				if((isWhitespace(c) || c == '\n') && dirwordi < 6) {
					dirword[dirwordi] = '\0';
					if(strcmp(dirword, DIRECTIVES[DIR_MACRO]) == 0) {
						errorNestedMacros();
						return -1;
					}
					if(strcmp(dirword, DIRECTIVES[DIR_ENDM]) == 0) break;
				}
				if(c == '\n') dirwordi = 0;
				else if(isWhitespace(c) && dirwordi > 0) dirwordi = 6;
				else if(!isWhitespace(c) && dirwordi < 6) dirword[dirwordi++] = c>='a' && c<='z' ? c+'A'-'a' : c;
				macros(macrocount).content[i++] = c;
				if(i % 1024 == 0) macros(macrocount).content = realloc(macros(macrocount).content, i + 1024);
				if(feof(asmfile) != 0) {
					errorMacroWithoutEnd();
					return -1;
				}
			}
			charisliteral = false;
			i -= strlen(DIRECTIVES[DIR_ENDM]);
			while(isWhitespace(macros(macrocount).content[i-1])) i--;
			macros(macrocount).content[i] = '\0';
			macros(macrocount).content = realloc(macros(macrocount).content, i+1);
			
			incrementMacrolist;
			
			break;
		
		case DIR_SHIFT:
			if(currentmacro == 0) {
				errorCannotShiftOutsideMacro();
				return -1;
			}
			
			unsigned int numshift = 1;
			
			saveAsmPos();
			token = identifyNextToken();
			restoreAsmPos();
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				}
				if(calcresult != 0) return -1;
				numshift = result;
			}
			
			for(unsigned int i = 0; i < numshift; i++) {
				free(currentargs[0]);
				for(unsigned int i = 0; i < _NARG; i++) currentargs[i] = currentargs[i+1];
				_NARG--;
			}
			
			break;
		
		
		
		case DIR_DEF: {
			indef = true;
			struct token subject = identifyNextToken();
			char* subjectname;
			if(subject.type == UNRECORDED_SYMBOL) {
				subjectname = malloc(strlen((char*)subject.content)+1);
				strcpy(subjectname, (char*)subject.content);
			}
			indef = false;
			token = identifyNextToken();
			
			if(token.type == OPERATOR) {
				int varvalue = subject.type == VARIABLE ? varbls(subject.content).value : 0;
				long result;
				int procresult;
				procresult = calculateExpression(&result);
				if(procresult != 0 && procresult != 1) return -1;
				else if(procresult == 0) {
					switch(token.content) {
					case EQUALS:
						varvalue = result;
						break;
					case PLUS_EQUALS:
						varvalue += result;
						break;
					case MINUS_EQUALS:
						varvalue -= result;
						break;
					case MULTIPLY_EQUALS:
						varvalue *= result;
						break;
					case DIVIDE_EQUALS:
						varvalue /= result;
						break;
					case MODULO_EQUALS:
						varvalue %= result;
						break;
					case SHIFT_LEFT_EQUALS:
						varvalue <<= result;
						break;
					case SHIFT_RIGHT_EQUALS:
						varvalue >>= result;
						break;
					case AND_EQUALS:
						varvalue &= result;
						break;
					case OR_EQUALS:
						varvalue |= result;
						break;
					case XOR_EQUALS:
						varvalue ^= result;
						break;
					default:
						errorUnexpectedToken(token);
						return -1;
					}
					if(subject.type == VARIABLE) {
						varbls(subject.content).value = varvalue;
					} else if(subject.type == ASSUMPTION) {
						varbls(variablecount) = (struct variable) { .name = asmpts(subject.content).name, .value = varvalue};
// unsigned long soughtvalue = 0xCD68;
// if(varbls(variablecount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", varbls(variablecount).name, soughtvalue);
// 	return -1;
// }
						incrementVariablelist;
						if(assumptioncount % 1024 == 0) free(assumptionlist[assumptioncount/1024]);
						assumptioncount--;
						for(unsigned int i = subject.content; i < assumptioncount; i++) asmpts(i) = asmpts(i+1);
					} else if(subject.type == UNRECORDED_SYMBOL) {
						varbls(variablecount).value = varvalue;
						varbls(variablecount).name = subjectname;
// unsigned long soughtvalue = 0xCD68;
// if(varbls(variablecount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", varbls(variablecount).name, soughtvalue);
// 	return -1;
// }
						incrementVariablelist;
					} else {
						errorUnexpectedToken(subject);
						return -1;
					}
				} else if(procresult == 1 && subject.type == VARIABLE) {
					free(varbls(subject.content).name);
					if(variablecount % 1024 == 0) free(variablelist[variablecount/1024]);
					variablecount--;
					for(unsigned int i = subject.content; i < variablecount; i++) varbls(i) = varbls(i+1);
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQU) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
				if(subject.type == UNRECORDED_SYMBOL) {
					consts(constantcount).name = subjectname;
					consts(constantcount).value = result;
// unsigned long soughtvalue = 0xCD68;
// if(consts(constantcount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", consts(constantcount).name, soughtvalue);
// 	return -1;
// }
					incrementConstantlist;
				} else if(subject.type == ASSUMPTION) {
					if((asmpts(subject.content).l && (result & 0xFF) != asmpts(subject.content).valuel) ||
						(asmpts(subject.content).m && ((result >> 8) & 0xFF) != asmpts(subject.content).valuem) ||
						(asmpts(subject.content).h && ((result >> 16) & 0xFF) != asmpts(subject.content).valueh)) {
						errorIncorrectAssumption(asmpts(subject.content).name, result, 0 | (asmpts(subject.content).l ? asmpts(subject.content).valuel : 0)
							| (asmpts(subject.content).m ? (asmpts(subject.content).valuem << 8) : 0) | (asmpts(subject.content).h ? (asmpts(subject.content).valueh << 16) : 0));
						return -1;
					}
					consts(constantcount) = (struct constant) { .name = asmpts(subject.content).name, .value = result};
// unsigned long soughtvalue = 0xCD68;
// if(consts(constantcount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", consts(constantcount).name, soughtvalue);
// 	return -1;
// }
					incrementConstantlist;
					if(assumptioncount % 1024 == 0) free(assumptionlist[assumptioncount/1024]);
					assumptioncount--;
					for(unsigned int i = subject.content; i < assumptioncount; i++) asmpts(i) = asmpts(i+1);
				} else if(subject.type == CONSTANT) {
					consts(subject.content).value = result;
				} else {
					errorUnexpectedToken(subject);
					return -1;
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQUS) {
				assertTokenType(subject, UNRECORDED_SYMBOL); 
				costrs(conststrcount).name = subjectname;
				token = assertNextTokenType(STRING);
				for(unsigned int i = 0; ((char*)token.content)[i] != '\0'; i++) {
					if(((char*)token.content)[i] == '\\' && ((char*)token.content)[i+1] == '@') {
						expandedstrs[expandedstrcount] = malloc(strlen((char*)token.content)+1);
						strcpy(expandedstrs[expandedstrcount], (char*)token.content);
						expandedstrspos[expandedstrcount] = 0;
						expandedstrcount++;
						indef = true;
						token = identifyNextToken();
						indef = false;
						break;
					}
				}
				costrs(conststrcount).content = malloc(strlen((char*)token.content)+1);
				strcpy(costrs(conststrcount).content, (char*)token.content);
				incrementConststrlist;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_RB) {
				if(subject.type != UNRECORDED_SYMBOL && subject.type != CONSTANT) {
					errorUnexpectedToken(subject);
					return -1;
				}
				unsigned int constexpr;
				saveAsmPos();
				token = identifyNextToken();
				restoreAsmPos();
				if(token.type == NEWLINE || token.type == END_OF_FILE) constexpr = 1;
				else {
					long result;
					int calcresult = calculateExpression(&result);
					if(calcresult == 1) {
						errorUnableToCalculate();
						return -1;
					} if(calcresult != 0) return -1;
					constexpr = (unsigned int) result;
				}
				if(subject.type == UNRECORDED_SYMBOL) {
					consts(constantcount).name = subjectname;
					consts(constantcount).value = _RS;
// unsigned long soughtvalue = 0xCD68;
// if(consts(constantcount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", consts(constantcount).name, soughtvalue);
// 	return -1;
// }
					incrementConstantlist;
				} else if(consts(subject.content).value != constexpr) {
					errorRedefinitionOfConstant(consts(subject.content));
					return -1;
				}
				_RS += constexpr;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_RW) {
				if(subject.type != UNRECORDED_SYMBOL && subject.type != CONSTANT) {
					errorUnexpectedToken(subject);
					return -1;
				}
				unsigned int constexpr;
				saveAsmPos();
				token = identifyNextToken();
				restoreAsmPos();
				if(token.type == NEWLINE || token.type == END_OF_FILE) constexpr = 1;
				else {
					long result;
					int calcresult = calculateExpression(&result);
					if(calcresult == 1) {
						errorUnableToCalculate();
						return -1;
					} if(calcresult != 0) return -1;
					constexpr = (unsigned int) result;
				}
				constexpr *= 2;
				if(subject.type == UNRECORDED_SYMBOL) {
					consts(constantcount).name = subjectname;
					consts(constantcount).value = _RS;
// unsigned long soughtvalue = 0xCD68;
// if(consts(constantcount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", consts(constantcount).name, soughtvalue);
// 	return -1;
// }
					incrementConstantlist;
				} else if(consts(subject.content).value != constexpr) {
					errorRedefinitionOfConstant(consts(subject.content));
					return -1;
				}
				_RS += constexpr;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_RL) {
				if(subject.type != UNRECORDED_SYMBOL && subject.type != CONSTANT) {
					errorUnexpectedToken(subject);
					return -1;
				}
				unsigned int constexpr;
				saveAsmPos();
				token = identifyNextToken();
				restoreAsmPos();
				if(token.type == NEWLINE || token.type == END_OF_FILE) constexpr = 1;
				else {
					long result;
					int calcresult = calculateExpression(&result);
					if(calcresult == 1) {
						errorUnableToCalculate();
						return -1;
					} if(calcresult != 0) return -1;
					constexpr = (unsigned int) result;
				}
				constexpr *= 4;
				if(subject.type == UNRECORDED_SYMBOL) {
					consts(constantcount).name = subjectname;
					consts(constantcount).value = _RS;
// unsigned long soughtvalue = 0xCD68;
// if(consts(constantcount).value == soughtvalue) {
// 	printLocation();
// 	printf("\n%s is defined as %04X\n", consts(constantcount).name, soughtvalue);
// 	return -1;
// }
					incrementConstantlist;
				} else if(consts(subject.content).value != constexpr) {
					errorRedefinitionOfConstant(consts(subject.content));
					return -1;
				}
				_RS += constexpr;
			}
			
			else {
				errorUnexpectedToken(token);
				return -1;
			}
			
			break;}
		
		case DIR_REDEF: {
			indef = true;
			struct token subject = identifyNextToken();
			char* subjectname;
			if(subject.type == UNRECORDED_SYMBOL) {
				subjectname = malloc(strlen((char*)subject.content)+1);
				strcpy(subjectname, (char*)subject.content);
			}
			indef = false;
			token = identifyNextToken();
			
			if(token.type == OPERATOR) {
				int varvalue = subject.type == VARIABLE ? varbls(subject.content).value : 0;
				long result;
				int procresult;
				procresult = calculateExpression(&result);
				if(procresult != 0 && procresult != 1) return -1;
				else if(procresult == 0) {
					switch(token.content) {
					case EQUALS:
						varvalue = result;
						break;
					case PLUS_EQUALS:
						varvalue += result;
						break;
					case MINUS_EQUALS:
						varvalue -= result;
						break;
					case MULTIPLY_EQUALS:
						varvalue *= result;
						break;
					case DIVIDE_EQUALS:
						varvalue /= result;
						break;
					case MODULO_EQUALS:
						varvalue %= result;
						break;
					case SHIFT_LEFT_EQUALS:
						varvalue <<= result;
						break;
					case SHIFT_RIGHT_EQUALS:
						varvalue >>= result;
						break;
					case AND_EQUALS:
						varvalue &= result;
						break;
					case OR_EQUALS:
						varvalue |= result;
						break;
					case XOR_EQUALS:
						varvalue ^= result;
						break;
					default:
						errorUnexpectedToken(token);
						return -1;
					}
					if(subject.type == VARIABLE) {
						varbls(subject.content).value = varvalue;
					} else if(subject.type == UNRECORDED_SYMBOL) {
						varbls(variablecount).value = varvalue;
						varbls(variablecount).name = subjectname;
						incrementVariablelist;
					} else {
						errorUnexpectedToken(subject);
						return -1;
					}
				} else if(procresult == 1 && subject.type == VARIABLE) {
					free(varbls(subject.content).name);
					if(variablecount % 1024 == 0) free(variablelist[variablecount/1024]);
					variablecount--;
					for(unsigned int i = subject.content; i < variablecount; i++) varbls(i) = varbls(i+1);
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQU) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
				if(subject.type == UNRECORDED_SYMBOL) {
					consts(constantcount).name = subjectname;
					consts(constantcount).value = result;
					incrementConstantlist;
				} else if(subject.type == ASSUMPTION) {
					if((asmpts(subject.content).l && (result & 0xFF) != asmpts(subject.content).valuel) ||
						(asmpts(subject.content).m && ((result >> 8) & 0xFF) != asmpts(subject.content).valuem) ||
						(asmpts(subject.content).h && ((result >> 16) & 0xFF) != asmpts(subject.content).valueh)) {
						errorIncorrectAssumption(asmpts(subject.content).name, result, 0 | (asmpts(subject.content).l ? asmpts(subject.content).valuel : 0)
							| (asmpts(subject.content).m ? (asmpts(subject.content).valuem << 8) : 0) | (asmpts(subject.content).h ? (asmpts(subject.content).valueh << 16) : 0));
						return -1;
					}
					consts(constantcount) = (struct constant) { .name = asmpts(subject.content).name, .value = result};
					incrementConstantlist;
					if(assumptioncount % 1024 == 0) free(assumptionlist[assumptioncount/1024]);
					assumptioncount--;
					for(unsigned int i = subject.content; i < assumptioncount; i++) asmpts(i) = asmpts(i+1);
				} else if(subject.type == CONSTANT) {
					consts(subject.content).value = result;
				} else {
					errorUnexpectedToken(subject);
					return -1;
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQUS) {
				if(subject.type == UNRECORDED_SYMBOL) {
					costrs(conststrcount).name = subjectname;
					token = assertNextTokenType(STRING);
					for(unsigned int i = 0; ((char*)token.content)[i] != '\0'; i++) {
						if(((char*)token.content)[i] == '\\' && ((char*)token.content)[i+1] == '@') {
							expandedstrs[expandedstrcount] = malloc(strlen((char*)token.content)+1);
							strcpy(expandedstrs[expandedstrcount], (char*)token.content);
							expandedstrspos[expandedstrcount] = 0;
							expandedstrcount++;
							indef = true;
							token = identifyNextToken();
							indef = false;
							break;
						}
					}
					costrs(conststrcount).content = malloc(strlen((char*)token.content)+1);
					strcpy(costrs(conststrcount).content, (char*)token.content);
					incrementConststrlist;
				} else if(subject.type == CONSTANT_STRING) {
					free(costrs(subject.content).content);
					token = assertNextTokenType(STRING);
					for(unsigned int i = 0; ((char*)token.content)[i] != '\0'; i++) {
						if(((char*)token.content)[i] == '\\' && ((char*)token.content)[i+1] == '@') {
							expandedstrs[expandedstrcount] = malloc(strlen((char*)token.content)+1);
							strcpy(expandedstrs[expandedstrcount], (char*)token.content);
							expandedstrspos[expandedstrcount] = 0;
							expandedstrcount++;
							indef = true;
							token = identifyNextToken();
							indef = false;
							break;
						}
					}
					costrs(subject.content).content = malloc(strlen((char*)token.content)+1);
					strcpy(costrs(subject.content).content, (char*)token.content);
				} else {
					errorUnexpectedToken(subject);
					return -1;
				}
			}
			
			else {
				errorUnexpectedToken(token);
				return -1;
			}
			
			break;}
		
		case DIR_PURGE:
			while(true) {
				indef = true;
				token = identifyNextToken();
				indef = false;
				switch(token.type) {
				case LABEL:
					free(labels(token.content).name);
					if(labelcount % 1024 == 0) free(labellist[labelcount/1024]);
					labelcount--;
					for(unsigned int i = token.content; i < labelcount; i++) labels(i) = labels(i+1);
					break;
				case ASSUMPTION:
					free(asmpts(token.content).name);
					if(assumptioncount % 1024 == 0) free(assumptionlist[assumptioncount/1024]);
					assumptioncount--;
					for(unsigned int i = token.content; i < assumptioncount; i++) asmpts(i) = asmpts(i+1);
					break;
				case MACRO:
					free(macros(token.content).name);
					free(macros(token.content).content);
					if(macrocount % 1024 == 0) free(macrolist[macrocount/1024]);
					macrocount--;
					for(unsigned int i = token.content; i < macrocount; i++) macros(i) = macros(i+1);
					break;
				case VARIABLE:
					free(varbls(token.content).name);
					for(unsigned int i = token.content; i < variablecount-1; i++) varbls(i) = varbls(i+1);
					if(variablecount % 1024 == 0) free(variablelist[variablecount/1024]);
					variablecount--;
					break;
				case CONSTANT:
					free(consts(token.content).name);
					if(constantcount % 1024 == 0) free(constantlist[constantcount/1024]);
					constantcount--;
					for(unsigned int i = token.content; i < constantcount; i++) consts(i) = consts(i+1);
					break;
				case CONSTANT_STRING:
					free(costrs(token.content).name);
					free(costrs(token.content).content);
					if(conststrcount % 1024 == 0) free(conststrlist[conststrcount/1024]);
					conststrcount--;
					for(unsigned int i = token.content; i < conststrcount; i++) costrs(i) = costrs(i+1);
					break;
				}
				
				saveAsmPos();
				token = identifyNextToken();
				if(token.type != COMMA) break;
			}
			restoreAsmPos();
			
			break;
		
		case DIR_EXPORT:
			token = identifyNextToken();
			switch(token.type) {
				case LABEL:
				case ASSUMPTION:
				case MACRO:
				case VARIABLE:
				case CONSTANT:
				case CONSTANT_STRING:
					break;
				default:
					errorUnexpectedToken(token);
					break;
			}
			
			break;
		
		
		
		case DIR_IF:
			while(true) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
				if(result != 0) {
					ifdepth++;
					break;
				}
				
				token = assertNextLineBreak();
				
				unsigned int localifdepth = 0;
				charisliteral = true;
				bool wasdebug = debug;
				debug = false;
				while(token.type != DIRECTIVE || (token.content != DIR_ELIF && token.content != DIR_ELSE && token.content != DIR_ENDC) || localifdepth != 0) {
					if(token.type == DIRECTIVE && token.content == DIR_IF) localifdepth++;
					if(token.type == DIRECTIVE && token.content == DIR_ENDC) localifdepth--;
					token = identifyNextToken();
				}
				debug = wasdebug;
				charisliteral = false;
				if(token.content == DIR_ELIF) continue;
				if(token.content == DIR_ELSE) ifdepth++;
				break;
			}
			break;
		
		case DIR_ELIF:
		case DIR_ELSE:
			if(ifdepth == 0) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			unsigned int localifdepth = 0;
			charisliteral = true;
			bool wasdebug = debug;
			debug = false;
			while(token.type != DIRECTIVE || token.content != DIR_ENDC || localifdepth != 0) {
				if(token.type == DIRECTIVE && token.content == DIR_IF) localifdepth++;
				if(token.type == DIRECTIVE && token.content == DIR_ENDC) localifdepth--;
				token = identifyNextToken();
			}
			debug = wasdebug;
			charisliteral = false;
		
		case DIR_ENDC:
			ifdepth--;
			break;
			
			
		
		case DIR_FOR:
			token = identifyNextToken();
			unsigned char returnrepeatsymbol[32];
			strcpy(returnrepeatsymbol, repeatsymbol);
			if(token.type == UNRECORDED_SYMBOL) strcpy(repeatsymbol, (char*)token.content);
			else if(token.type == VARIABLE) strcpy(repeatsymbol, varbls(token.content).name);
			else {
				errorUnexpectedToken(token);
				return -1;
			}
			unsigned long returnrepeatcount = repeatcount;
			
			token = assertNextTokenType(COMMA);
			
			unsigned long repeatstart = 0;
			unsigned long repeatstop = 0;
			unsigned long repeatstep = 1;
			int calcresult = calculateExpression(&repeatstop);
			if(calcresult == 1) {
				errorUnableToCalculate();
				return -1;
			} if(calcresult != 0) return -1;
			
			token = identifyNextToken();
			if(token.type == COMMA) {
				repeatstart = repeatstop;
				calcresult = calculateExpression(&repeatstop);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
				
				token = identifyNextToken();
				if(token.type == COMMA) {
				calcresult = calculateExpression(&repeatstep);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
					token = identifyNextToken();
				}
			}
			
			assertLineBreak(token);
			
			long repeatasmfilepos;
			unsigned int repeatpos;
			unsigned char* repeatcurrentmacro = currentmacro;
			unsigned int repeatexpandedstrcount = expandedstrcount;
			if(currentmacro != 0) repeatpos = currentmacropos;
			else if(expandedstrcount > 0) repeatpos = expandedstrspos[expandedstrcount];
			else repeatasmfilepos = ftell(asmfile);
			
			unsigned int repeatlinenumber = linenumber;
			unsigned int repeatungottencharcount = ungottencharcount;
			
			if(repeatstart == repeatstop) {
				unsigned int localreptdepth = 0;
				charisliteral = true;
				bool wasdebug = debug;
				debug = false;
				while(token.type != DIRECTIVE || token.content != DIR_ENDR || localreptdepth != 0) {
					if(token.type == DIRECTIVE && (token.content == DIR_REPT || token.content == DIR_FOR)) localreptdepth++;
					if(token.type == DIRECTIVE && token.content == DIR_ENDR) localreptdepth--;
					token = identifyNextToken();
				}
				debug = wasdebug;
				charisliteral = false;
			}
			
			bool wasrepeating = repeating;
			repeating = true;
			for(repeatcount = repeatstart; repeatcount < repeatstop; repeatcount += repeatstep) {
				if(currentmacro != 0) currentmacropos = repeatpos;
				else if(expandedstrcount > 0) expandedstrspos[expandedstrcount] = repeatpos;
				else fseek(asmfile, repeatasmfilepos, SEEK_SET);
				
				linenumber = repeatlinenumber;
				ungottencharcount = repeatungottencharcount;
				
				int statementresult = 0;
				while(statementresult == 0) {
				  if(debug && currentmacro == 0) printf("\n%d. ", linenumber);
					statementresult = processNextStatement();
				}
				if(statementresult == 1) {
					errorReptWithoutEndr();
					return -1;
				}
				if(statementresult != 3) return statementresult;
			}
			repeating = wasrepeating;
			
			strcpy(repeatsymbol, returnrepeatsymbol);
			repeatcount = returnrepeatcount;
			break;
			
		case DIR_REPT: {
			unsigned long returnrepeatcount = repeatcount;
			
			unsigned long repeatnum;
			int calcresult = calculateExpression(&repeatnum);
			if(calcresult == 1) {
				errorUnableToCalculate();
				return -1;
			} if(calcresult != 0) return -1;
			
			token = assertNextLineBreak();
			
			long repeatasmfilepos;
			unsigned int repeatpos;
			unsigned char* repeatcurrentmacro = currentmacro;
			unsigned int repeatexpandedstrcount = expandedstrcount;
			if(currentmacro != 0) repeatpos = currentmacropos;
			else if(expandedstrcount > 0) repeatpos = expandedstrspos[expandedstrcount];
			else repeatasmfilepos = ftell(asmfile);
			
			unsigned int repeatlinenumber = linenumber;
			unsigned int repeatungottencharcount = ungottencharcount;
			
			if(repeatnum == 0) {
				unsigned int localreptdepth = 0;
				charisliteral = true;
				bool wasdebug = debug;
				debug = false;
				while(token.type != DIRECTIVE || token.content != DIR_ENDR || localreptdepth != 0) {
					if(token.type == DIRECTIVE && (token.content == DIR_REPT || token.content == DIR_FOR)) localreptdepth++;
					if(token.type == DIRECTIVE && token.content == DIR_ENDR) localreptdepth--;
					token = identifyNextToken();
				}
				debug = wasdebug;
				charisliteral = false;
			}
			
			bool wasrepeating = repeating;
			repeating = true;
			for(repeatcount = 0; repeatcount < repeatnum; repeatcount++) {
				if(currentmacro != 0) currentmacropos = repeatpos;
				else if(expandedstrcount > 0) expandedstrspos[expandedstrcount] = repeatpos;
				else fseek(asmfile, repeatasmfilepos, SEEK_SET);
				
				linenumber = repeatlinenumber;
				ungottencharcount = repeatungottencharcount;
				
				int statementresult = 0;
				while(statementresult == 0) {
				  if(debug && currentmacro == 0) printf("\n%d. ", linenumber);
					statementresult = processNextStatement();
				}
				if(statementresult == 1 || currentmacro != repeatcurrentmacro || expandedstrcount != repeatexpandedstrcount) {
					errorReptWithoutEndr();
					return -1;
				}
				if(statementresult != 3) return statementresult;
			}
			repeating = wasrepeating;
			
			repeatcount = returnrepeatcount;
			break;}
		
		case DIR_ENDR:
			if(repeating) return 3;
			errorUnexpectedToken(token);
			return -1;
		
		
		
		case DIR_INCLUDE: {
			if(currentsection[0] == '\0' && currentsection[1] == '\1') {
				return 2;
			}
			
			token = assertNextTokenType(STRING);
			
			FILE* returnasmfile = asmfile;
			char* returnasmpath = asmpath;
			long returnasmfilepos = ftell(asmfile);
			char* initialsection = malloc(strlen(currentsection)+1);
			strcpy(initialsection, currentsection);
			unsigned int returnlinenumber = linenumber;
			bool returnbackedup = asmfilebackedup;
			asmfile = fopen((char*)token.content, "rb");
			if(asmfile == NULL) {
				errorCannotFindFile((char*)token.content);
				return -1;
			}
			char newasmpath[strlen((char*)token.content)+1];
			strcpy(newasmpath, (char*)token.content);
			asmpath = newasmpath;
			linenumber = 1;
			
		  if(debug) printf("\n\n%s:", (char*)token.content);
			int statementresult = 0;
			while(statementresult == 0) {
		  if(debug) printf("\n%d. ", linenumber);
				statementresult = processNextStatement();
				if(strcmp(initialsection, currentsection) != 0) return 0;
			}
			
			if(statementresult != 1) return statementresult;
			
			asmpath = returnasmpath;
			fclose(asmfile);
			asmfile = returnasmfile;
			fseek(asmfile, returnasmfilepos, SEEK_SET);
			linenumber = returnlinenumber;
			asmfilebackedup = returnbackedup;
		  if(debug) printf("\n\n:");
			
			break;}
		
		case DIR_INCBIN:
			token = assertNextTokenType(STRING);
			
			FILE* binfile = fopen((char*)token.content, "rb");
			if(binfile == NULL) {
				errorCannotFindFile((char*)token.content);
				return -1;
			}
			
			unsigned long binlength = -1;
			
			saveAsmPos();
			token = identifyNextToken();
			if(token.type == COMMA) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				}
				if(calcresult != 0) return -1;
				fseek(binfile, result, SEEK_SET);
				
				saveAsmPos();
				token = identifyNextToken();
				if(token.type == COMMA) {
					calcresult = calculateExpression(&result);
					if(calcresult == 1) {
						errorUnableToCalculate();
						return -1;
					}
					if(calcresult != 0) return -1;
					binlength = result;
				} else restoreAsmPos();
			} else restoreAsmPos();
			
			unsigned char bindata = fgetc(binfile);
			while(feof(binfile) == 0 && binlength > 0) {
				unsigned char romdata = fgetc(romfile);
				if(bindata != romdata) {
					errorInconsistentData(romdata, bindata);
					return -1;
				}
				mempos++;
				binlength--;
				bindata = fgetc(binfile);
			}
			
			fclose(binfile);
			
			break;
		
		
		
		case DIR_SECTION:
			token = assertNextTokenType(STRING);
			strcpy(currentsection, (char*)token.content);
			
			token = assertNextTokenType(COMMA);
			
			token = assertNextTokenType(REGION_TYPE);
			currentregion = token.content;
			
			saveAsmPos();
			token = identifyNextToken();
			if(token.type == MEMORY_OPEN) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
				// mempos = result;
				// romfilepos?
				token = assertNextTokenType(MEMORY_CLOSE);
			} else restoreAsmPos();
			
			loaddata = false;
			loadlabels = false;
			
			break;
		
		case DIR_ENDSECTION:
			currentsection[0] = '\0';
			currentregion = INVALID_REGION_TYPE;
			
			loaddata = false;
			loadlabels = false;
			
			break;
		
		case DIR_LOAD:
			loaddata = true;
			
			token = assertNextTokenType(STRING);
			token = assertNextTokenType(COMMA);
			token = assertNextTokenType(REGION_TYPE);
			
			break;
		
		case DIR_ENDL:
			if(loaddata) loaddata = false;
			
			if(loadlabels) {
				strcpy(currentsection, metasection);
				loadlabels = false;
			}
			
			break;
		
		
		
		case DIR_UNION:
			token = assertNextLineBreak();
			
			unionstart = mempos;
			unionend = mempos;
			
			bool wasinunion = inunion;
			inunion = true;
			int ustatementresult = 0;
			while(ustatementresult == 0) {
			  if(debug) printf("\n%d. ", linenumber);
				ustatementresult = processNextStatement();
			}
			if(ustatementresult == 1) {
				errorUnionWithoutEndu();
				return -1;
			}
			if(ustatementresult != 3) return -1;
			inunion = wasinunion;
			
			break;
		
		case DIR_NEXTU:
			if(!inunion) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			if(mempos > unionend) unionend = mempos;
			mempos = unionstart;
			
			break;
		
		case DIR_ENDU:
			if(!inunion) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			if(mempos > unionend) unionend = mempos;
			mempos = unionend;
			
			return 3;
		
		
		
		case DIR_FAIL:
			token = assertNextTokenType(STRING);
			errorFail((char*)token.content);
			return -1;
			
		case DIR_WARN:
			token = assertNextTokenType(STRING)
			errorWarn((char*)token.content);
			break;
			
		case DIR_ASSERT: {
			long result;
			int calcresult = calculateExpression(&result);
			if(calcresult == 1) {
				result = 1;
				saveAsmPos();
				token = identifyNextToken();
				if(token.type != COMMA) restoreAsmPos();
				else {
					token = identifyNextToken();
					if(token.type == DIRECTIVE && token.content == DIR_STRCAT) {
						token = assertNextToken(OPERATOR, OPEN_PARENTHESIS);
						token = assertNextTokenType(STRING);
						token = assertNextTokenType(COMMA);
						token = assertNextTokenType(STRING);
						token = assertNextToken(OPERATOR, CLOSE_PARENTHESIS);
					}
					else assertTokenType(token, STRING);
				}
			}
			else if(calcresult != 0) return -1;
			
			saveAsmPos();
			token = identifyNextToken();
			if(token.type != COMMA) {
				if(result == 0) {
					errorAssert();
					return -1;
				}
				restoreAsmPos();
			} else {
				token = identifyNextToken();
				if(token.type == DIRECTIVE && token.content == DIR_STRCAT) {
					token = assertNextToken(OPERATOR, OPEN_PARENTHESIS);
					token = assertNextTokenType(STRING);
					char str1[strlen((char*)token.content)+1];
					strcpy(str1, (char*)token.content);
					token = assertNextTokenType(COMMA);
					token = assertNextTokenType(STRING);
					char str2[strlen(str1) + strlen((char*)token.content)+1];
					strcpy(str2, str1);
					strcpy(&str2[strlen(str1)], (char*)token.content);
					token = assertNextToken(OPERATOR, CLOSE_PARENTHESIS);
					if(result == 0) {
						errorAssertWithMessage(str2);
						return -1;
					}
				} else assertTokenType(token, STRING);
			}
			
			break;}
		
		
		
		case DIR_OPT:
		case DIR_PUSHO: {
			char c = getNextChar();
			while(isWhitespace(c)) c = getNextChar();
			while(c != '\n' && c != ';') {
				if(c == 'b') {
					c = getNextChar();
					while(isWhitespace(c)) c = getNextChar();
					bin0 = c;
					c = getNextChar();
					while(isWhitespace(c)) c = getNextChar();
					bin1 = c;
				}
				
				saveAsmPos();
				c = getNextChar();
				while(isWhitespace(c)) c = getNextChar();
			}
			
			restoreAsmPos();
			
			break;}
		
		case DIR_POPO:
			bin0 = '0';
			bin1 = '1';
			
			break;
		
		
		
		
		default:
			errorUnexpectedToken(token);
			return -1;
		}
		
		token = assertNextLineBreak();
		
		break;
	
	
	
	case MACRO: {
		unsigned int macronum = token.content;
		
		// process arguments
		unsigned char* macroargs[64];
		unsigned int macroargcount = 0;
		unsigned int currentlinenumber = linenumber;
		char nextchar = getNextChar(asmfile);
		while(isWhitespace(nextchar)) nextchar = getNextChar(asmfile);
		long asmfilepos = ftell(asmfile) - 1;
		if(nextchar == ';' || nextchar == '\n') {
			if(nextchar == ';') while(getNextChar(asmfile) != '\n') continue;
			if(currentmacro == 0) linenumber++;
		} else {
			macroargs[0] = malloc(1024);
			unsigned int ii = 0;
			while(true) {
				if(nextchar == ';' || nextchar == '\n') {
					if(nextchar == ';') while(getNextChar(asmfile) != '\n') continue;
					if(currentmacro == 0) linenumber++;
					while(isWhitespace(macroargs[macroargcount][ii-1])) ii--;
					if(ii > 0) {
						macroargs[macroargcount][ii] = '\0';
						macroargs[macroargcount] = realloc(macroargs[macroargcount], ii+1);
					} else free(macroargs[macroargcount--]);
					break;
				} else if(nextchar == ',') {
					if(ii > 0) {
						macroargs[macroargcount][ii] = '\0';
						macroargs[macroargcount] = realloc(macroargs[macroargcount], ii+1);
						macroargcount++;
						ii = 0;
						macroargs[macroargcount] = malloc(1024);
					}
					nextchar = getNextChar(asmfile);
					while(isWhitespace(nextchar)) nextchar = getNextChar(asmfile);
				} else if(nextchar == '\"' && (ii == 0 || macroargs[macroargcount][ii-1] != '\\')) {
					charisliteral = true;
					macroargs[macroargcount][ii++] = nextchar;
					nextchar = getNextChar(asmfile);
					while(nextchar != '\"') {
						macroargs[macroargcount][ii++] = nextchar;
						nextchar = getNextChar(asmfile);
					}
					if(strcmp(macros(macronum).name, "li") == 0 && currentmacro == 0) saveAsmPos();
					macroargs[macroargcount][ii++] = nextchar;
					charisliteral = false;
					nextchar = getNextChar(asmfile);
				} else if(nextchar == 18) {
					// cancel
					nextchar = getNextChar(asmfile);
				} else {
					macroargs[macroargcount][ii++] = nextchar;
					nextchar = getNextChar(asmfile);
					if(strcmp(macros(macronum).name, "hlcoord") == 0 && currentmacro == 0) saveAsmPos();
				}
			}
			macroargcount++;
		}
		
		if(strcmp(macros(macronum).name, "li") == 0 && currentmacro == 0) {
			long romfilepos = ftell(romfile);
			char listr[strlen(macroargs[0])-1];
			strncpy(listr, &macroargs[0][1], strlen(macroargs[0])-2);
			listr[strlen(macroargs[0])-2] = '\0';
			unsigned char* data = mapstring(listr);
			unsigned int i;
			for(i = 0; data[i] != '\0'; i++) {
				unsigned char romdata = fgetc(romfile);
				if(data[i] != romdata) break;
			}
			fseek(romfile, romfilepos, SEEK_SET);
			
			if(data[i] != '\0') {
				// edit asm file to match rom file
				// create the new asmfile
				FILE* newasmfile = fopen("edit", "wb");
				if(newasmfile == NULL) {
					printf("Failed to create %s.\n", asmpath);
					return -1;
				}
				
				// copy the code up to 'li "'
				rewind(asmfile);
				while(ftell(newasmfile) < asmfilepos + 1) {
					char c = getNextChar(asmfile);
					fputc(c, newasmfile);
				}
				
				// insert the new text
				macroargs[0][1] = '\0';
				unsigned char romdata;
				romdata = fgetc(romfile);
				while(romdata != 0x50) {
					unsigned int mapchar;
					for(mapchar = charmapcount-1; mapchar >= 0; mapchar--) if(charms(mapchar).value == romdata) break;
					fputs(charms(mapchar).chars, newasmfile);
					strcat(macroargs[0], charms(mapchar).chars);
					romdata = fgetc(romfile);
				}
				fputc('\"', newasmfile);
				strcat(macroargs[0], "\"");
				
				// copy the rest of the code
				restoreAsmPos();
				char c = getNextChar();
				while(c != '\n') {
					fputc(c, newasmfile);
					c = getNextChar();
				}
				long resumeasmfile = ftell(newasmfile);
				while(feof(asmfile) == 0) {
					fputc(c, newasmfile);
					c = getNextChar();
				}
				
				// back up the old asmfile
				char bakpath[strlen(asmpath)+strlen(".bak")+1];
				strcpy(bakpath, asmpath);
				strcat(bakpath, ".bak");
				FILE* bakfile = fopen(bakpath, "rb");
				if(bakfile == NULL) {
					fclose(asmfile);
					rename(asmpath, bakpath);
				} else {
					fclose(bakfile);
					fclose(asmfile);
					remove(asmpath);
				}
				
				// replace the old asmfile with the new asmfile
				fclose(newasmfile);
				rename("edit", asmpath);
				asmfile = fopen(asmpath, "rb");
				fseek(asmfile, resumeasmfile, SEEK_SET);
				while(getNextChar() != '\n') continue;
				fseek(romfile, romfilepos, SEEK_SET);
			}
		}
		
		if(strcmp(macros(macronum).name, "hlcoord") == 0 && currentmacro == 0) {
			long romfilepos = ftell(romfile);
			#define SCREEN_WIDTH (20)
			#define wTileMap (0xC3A0)
			unsigned char coordx = 0;
			unsigned char coordy = 0;
			bool validcoords = true;
			for(unsigned int i = 0; macroargs[0][i] != '\0'; i++) {
				if(!isNumber(macroargs[0][i])) {
					validcoords = false;
					break;
				}
				coordx *= 10;
				coordx += macroargs[0][i] - '0';
			}
			for(unsigned int i = 0; macroargs[1][i] != '\0'; i++) {
				if(!isNumber(macroargs[1][i])) {
					validcoords = false;
					break;
				}
				coordy *= 10;
				coordy += macroargs[1][i] - '0';
			}
			unsigned int asmdata = coordy * SCREEN_WIDTH + coordx + wTileMap;
			if(fgetc(romfile) != 0x21) {
				printLocation();
				printf("Error: Instruction 'hlcoord %s, %s' inconsistent with ROM data", macroargs[0], macroargs[1]);
				return -1;
			}
			unsigned int romdata = fgetc(romfile) | (fgetc(romfile) << 8);
			fseek(romfile, romfilepos, SEEK_SET);
			
			if(asmdata != romdata && validcoords) {
				// edit asm file to match rom file
				// create the new asmfile
				FILE* newasmfile = fopen("edit", "wb");
				if(newasmfile == NULL) {
					printf("Failed to create %s.\n", asmpath);
					return -1;
				}
				
				// copy the code up to 'hlcoord '
				rewind(asmfile);
				while(ftell(newasmfile) < asmfilepos) {
					char c = getNextChar(asmfile);
					fputc(c, newasmfile);
				}
				
				// replace args and insert the new text
				sprintf(macroargs[0], "%d", (romdata - wTileMap) % 20);
				sprintf(macroargs[1], "%d", (romdata - wTileMap) / 20);
				fputs(macroargs[0], newasmfile);
				fputs(", ", newasmfile);
				fputs(macroargs[1], newasmfile);
				fputc('\n', newasmfile);
				linenumber++;
				long resumeasmfile = ftell(newasmfile);
				
				// copy the rest of the code
				restoreAsmPos();
				char c = getNextChar();
				while(feof(asmfile) == 0) {
					fputc(c, newasmfile);
					c = getNextChar();
				}
				
				// back up the old asmfile
				char bakpath[strlen(asmpath)+strlen(".bak")+1];
				strcpy(bakpath, asmpath);
				strcat(bakpath, ".bak");
				FILE* bakfile = fopen(bakpath, "rb");
				fclose(asmfile);
				if(bakfile == NULL) {
					rename(asmpath, bakpath);
				} else {
					fclose(bakfile);
					remove(asmpath);
				}
				
				// replace the old asmfile with the new asmfile
				fclose(newasmfile);
				rename("edit", asmpath);
				asmfile = fopen(asmpath, "rb");
				fseek(asmfile, resumeasmfile, SEEK_SET);
				fseek(romfile, romfilepos, SEEK_SET);
			}
		}
// if(strcmp(asmpath, "constants/music_constants.asm") == 0) {
// 	printf("\n");
// 	for(unsigned int i = 0; i < macroargcount; i++) {
// 		printf("%s, ", macroargs[i]);
// 	}
// 	printf("\n");
// }
		
		// perform macro
		unsigned char* returncurrentmacro = currentmacro;
		unsigned int returncurrentmacropos = currentmacropos;
		currentmacro = macros(macronum).content;
		currentmacropos = 0;
		
		unsigned char** returncurrentargs = currentargs;
		unsigned int return_NARG = _NARG;
		currentargs = macroargs;
		_NARG = macroargcount;
		
		unsigned char* macroexpandedstrs[64];
		unsigned int macroexpandedstrspos[64];
		unsigned char** returnexpandedstrs = expandedstrs;
		unsigned int* returnexpandedstrspos = expandedstrspos;
		expandedstrs = macroexpandedstrs;
		expandedstrspos = macroexpandedstrspos;
		unsigned int returnexpandedstrcount = expandedstrcount;
		expandedstrcount = 0;
		newuniqueaffix = true;
		
		unsigned int returnlinenumber = linenumber;
		linenumber = currentlinenumber;
		bool returnbackedup = asmfilebackedup;
		
		int statementresult = 0;
		while(statementresult == 0) {
			statementresult = processNextStatement();
		}
		if(statementresult != 1) return statementresult;
		
		currentmacro = returncurrentmacro;
		currentmacropos = returncurrentmacropos;
		
		for(unsigned int i = 0; i < _NARG; i++) free(currentargs[i]);
		currentargs = returncurrentargs;
		_NARG = return_NARG;
	
		expandedstrs = returnexpandedstrs;
		expandedstrspos = returnexpandedstrspos;
		expandedstrcount = returnexpandedstrcount;
		
		linenumber = returnlinenumber;
		asmfilebackedup = returnbackedup;
		//if(currentmacro == 0) linenumber++;
		
		break;}
	
	
	
	case TEXT_MACRO: {
		// create the new asmfile
		FILE* newasmfile = fopen("edit", "wb");
		if(newasmfile == NULL) {
			printf("Failed to create %s.\n", asmpath);
			return -1;
		}
		
		
		// copy the code so far
		saveAsmPos();
		rewind(asmfile);
		while(ftell(newasmfile) < savedasmfilepos - strlen("\n\ttext_far")-1) fputc(fgetc(asmfile), newasmfile);
		restoreAsmPos();
		long resumeasmfile = ftell(newasmfile);
		long resumeromfile = ftell(romfile);
		
		
		// find the existing text
		token = identifyNextToken();
		if(token.type != LABEL && token.type != UNRECORDED_SYMBOL && token.type != ASSUMPTION) {
			errorUnexpectedToken(token);
			return -1;
		}
		char textlabel[strlen((char*)token.content)+1];
		strcpy(textlabel, (char*)token.content);
		FILE* textfile = 0;
		{
			FILE* supertextfile = fopen("text.asm", "rb");
			if(supertextfile == NULL) {
				errorCannotFindFile("text.asm");
				return -1;
			}
			char symbol[256];
			unsigned int i = 0;
			while(feof(supertextfile) == 0) {
				symbol[i] = fgetc(supertextfile);
				if(!isAlpha(symbol[i])) {
					symbol[i] = '\0';
					if(i > 0 && strcmp(symbol, DIRECTIVES[DIR_INCLUDE]) == 0) {
						while(fgetc(supertextfile) != '\"') continue;
						i = 0;
						symbol[i] = fgetc(supertextfile);
						while(symbol[i] != '\"') symbol[++i] = fgetc(supertextfile);
						symbol[i] = '\0';
						textfile = fopen(symbol, "rb");
						if(textfile == NULL) {
							errorCannotFindFile(symbol);
							return -1;
						}
						i = 0;
						while(feof(textfile) == 0) {
							symbol[i] = fgetc(textfile);
							if(!isAlphanumeric(symbol[i]) && symbol[i] != '_') {
								char c = symbol[i];
								symbol[i] = '\0';
								if(c == '\"') while(fgetc(textfile) != '\"') continue;
								else if(c == ':') {
									if(strcmp(textlabel, symbol) == 0) {
										while(fgetc(textfile) != '\n') continue;
										break;
									}
								}
								i = 0;
							} else i++;
						}
						if(feof(textfile) == 0) break;
						fclose(textfile);
						textfile = 0;
					}
					i = 0;
				} else i++;
			}
			if(textfile == 0) {
				errorCannotFindFile(textlabel);
				return -1;
			}
		}
		long textfilestart = ftell(textfile);
		linenumber++;
		
		
		// insert the text data inferred by the romfile
		unsigned int commandcount[TEXT_COMMAND_COUNT];
		for(unsigned int i = 0; i < TEXT_COMMAND_COUNT; i++) commandcount[i] = 0;
		while(true) {
			long romfilepos = ftell(romfile);
			unsigned char romdata = fgetc(romfile);
			enum textcommand command;
			for(command = 0; command < TEXT_COMMAND_COUNT; command++) if(romdata == TEXT_COMMAND_IDS[command]) break;
			if(command == TX_DB) fseek(romfile, romfilepos, SEEK_SET);
			commandcount[command]++;
			
			fputc('\n', newasmfile);
			if(command == PARA) fputc('\n', newasmfile);
			fputc('\t', newasmfile);
			
			enum textcommand nextcommand;
			romfilepos = ftell(romfile);
			switch(command) {
			case TEXT:
				// determine "text" or "text_start"
				romdata = fgetc(romfile);
				fseek(romfile, romfilepos, SEEK_SET);
				for(nextcommand = 0; nextcommand < TEXT_COMMAND_COUNT; nextcommand++) if(romdata == TEXT_COMMAND_IDS[nextcommand]) break;
				if(newTextcommand(nextcommand)) {
					command = TEXT_START;
					goto textcommand_default;
				}
			case NEXT:
			case LINE:
			case PARA:
			case CONT:
			case PAGE:
			case TX_DB:
				fputs(TEXT_COMMANDS[command], newasmfile);
				fputc(' ', newasmfile);
				
				// write text
				romdata = fgetc(romfile);
				for(nextcommand = 0; nextcommand < TEXT_COMMAND_COUNT; nextcommand++) if(romdata == TEXT_COMMAND_IDS[nextcommand]) break;
				if(newTextcommand(nextcommand)) {
					fseek(romfile, romfilepos, SEEK_SET);
					break;
				}
				
				fputc('\"', newasmfile);
				while(!newTextcommand(nextcommand)) {
					romfilepos = ftell(romfile);
					
					// reverse charmap
					unsigned int mapchar;
					for(mapchar = charmapcount-1; mapchar >= 0; mapchar--) if(charms(mapchar).value == romdata) break;
					fputs(charms(mapchar).chars, newasmfile);
					if(romdata == 0x50) break;
					
					romdata = fgetc(romfile);
					for(nextcommand = 0; nextcommand < TEXT_COMMAND_COUNT; nextcommand++) if(romdata == TEXT_COMMAND_IDS[nextcommand]) break;
				}
				fseek(romfile, romfilepos, SEEK_SET);
				fputc('\"', newasmfile);
				
				break;
			
			case TEXT_RAM:
			case TEXT_BCD:
			case TEXT_MOVE:
			case TEXT_BOX:
			case TEXT_DECIMAL:
			case TEXT_DOTS: {
				fputs(TEXT_COMMANDS[command], newasmfile);
				unsigned int commandnum = commandcount[command];
				fseek(textfile, textfilestart, SEEK_SET);
				char symbol[256];
				unsigned int i = 0;
				while(true) {
					long textfilepos = ftell(textfile);
					symbol[i] = fgetc(textfile);
					if(!isAlpha(symbol[i]) && symbol[i] != '_') {
						char c = symbol[i];
						symbol[i] = '\0';
						if(c == '\"') while(fgetc(textfile) != '\"') continue;
						else if(i > 0 && strcmp(symbol, TEXT_COMMANDS[command]) == 0) {
							if(commandnum == 1) {
								while(c != '\n') {
									fputc(c, newasmfile);
									c = fgetc(textfile);
								}
								break;
							} else commandnum--;
						} else if(c == ':') {
							printLocation();
							printf("\nNo matching text command for %s\n", TEXT_COMMANDS[command]);
							return -1;
						}
						i = 0;
					} else i++;
				}
				
				for(unsigned int i = TEXT_COMMAND_SIZES[command]-1; i > 0; i--) fgetc(romfile);
				
				break;}
			
			case DEX:
				romdata = fgetc(romfile);
				if(romdata != 0x50) {
					fseek(romfile, romfilepos, SEEK_SET);
					fputs("db \"<DEXEND>\"", newasmfile);
					break;
				}
				mempos++;
			
			textcommand_default:
			default:
				fputs(TEXT_COMMANDS[command], newasmfile);
				break;
			}
			if(terminatingTextcommand(command)) break;
		}
		
		fclose(textfile);
		
		
		// copy the rest of the code
		token = assertNextLineBreak();
		while(token.type != TEXT_MACRO && (token.type != MACRO && strcmp(macros(token.content).name, "text_asm") != 0) && (feof(asmfile) == 0)) token = identifyNextToken();
		char c = getNextChar();
		while(feof(asmfile) == 0) {
			fputc(c, newasmfile);
			c = getNextChar();
		}
		
		
		// back up the old asmfile
		char bakpath[strlen(asmpath)+strlen(".bak")+1];
		strcpy(bakpath, asmpath);
		strcat(bakpath, ".bak");
		FILE* bakfile = fopen(bakpath, "rb");
		if(bakfile == NULL) {
			fclose(asmfile);
			rename(asmpath, bakpath);
		} else {
			fclose(bakfile);
			fclose(asmfile);
			remove(asmpath);
		}
		
		
		// replace the old asmfile with the new asmfile
		fclose(newasmfile);
		rename("edit", asmpath);
		asmfile = fopen(asmpath, "rb");
		fseek(asmfile, resumeasmfile, SEEK_SET);
		fseek(romfile, resumeromfile, SEEK_SET);
		
		
		break;}
	
	
	
	case NEWLINE:
		break;
	
	case END_OF_FILE:
		return 1;
	
	default:
		errorUnexpectedToken(token);
		return -1;
	
	}
	
	return 0;
}



int findSection(enum asmregiontype region, char* section) {
  bool wasdebug = debug;
  debug = false;
	// process pre-section
	while(currentsection[0] == '\0') {
	  if(debug) printf("\n%d. ", linenumber);
		int statementresult = processNextStatement();
		if(statementresult == 1) break;
		if(statementresult == 2) {
			// include
			struct token token = assertNextTokenType(STRING);
			
			FILE* returnasmfile = asmfile;
			char* returnasmpath = asmpath;
			long returnasmfilepos = ftell(asmfile);
			unsigned int returnlinenumber = linenumber;
			bool returnbackedup = asmfilebackedup;
			asmfile = fopen((char*)token.content, "rb");
			if(asmfile == NULL) {
				errorCannotFindFile((char*)token.content);
				return -1;
			}
			char newasmpath[strlen((char*)token.content)+1];
			strcpy(newasmpath, (char*)token.content);
			asmpath = newasmpath;
			linenumber = 1;
			
		  if(debug) printf("\n\n%s:", token.content);
			int findresult = 0;
		  debug = wasdebug;
			while(findresult == 0) findresult = findSection(region, section);
		  debug = false;
			
			fclose(asmfile);
			asmfile = returnasmfile;
			fseek(asmfile, returnasmfilepos, SEEK_SET);
			linenumber = returnlinenumber;
			asmfilebackedup = returnbackedup;
			asmpath = returnasmpath;
			
		  debug = wasdebug;
			if(findresult != 1) return findresult;
		  debug = false;
		  if(debug) printf("\n\n:");
			
			token = assertNextLineBreak();
			
			continue;
		}
		if(statementresult != 0) return -1;
	}
  debug = wasdebug;
	
	// skip over unrelated sections
	if(strcmp(currentsection, section) != 0) {
	  debug = false;
		charisliteral = true;
		
	  if(debug) printf("\n%d. ", linenumber);
		struct token token = identifyNextToken();
		while((token.type != DIRECTIVE || (token.content != DIR_SECTION && token.content != DIR_INCLUDE && token.content != DIR_LOAD))
			&& token.type != END_OF_FILE) {
		  if(debug) if(token.type == NEWLINE) printf("\n%d. ", linenumber);
			token = identifyNextToken();
		}
		
		
	  debug = wasdebug;
		if(token.type == END_OF_FILE) return 1;
	  debug = false;
		
		
		// include
		if(token.content == DIR_INCLUDE) {
			token = assertNextTokenType(STRING);
			
			FILE* returnasmfile = asmfile;
			char* returnasmpath = asmpath;
			long returnasmfilepos = ftell(asmfile);
			unsigned int returnlinenumber = linenumber;
			bool returnbackedup = asmfilebackedup;
			asmfile = fopen((char*)token.content, "rb");
			if(asmfile == NULL) {
				errorCannotFindFile((char*)token.content);
				return -1;
			}
			char newasmpath[strlen((char*)token.content)+1];
			strcpy(newasmpath, (char*)token.content);
			asmpath = newasmpath;
			linenumber = 1;
			
		  if(debug) printf("\n\n%s:", token.content);
			int findresult = 0;
		  debug = wasdebug;
			while(findresult == 0) findresult = findSection(region, section);
		  debug = false;
				
			fclose(asmfile);
			asmfile = returnasmfile;
			fseek(asmfile, returnasmfilepos, SEEK_SET);
			asmpath = returnasmpath;
			linenumber = returnlinenumber;
			asmfilebackedup = returnbackedup;
			
		  debug = wasdebug;
			if(findresult != 1) return findresult;
		  debug = false;
			
			token = identifyNextToken();
		}
		
		
		// section
		else if(token.content == DIR_SECTION) {
		  debug = wasdebug;
			charisliteral = false;
			loadlabels = false;
			
			token = assertNextTokenType(STRING);
			strcpy(currentsection, (char*)token.content);
			
			token = assertNextTokenType(COMMA);
			
			token = assertNextTokenType(REGION_TYPE);
			currentregion = token.content;
			
			token = identifyNextToken();
			if(token.type == MEMORY_OPEN) {
				long result;
				int calcresult = calculateExpression(&result);
				if(calcresult == 1) {
					errorUnableToCalculate();
					return -1;
				} if(calcresult != 0) return -1;
				if(strcmp(currentsection, section) == 0) {
					mempos = result | (mempos & 0xFF0000);
					fseek(romfile, mempos>>16 == 0 ? mempos : (mempos&0xFFFF) - 0x4000 + (0x4000 * (mempos>>16)), SEEK_SET);
				}
				token = assertNextTokenType(MEMORY_CLOSE);
				token = identifyNextToken();
			}
		}
		
		// load
		else {
		  debug = wasdebug;
			charisliteral = false;
			loadlabels = true;
			
			token = assertNextTokenType(STRING);
			strcpy(metasection, currentsection);
			strcpy(currentsection, (char*)token.content);
			
			token = assertNextTokenType(COMMA);
			
			token = assertNextTokenType(REGION_TYPE);
			currentregion = token.content;
			
			token = identifyNextToken();
		}
		
		assertLineBreak(token);
		
	  debug = wasdebug;
		return 0;
	}
	
	
	// process section
	else {
	  debug = wasdebug;
		charisliteral = false;
		int statementresult = 0;
		while(statementresult == 0) {
		  if(debug) printf("\n%d. ", linenumber);
			statementresult = processNextStatement();
			if(statementresult == -1) return -1;
			if(strcmp(currentsection, section) != 0 || feof(asmfile)) return 2;
		}
	  if(debug) printf("\n");
		if(statementresult == 1) return statementresult;
		else return -1;
	}
}

int processRegion(FILE* layoutfile, enum asmregiontype region, unsigned char bank, char** files) {
	rewind(layoutfile);
	
	// find region
	while(true) {
		if(feof(layoutfile) != 0) return -1;
		
		char c;
		c = fgetc(layoutfile);
		while(isWhitespace(c)) c = fgetc(layoutfile);
		
		unsigned int ii;
		for(ii = 0; ii < strlen(REGIONS[region]); ii++) {
			if(c != REGIONS[region][ii]) break;
			c = fgetc(layoutfile);
		} if(ii == strlen(REGIONS[region])) {
			while(isWhitespace(c)) c = fgetc(layoutfile);
			
			unsigned int value = 0;
			
			if(isNumber(c)) {
				while(isNumber(c)) {
					value *= 10;
					value += c - '0';
					c = fgetc(layoutfile);
				}
			} else if(c == '%') {
				c = fgetc(layoutfile);
				while(c == bin0 || c == bin1) {
					value <<= 1;
					if(c == bin1) value += 1;
					c = fgetc(layoutfile);
				}
			} else if(c == '$') {
				c = fgetc(layoutfile);
				while(isHexadecimal(c)) {
					value *= 0x10;
					if(isNumber(c)) value += c - '0';
					else if(isUppercase(c)) value += c - 'A' + 0xA;
					else if(isLowercase(c)) value += c - 'a' + 0xa;
					c = fgetc(layoutfile);
				}
			} else if(c != '\n') {
				printf("%s error: unexpected %c", LAYOUT_FILE, c);
				return -1;
			} else break;
			
			while(isWhitespace(c)) c = fgetc(layoutfile);
			
			if(value == bank && c == '\n') break;
		}
		
		while(c != '\n' && feof(layoutfile) == 0) c = fgetc(layoutfile);
	}
	
	mempos = bank << 16 | REGION_ADDRESSES[region];
	fseek(romfile, mempos>>16 == 0 ? mempos : (mempos&0xFFFF) - 0x4000 + (0x4000 * (mempos>>16)), SEEK_SET);
	currentregion = region;
	
	
	
	// process region
	while(true) {
		char token[32];
		unsigned int i = 0;
		
		token[i] = fgetc(layoutfile);
		while(isWhitespace(token[i])) token[i] = fgetc(layoutfile);
		
		if(token[i] == ';') while(token[i] != '\n') token[i] = fgetc(layoutfile);
		if(token[i] == '\n') continue;
		
		
		// process section
		if(token[i] == '\"') {
			token[i] = fgetc(layoutfile);
			while(token[i] != '\"') {
				if(token[i] == '\\') {
					token[i] = fgetc(layoutfile);
				}
				token[++i] = fgetc(layoutfile);
			}
			token[i] = '\0';
			
			char c = fgetc(layoutfile);
			while(isWhitespace(c)) c = fgetc(layoutfile);
			if(c == ';') while(c != '\n') c = fgetc(layoutfile);
			if(c != '\n') break;
			
		  printf("\nProcessing %s...", token);
			for(unsigned int i = 0; files[i] != 0; i++) {
				while(conststrcount > localconststrstart) {
					if(conststrcount % 1024 == 0) free(conststrlist[conststrcount/1024]);
					conststrcount--;
					free(costrs(conststrcount).name);
					free(costrs(conststrcount).content);
				}
				charisliteral = false;
				
				asmfile = fopen(files[i], "rb");
				asmpath = files[i];
				if(asmfile == NULL) {
					errorCannotFindFile(files[i]);
					return -1;
				}
				linenumber = 1;
				currentsection[0] = '\0';
				currentsection[1] = '\1';
				
				int findresult = 0;
			  if(debug) printf("\n%s: ", files[i]);
				while(findresult == 0) findresult = findSection(region, token);
				if(findresult == 2) break;
				if(findresult != 1) return -1;
				
				if(files[i+1] == 0) {
					errorSectionNotFound(token);
					return -1;
				}
			}
			
			continue;
		}
		
		
		// process commands
		while(isAlpha(token[i])) token[++i] = fgetc(layoutfile);
		ungetc(token[i], layoutfile);
		token[i] = '\0';
		
		if(strcmp(token, "org") == 0) {
			i = 0;
			token[i] = fgetc(layoutfile);
			while(isWhitespace(token[i])) token[i] = fgetc(layoutfile);
			
			unsigned int value = 0;
			if(isNumber(token[i])) {
				while(isNumber(token[i])) {
					value *= 10;
					value += token[i] - '0';
					token[i] = fgetc(layoutfile);
				}
			} else if(token[i] == '%') {
				token[++i] = fgetc(layoutfile);
				while(token[i] == bin0 || token[i] == bin1) {
					value <<= 1;
					if(token[i] == bin1) value += 1;
					token[i] = fgetc(layoutfile);
				}
			} else if(token[i] == '$') {
				token[++i] = fgetc(layoutfile);
				while(isHexadecimal(token[i])) {
					value *= 0x10;
					if(isNumber(token[i])) value += token[i] - '0';
					else if(isUppercase(token[i])) value += token[i] - 'A' + 0xA;
					else if(isLowercase(token[i])) value += token[i] - 'a' + 0xa;
					token[i] = fgetc(layoutfile);
				}
			}
			
			else break;
			
			mempos = (mempos & 0xFF0000) | value;
			fseek(romfile, mempos>>16 == 0 ? mempos : (mempos&0xFFFF) - 0x4000 + (0x4000 * (mempos>>16)), SEEK_SET);
			
			continue;
		}
		
		break;
	}
	
	if(region == WRAMX || region == SRAM || region == ROMX) {
		char regstr[5];
		strcpy(regstr, REGIONS[region]);
		if(region == ROMX) regstr[3] = '\0';
		else regstr[4] = '\0';
		printf("\nFinished processing %s%0X at %0X\n", regstr, bank, mempos&0xFFFF);
	}
	else printf("\nFinished processing %s at %0X\n", REGIONS[region], mempos&0xFFFF);
	return 0;
}



int main(int argc, char *argv[]) {
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
	
	macros(macrocount).name = "_GREEN";
	incrementMacrolist;
	
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
	char* mainfiles[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	
	
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
			mainfiles[0] = "main.asm";
			mainfiles[1] = "maps.asm";
			mainfiles[2] = "audio.asm";
			mainfiles[3] = "text.asm";
			mainfiles[4] = "ram.asm";
			mainfiles[5] = "gfx/pics.asm";
			mainfiles[6] = "gfx/sprites.asm";
			mainfiles[7] = "gfx/tilesets.asm";
			
			if(processRegion(layoutfile, region, bank, mainfiles) != 0) return -1;
		}
	}
	
	
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
	
	
	
	
	// success!
	char projectname[256];
	getcwd(projectname, 256);
	unsigned int shortnameindex;
	for(unsigned int i = 0; projectname[i] != '\0'; i++) if(projectname[i] == '\\' || projectname[i] == '/') shortnameindex = i + 1;
	strcpy(projectname, &projectname[shortnameindex]);
	printf("\nSuccessfully validated %s with %s\n", projectname, ROM_FILE);
	
	return 0;
}