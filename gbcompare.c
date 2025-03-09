#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>



// POKERED-SPECIFIC
const char* INCLUDE_FILES[] = {
	"includes.asm"
};

const char* MAIN_FILES[] = {
	"audio.asm",
	"home.asm",
	"main.asm",
	"maps.asm",
	"ram.asm",
	"text.asm",
	"gfx/pics.asm",
	"gfx/sprites.asm",
	"gfx/tilesets.asm"
};

const char* LAYOUT_FILE = "layout.link";

const char* ROM_FILE = "aka.gb";
// POKERED-SPECIFIC

bool debug = false;



#define isWhitespace(c) (c==' ' || c=='\t' || c=='\v')
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
	CONSTANT, CONSTANT_STRING, VARIABLE, MACRO, LABEL, ASSUMPTION, UNRECORDED_SYMBOL, NEWLINE, REGION_TYPE, END_OF_FILE, UNKNOWN_TOKEN
};
const char* TOKEN_STRINGS[] = {
	"opcode", "directive", "predefined symbol", "register", "condition", "number", "string", "operator", ",", "[", "]",
	"constant", "constant string", "variable", "macro", "label", "assumption", "unrecorded symbol", "newline", "region type", "end of file", "?????"
};
#define TOKEN_STRINGS_COUNT sizeof(TOKEN_STRINGS)/sizeof(TOKEN_STRINGS[0])

struct token {
	enum asmtoken type;
	unsigned int content;
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
	LD, LDH,
	INC, DEC, ADD, ADC, SUB, SBC,
	SET, SCF, RES, OR, AND, XOR, CPL, CCF, SWAP, CP, BIT,
	SRL, SRA, SLA, RR, RRA, RRC, RRCA, RL, RLA, RLC, RLCA,
	JP, JR, CALL, RST, RET, RETI,
	PUSH, POP,
	EI, DI, DAA, STOP, HALT,
	INVALID_OPCODE
};
const char* OPCODE_STRINGS[] = {
	"nop",
	"ld", "ldh",
	"inc", "dec", "add", "adc", "sub", "sbc",
	"set", "scf", "res", "or", "and", "xor", "cpl", "ccf", "swap", "cp", "bit",
	"srl", "sra", "sla", "rr", "rra", "rrc", "rrca", "rl", "rla", "rlc", "rlca",
	"jp", "jr", "call", "rst", "ret", "reti",
	"push", "pop",
	"ei", "di", "daa", "stop", "halt"
};
#define OPCODE_STRINGS_COUNT sizeof(OPCODE_STRINGS)/sizeof(OPCODE_STRINGS[0])

enum opcodearg {
	NO,
	A, B, C, D, E, H, L, AF, BC, DE, HL, SP, SPe8,
	n8, n16, e8,
	u0, u1, u2, u3, u4, u5, u6, u7,
	v00, v08, v10, v18, v20, v28, v30, v38,
	m_C, m_BC, m_DE, m_HL, m_HLI, m_HLD, m_a8, m_a16,
	ccZ, ccNZ, ccC, ccNC,
	INVALID_ARG
};
const char* OPCODEARG_STRINGS[] = {
	"",
	"a", "b", "c", "d", "e", "h", "l", "af", "bc", "de", "hl", "sp", "sp + e8",
	"n8", "n16", "e8",
	"0", "1", "2", "3", "4", "5", "6", "7",
	"$00", "$08", "$10", "$18", "$20", "$28", "30", "$38",
	"[c]", "[bc]", "[de]", "[hl]", "[hli]", "[hld]", "[a8]", "[a16]",
	"z", "nz", "c", "nc"
};
#define OPCODEARG_STRINGS_COUNT sizeof(OPCODEARG_STRINGS)/sizeof(OPCODEARG_STRINGS[0])

enum asmdirective {
	DIR_DB, DIR_DW, DIR_DL, DIR_DS, DIR_RSRESET, DIR_RSSET, DIR_RB, DIR_RW, DIR_RL, DIR_CHARMAP,
	DIR_BANK, DIR_HIGH, DIR_LOW,
	DIR_DEF, DIR_REDEF, DIR_EQU, DIR_EQUS, DIR_IF, DIR_ELIF, DIR_ELSE, DIR_ENDC, DIR_REPT, DIR_FOR, DIR_ENDR, DIR_MACRO, DIR_ENDM, DIR_INCLUDE,
	DIR_SECTION, DIR_UNION, DIR_NEXTU, DIR_ENDU, DIR_LOAD,
	DIR_FAIL, DIR_WARN, DIR_ASSERT,
	INVALID_DIRECTIVE
};
const char* DIRECTIVE_STRINGS[] = {
	"DB", "DW", "DL", "DS", "RSRESET", "RSSET", "RB", "RW", "RL", "CHARMAP",
	"BANK", "HIGH", "LOW",
	"DEF", "REDEF", "EQU", "EQUS", "IF", "ELIF", "ELSE", "ENDC", "REPT", "FOR", "ENDR", "MACRO", "ENDM", "INCLUDE",
	"SECTION", "UNION", "NEXTU", "ENDU", "LOAD",
	"FAIL", "WARN", "ASSERT",
};
#define DIRECTIVE_STRINGS_COUNT sizeof(DIRECTIVE_STRINGS)/sizeof(DIRECTIVE_STRINGS[0])

enum asmregiontype {
	ROM0, ROMX, VRAM, SRAM, WRAM0, WRAMX, OAM, HRAM, INVALID_REGION_TYPE
};
const char* REGION_STRINGS[] = {
	"ROM0", "ROMX", "VRAM", "SRAM", "WRAM0", "WRAMX", "OAM", "HRAM"
};
#define REGION_STRINGS_COUNT sizeof(REGION_STRINGS)/sizeof(REGION_STRINGS[0])
const unsigned long REGION_ADDRESSES[] = {
	0x0000, 0x4000, 0x8000, 0xA000, 0xC000, 0xC000, 0xFE00, 0xFF80
};

enum asmpredef {
	NARG, RS, INVALID_PREDEF
};
const char* PREDEF_STRINGS[] = {
	"_NARG", "_RS"
};
#define PREDEF_STRINGS_COUNT sizeof(PREDEF_STRINGS)/sizeof(PREDEF_STRINGS[0])

enum asmregister {
	REG_A, REG_B, REG_C, REG_D, REG_E, REG_H, REG_L,
	REG_AF, REG_BC, REG_DE, REG_HL, REG_SP, REG_HLI, REG_HLD,
	INVALID_REGISTER
};
const char* REGISTER_STRINGS[] = {
	"a", "b", "c", "d", "e", "h", "l", "af", "bc", "de", "hl", "sp", "hli", "hld"
};
#define REGISTER_STRINGS_COUNT sizeof(REGISTER_STRINGS)/sizeof(REGISTER_STRINGS[0])

enum asmcondition {
	CC_Z, CC_NZ, CC_C, CC_NC, INVALID_CONDITION
};
const char* CC_STRINGS[] = {
	"z", "nz", "c", "nc"
};
#define CC_STRINGS_COUNT sizeof(CC_STRINGS)/sizeof(CC_STRINGS[0])

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
const char* OPERATOR_STRINGS[] = {
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
#define OPERATOR_STRINGS_COUNT sizeof(OPERATOR_STRINGS)/sizeof(OPERATOR_STRINGS[0])

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





struct instruction {
	enum asmopcode opcode;
	enum opcodearg arg1;
	enum opcodearg arg2;
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
	{ LD, m_C,   A,   0xE2 },
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
	{ LD,   A, m_C,   0xF2 },
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
unsigned long mempos;
enum asmregiontype currentregion = INVALID_REGION_TYPE;
unsigned char currentsection[1024];

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

long predeclaredsymbols[PREDEF_STRINGS_COUNT];
#define _NARG predeclaredsymbols[NARG]
#define _RS predeclaredsymbols[RS]


bool charisliteral = false;
bool instring = false;
bool lastsymbolislocal = false;
unsigned char** currentargs = 0;

unsigned char ungottenchars[1024];
unsigned int ungottencharcount = 0;
unsigned char** expandedstrs;
unsigned int expandedstrcount = 0;
unsigned int* expandedstrspos;

unsigned char* currentmacro = 0;
unsigned int currentmacropos;

char symbolstr[1024];
unsigned char* labelscope;
bool lastsymbolislocal;

unsigned int ifdepth = 0;

unsigned long repeatcount = 0;
unsigned char repeatsymbol[32];
long repeatasmfilepos;
unsigned int repeatlinenumber;
bool repeating = false;

unsigned long unionstart;
unsigned long unionend;
bool inunion = false;



// errors
	void errorUnexpectedToken(struct token token) {
		const char* content;
		switch(token.type) {
		case OPCODE:
			content = OPCODE_STRINGS[token.content];
			break;
		case DIRECTIVE:
			content = DIRECTIVE_STRINGS[token.content];
			break;
		case REGISTER:
			content = REGISTER_STRINGS[token.content];
			break;
		case CONDITION:
			content = CC_STRINGS[token.content];
			break;
		case OPERATOR:
			content = OPERATOR_STRINGS[token.content];
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
		default:
			content = TOKEN_STRINGS[token.type];
			break;
		}
		printf("Error(%d)(%02X:%04X): Unexpected %s \"%s\"\n", linenumber, mempos>>16, mempos&0xFFFF, TOKEN_STRINGS[token.type], content);
		return;
	}
	
	void errorUnexpectedArg() {
		printf("Error(%d)(%02X:%04X): Unexpected argument for directive\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorMustBe3Bit(struct token token) {
		printf("Error(%d)(%02X:%04X): Value must be 3-bit\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorMustBe8Bit(struct token token) {
		printf("Error(%d)(%02X:%04X): Value %d must be 8-bit\n", linenumber, mempos>>16, mempos&0xFFFF, token.content);
		return;
	}

	void errorMustBe16Bit(struct token token) {
		printf("Error(%d)(%02X:%04X): Value must be 16-bit\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorTargetOutOfReach(struct token token) {
		printf("Error(%d)(%02X:%04X): Target out of reach\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorInconsistentAssumption(char* name, int currentvalue, int previousvalue) {
		printf("Error(%d)(%02X:%04X): %s was previously assumed as %06X, and is now assumed as %06X.\n", linenumber, mempos>>16, mempos&0xFFFF, name, previousvalue, currentvalue);
		return;
	}

	void errorIncorrectAssumption(char* name, int realvalue, int assumedvalue) {
		printf("Error(%d)(%02X:%04X): %s was previously assumed as %06X, and is now defined as %06X.\n", linenumber, mempos>>16, mempos&0xFFFF, name, assumedvalue, realvalue);
		return;
	}

	void errorLabelOverflow() {
		printf("Error(%d)(%02X:%04X): Too many labels!!\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorMacroOverflow() {
		printf("Error(%d)(%02X:%04X): Too many macros!!\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorNestedMacros() {
		printf("Error(%d)(%02X:%04X): Cannot nest macros\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}
	
	void errorMacroWithoutEnd() {
		printf("Error(%d)(%02X:%04X): Macro started without end\n", linenumber, mempos>>16, mempos&0xFFFF);
		return;
	}

	void errorArgRequired(unsigned int expectedarg, unsigned int givenargcount) {
		printf("Error(%d)(%02X:%04X): Argument %d is required in this maco, but only %d are giveen\n", linenumber, mempos>>16, mempos&0xFFFF, expectedarg, givenargcount);
		return;
	}

	void errorLabelAlreadyDefined(char* name) {
		printf("Error(%d)(%02X:%04X): Second definition of %s\n", linenumber, mempos>>16, mempos&0xFFFF, name);
		return;
	}

	void errorInconsistentData(int romdata, int asmdata) {
		printf("Error(%d)(%02X:%04X): Data ($%02X) is inconsistent with ROM data ($%02X)\n", linenumber, mempos>>16, mempos&0xFFFF, asmdata, romdata);
		return;
	}

	void errorInconsistentDataOpcode(int romdata, int asmdata) {
		printf("Error(%d)(%02X:%04X): Opcode ($%02X) is inconsistent with ROM data ($%02X)\n", linenumber, mempos>>16, mempos&0xFFFF, asmdata, romdata);
		return;
	}

	void errorInconsistentDataFirstarg(int romdata, int asmdata) {
		printf("Error(%d)(%02X:%04X): First argument ($%02X) is inconsistent with ROM data ($%02X)\n", linenumber, mempos>>16, mempos&0xFFFF, asmdata, romdata);
		return;
	}

	void errorInconsistentDataSecondarg(int romdata, int asmdata) {
		printf("Error(%d)(%02X:%04X): Second argument ($%02X) is inconsistent with ROM data ($%02X)\n", linenumber, mempos>>16, mempos&0xFFFF, asmdata, romdata);
		return;
	}

	void errorUnknownInstruction(enum asmopcode opcode, enum opcodearg arg1, enum opcodearg arg2) {
		if(arg1 == NO) printf("Error(%d)(%02X:%04X): Unknown instruction %s\n", linenumber, mempos>>16, mempos&0xFFFF, OPCODE_STRINGS[opcode]);
		else if(arg2 == NO) printf("Error(%d)(%02X:%04X): Unknown instruction %s %s\n", linenumber, mempos>>16, mempos&0xFFFF, OPCODE_STRINGS[opcode], OPCODEARG_STRINGS[arg1]);
		else printf("Error(%d)(%02X:%04X): Unknown instruction %s %s, %s\n", linenumber, mempos>>16, mempos&0xFFFF,
			OPCODE_STRINGS[opcode], OPCODEARG_STRINGS[arg1], OPCODEARG_STRINGS[arg2]);
	}
	
	void errorUnbalancedParentheses() {
		printf("Error(%d)(%02X:%04X): Unbalanced parentheses\n", linenumber, mempos>>16, mempos&0xFFFF);
	}
	
	void errorUnrecordedSymbol(char* symbol) {
		printf("Error(%d)(%02X:%04X): Unrecorded symbol %s\n", linenumber, mempos>>16, mempos&0xFFFF, symbol);
	}
	
	void errorFail(char* failure) {
		printf("Failure(%d)(%02X:%04X): %s\n", linenumber, mempos>>16, mempos&0xFFFF, failure);
	}
	
	void errorWarn(char* warning) {
		printf("Warning(%d)(%02X:%04X): %s\n", linenumber, mempos>>16, mempos&0xFFFF, warning);
	}
	
	void errorAssert() {
		printf("Failure(%d)(%02X:%04X): Assertion failed\n", linenumber, mempos>>16, mempos&0xFFFF);
	}
	
	void errorAssertWithMessage(char* message) {
		printf("Failure(%d)(%02X:%04X): %s\n", linenumber, mempos>>16, mempos&0xFFFF, message);
	}
	
	void errorSectionCannotContainData() {
		printf("Error(%d)(%02X:%04X): Section cannot contain code or data\n", linenumber, mempos>>16, mempos&0xFFFF);
	}
	
	void errorLabelCannotExistOutsideOfSection() {
		printf("Error(%d)(%02X:%04X): Label cannot exist outside of a section\n", linenumber, mempos>>16, mempos&0xFFFF);
	}
	
	void errorDataCannotExistOutsideOfSection() {
		printf("Error(%d)(%02X:%04X): Code or data cannot exist outside of a section\n", linenumber, mempos>>16, mempos&0xFFFF);
	}
	
	void errorIncorrectRegion(enum asmregiontype currentregion, enum asmregiontype expectedregion) {
		printf("Error(%d)(%02X:%04X): Section assigned to %s, expected %s\n", linenumber, mempos>>16, mempos&0xFFFF, REGION_STRINGS[currentregion], REGION_STRINGS[expectedregion]);
	}
	
	void errorSectionNotFound(char* section) {
		printf("Error: Section \"%s\" was not found\n", section);
	}
	
	void errorReptWithoutEndr() {
		printf("Error(%d)(%02X:%04X): REPT declared without matching ENDR\n", linenumber, mempos>>16, mempos&0xFFFF);
	}
	
	void errorUnionWithoutEndu() {
		printf("Error(%d)(%02X:%04X): UNION declared without matching ENDU\n", linenumber, mempos>>16, mempos&0xFFFF);
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
	for(unsigned int i = 0; i < savedungottencharcount; i++) savedungottenchars[i] = ungottenchars[i];
	savedungottencharcount = ungottencharcount;
}

void restoreAsmPos() {
	fseek(asmfile, savedasmfilepos, SEEK_SET);
	linenumber = savedlinenumber;
	currentmacropos = savedcurrentmacropos;
	for(unsigned int i = 0; i < savedexpandedstrcount; i++) expandedstrspos[i] = savedexpandedstrspos[i];
	expandedstrcount = savedexpandedstrcount;
	for(unsigned int i = 0; i < ungottencharcount; i++) ungottenchars[i] = savedungottenchars[i];
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
	
	unsigned int valuedigit = 0;
	while(width > 0 || value != 0 || valuedigit != 0) {
		unsigned long digit = 1;
		if(!align) {
			for(unsigned int ii = 1; ii < width; ii++) {
				digit *= base;
			}
		}
		if(value >= digit) {
			while(value >= digit*base) {
				if(digit * base == 0) break;
				digit *= base;
			}
			bracestr[i++] = (value / digit) + '0';
			value = value % digit;
			valuedigit = digit;
		} else if(valuedigit != 0) {
			bracestr[i++] = '0';
		} else {
			bracestr[i++] = padchar;
		}
		if(valuedigit != 0) valuedigit /= base;
		if(width != 0) width--;
	}
	
	// frac, prec, and fixed-point types are unused
	
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
			bool wasinstring = instring;
			instring = false;
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

			symbol[i] = getNextChar();
			while(symbol[i] != '}') {
				if(symbol[i] == '\n' || symbol[i] == ';') break;
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
				symbol[++i] = getNextChar();
			}
			symbol[i] = '\0';
			
			if(strcmp(repeatsymbol, symbol) == 0) {
				numToBracestr(repeatcount, sign, exact, align, pad, width, frac, prec, type);
				c = getNextChar();
				instring = wasinstring;
				return c;
			}
			
			for(unsigned int ii = 0; ii < labelcount; ii++) if(strcmp(symbol, labels(ii).name) == 0) {
				numToBracestr(labels(ii).value, sign, exact, align, pad, width, frac, prec, type);
				c = getNextChar();
				instring = wasinstring;
				return c;
			}
			
			for(unsigned int ii = 0; ii < constantcount; ii++) if(strcmp(symbol, consts(ii).name) == 0) {
				numToBracestr(consts(ii).value, sign, exact, align, pad, width, frac, prec, type);
				c = getNextChar();
				instring = wasinstring;
				return c;
			}
			
			for(unsigned int ii = 0; ii < variablecount; ii++) if(strcmp(symbol, varbls(ii).name) == 0) {
				numToBracestr(varbls(ii).value, sign, exact, align, pad, width, frac, prec, type);
				c = getNextChar();
				instring = wasinstring;
				return c;
			}
			
			for(unsigned int ii = 0; ii < conststrcount; ii++) if(strcmp(symbol, costrs(ii).name) == 0) {
				expandedstrs[expandedstrcount] = malloc(strlen(costrs(ii).content)+1 + ungottencharcount);
				strcpy(expandedstrs[expandedstrcount], costrs(ii).content);
				strncat(expandedstrs[expandedstrcount], ungottenchars, ungottencharcount);
				ungottencharcount = 0;
				expandedstrspos[expandedstrcount] = 0;
				expandedstrcount++;
				c = getNextChar();
				instring = wasinstring;
				return c;
			}
			
			fseek(asmfile, savedasmfilepos, SEEK_SET);
			currentmacropos = savedcurrentmacropos;
			expandedstrcount = savedexpandedstrcount;
			for(unsigned int ii = 0; ii < savedexpandedstrcount; ii++) expandedstrspos[ii] = savedexpandedstrspos[ii];
			return '{';
		}
		
		
		else if(c == '\\' && !instring) {
			long savedasmfilepos = ftell(asmfile);
			unsigned int savedcurrentmacropos = currentmacropos;
			unsigned int savedexpandedstrcount = expandedstrcount;
			unsigned int savedexpandedstrspos[1024];
			for(unsigned int ii = 0; ii < expandedstrcount; ii++) savedexpandedstrspos[ii] = expandedstrspos[ii];
			
			char n = getNextChar();
			unsigned int argnum = 0;
			if(n >= '1' && n <= '9') argnum = n - '0';
			else if(n == '<') {
				for(n = getNextChar(); n != '>'; n = getNextChar()) {
					argnum *= 10;
					argnum += n - '0';
				}
				ungetChar(n);
			}
			if(argnum > 0 && _NARG >= argnum) {
				expandedstrs[expandedstrcount] = malloc(strlen(currentargs[argnum-1])+1 + ungottencharcount);
				strcpy(expandedstrs[expandedstrcount], currentargs[argnum-1]);
				strncat(expandedstrs[expandedstrcount], ungottenchars, ungottencharcount);
				ungottencharcount = 0;
				expandedstrspos[expandedstrcount] = 0;
				expandedstrcount++;
				return getNextChar();
			} else {
				fseek(asmfile, savedasmfilepos, SEEK_SET);
				currentmacropos = savedcurrentmacropos;
				expandedstrcount = savedexpandedstrcount;
				for(unsigned int ii = 0; ii < savedexpandedstrcount; ii++) expandedstrspos[ii] = savedexpandedstrspos[ii];
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
					return getNextChar();
				}
				
				fseek(asmfile, savedasmfilepos, SEEK_SET);
				currentmacropos = savedcurrentmacropos;
				expandedstrcount = savedexpandedstrcount;
				for(unsigned int ii = 0; ii < savedexpandedstrcount; ii++) expandedstrspos[ii] = savedexpandedstrspos[ii];
				return '\\';
			}
		}
	}
	
	
	return c;
}

enum asmopcode identifyOpcode(char* token) {
	enum asmopcode opcode;
	for(opcode = 0; opcode < OPCODE_STRINGS_COUNT; opcode++) {
		int i;
		for(i = 0; token[i] != '\0' && OPCODE_STRINGS[opcode][i] != '\0'; i++) {
			if(token[i] != OPCODE_STRINGS[opcode][i] && token[i]-'A'+'a' != OPCODE_STRINGS[opcode][i]) break;
		}
		if(token[i] == '\0' && OPCODE_STRINGS[opcode][i] == '\0') return opcode;
	}
	
	return INVALID_OPCODE;
}

enum asmdirective identifyDirective(char* token) {
	enum asmdirective directive;
	for(directive = 0; directive < DIRECTIVE_STRINGS_COUNT; directive++) {
		int i;
		for(i = 0; token[i] != '\0' && DIRECTIVE_STRINGS[directive][i] != '\0'; i++) {
			if(token[i] != DIRECTIVE_STRINGS[directive][i] && token[i]-'a'+'A' != DIRECTIVE_STRINGS[directive][i]) break;
		}
		if(token[i] == '\0' && DIRECTIVE_STRINGS[directive][i] == '\0') return directive;
	}
	
	return INVALID_DIRECTIVE;
}

enum asmregiontype identifyRegiontype(char* token) {
	enum asmregiontype type;
	for(type = 0; type < REGION_STRINGS_COUNT; type++) {
		int i;
		for(i = 0; token[i] != '\0' && REGION_STRINGS[type][i] != '\0'; i++) {
			if(token[i] != REGION_STRINGS[type][i] && token[i]-'a'+'A' != REGION_STRINGS[type][i]) break;
		}
		if(token[i] == '\0' && REGION_STRINGS[type][i] == '\0') return type;
	}
	
	return INVALID_REGION_TYPE;
}

enum asmdirective identifyPredef(char* token) {
	enum asmpredef predef;
	for(predef = 0; predef < PREDEF_STRINGS_COUNT; predef++) {
		int i;
		for(i = 0; token[i] != '\0' && PREDEF_STRINGS[predef][i] != '\0'; i++) {
			if(token[i] != PREDEF_STRINGS[predef][i]) break;
		}
		if(token[i] == '\0' && PREDEF_STRINGS[predef][i] == '\0') return predef;
	}
	
	return INVALID_PREDEF;
}

enum asmregister identifyRegister(char* token) {
	enum asmregister reg;
	for(reg = 0; reg < REGISTER_STRINGS_COUNT; reg++) {
		int i;
		for(i = 0; token[i] != '\0' && REGISTER_STRINGS[reg][i] != '\0'; i++) {
			if(token[i] != REGISTER_STRINGS[reg][i] && token[i]-'A'+'a' != REGISTER_STRINGS[reg][i]) break;
		}
		if(token[i] == '\0' && REGISTER_STRINGS[reg][i] == '\0') return reg;
	}
	
	return INVALID_REGISTER;
}

enum asmcondition identifyCondition(char* token) {
	enum asmcondition condition;
	for(condition = 0; condition < CC_STRINGS_COUNT; condition++) {
		int i;
		for(i = 0; token[i] != '\0' && CC_STRINGS[condition][i] != '\0'; i++) {
			if(token[i] != CC_STRINGS[condition][i] && token[i]-'A'+'a' != CC_STRINGS[condition][i]) break;
		}
		if(token[i] == '\0' && CC_STRINGS[condition][i] == '\0') return condition;
	}
	
	return INVALID_CONDITION;
}

struct token identifyNextToken() {
  if(debug) printf(".");
	char token[256];
	int i = 0;
	token[i] = getNextChar();
	
	// cross whitespace
	while(isWhitespace(token[i])) {
		token[i] = getNextChar();
	}
	
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
			strcpy(&token[i], labelscope);
			i += strlen(labelscope);
			token[i] = '.';
			lastsymbolislocal = true;
		} else lastsymbolislocal = false;
		while(isValidSymbolChar(token[i])) token[++i] = getNextChar();
		ungetChar(token[i--]);
		token[i+1] = '\0';
		
		// identify repeatsymbol
		if(strcmp(token, repeatsymbol) == 0) return (struct token) { .type = NUMBER, .content = repeatcount };
		
		// identify region type
		enum asmregiontype type = identifyRegiontype(token);
		if(type != INVALID_REGION_TYPE) return (struct token) { .type = REGION_TYPE, .content = type };
		
		// identify predefined symbol
		enum asmpredef predef = identifyPredef(token);
		if(predef != INVALID_PREDEF) return (struct token) { .type = PREDECLARED_SYMBOL, .content = predef };
		
		// identify variable
		for(unsigned int ii = 0; ii < variablecount; ii++) {
			if(strcmp(token, varbls(ii).name) == 0) return (struct token) { .type = VARIABLE, .content = ii };
		}
		
		// identify label
		for(unsigned int ii = 0; ii < labelcount; ii++) {
			if(strcmp(token, labels(ii).name) == 0) return (struct token) { .type = LABEL, .content = ii };
		}
		
		// identify constant
		for(unsigned int ii = 0; ii < constantcount; ii++) {
			if(strcmp(token, consts(ii).name) == 0) return (struct token) { .type = CONSTANT, .content = ii };
		}
		
		// identify assumption
		for(unsigned int ii = 0; ii < assumptioncount; ii++) {
			if(strcmp(token, asmpts(ii).name) == 0) return (struct token) { .type = ASSUMPTION, .content = ii };
		}
		
		// identify macro
		for(unsigned int ii = 0; ii < macrocount; ii++) {
			if(strcmp(token, macros(ii).name) == 0) return (struct token) { .type = MACRO, .content = ii };
		}
	
		// identify constant string
		for(unsigned int ii = 0; ii < conststrcount; ii++) if(strcmp(token, costrs(ii).name) == 0) {
			if(charisliteral) return (struct token) { .type = CONSTANT_STRING, .content = ii };
			
			expandedstrs[expandedstrcount] = malloc(strlen(costrs(ii).content)+1 + ungottencharcount);
			strcpy(expandedstrs[expandedstrcount], costrs(ii).content);
			strncat(expandedstrs[expandedstrcount], ungottenchars, ungottencharcount);
			ungottencharcount = 0;
			expandedstrspos[expandedstrcount] = 0;
			expandedstrcount++;
			struct token returntoken = identifyNextToken();
			return returntoken;
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
		if(isBin(token[i])) {
			unsigned long value = 0;
			while(isBin(token[i])) {
				if(isNumber(token[i])) {
					value <<= 1;
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
	if(token[i] == '\"') {
		// identify string
		instring = true;
		symbolstr[0] = getNextChar();
		unsigned int ii = 0;
		while(symbolstr[ii] != '\"') {
			if(symbolstr[ii] == '\\') {
				bool wascharliteral = charisliteral;
				charisliteral = true;
				symbolstr[ii] = getNextChar();
				charisliteral = wascharliteral;
			}
			symbolstr[++ii] = getNextChar();
		}
		instring = false;
		symbolstr[ii] = '\0';
		return (struct token) { .type = STRING, .content = (int) symbolstr };
	}
	// identify at
	if(token[i] == '@') return (struct token) { .type = NUMBER, .content = mempos };
	
	// identify comma
	if(token[i] == ',') return (struct token) { .type = COMMA };
	
	// identify memory
	if(token[i] == '[') return (struct token) { .type = MEMORY_OPEN };
	if(token[i] == ']') return (struct token) { .type = MEMORY_CLOSE };
	
	// identify operator
	unsigned int matchcount = 0;
	unsigned int lastmatch;
	for(unsigned int ii = 0; ii < OPERATOR_STRINGS_COUNT; ii++) {
		if(token[i] == OPERATOR_STRINGS[ii][0]) {
			matchcount++;
			if(strlen(OPERATOR_STRINGS[ii]) == 1) lastmatch = ii;
		}
	}
	if(matchcount > 0) {
		while(matchcount > 1) {
			matchcount = 0;
			token[++i] = getNextChar();
			for(unsigned int ii = 0; ii < OPERATOR_STRINGS_COUNT; ii++) {
				unsigned int iii;
				for(iii = 0; iii <= i; iii++) {
					if(token[iii] != OPERATOR_STRINGS[ii][iii]) {
						break;
					}
				}
				if(iii > i) {
					matchcount++;
					if(iii == strlen(OPERATOR_STRINGS[ii])) lastmatch = ii;
				}
			}
		}
		if(matchcount == 0) {
			while(i >= strlen(OPERATOR_STRINGS[lastmatch])) ungetChar(token[i--]);
		}
		return (struct token) { .type = OPERATOR, .content = lastmatch };
	}
	
	if(feof(asmfile) != 0) return (struct token) { .type = END_OF_FILE };
	
	
	
  if(debug) printf("?%c ", token[0]);
	return (struct token) { .type = UNKNOWN_TOKEN };
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
		case LABEL:
			expression[i].value = labels(token.content).value;
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
			token = identifyNextToken();
			if(token.type != OPERATOR || token.content != OPEN_PARENTHESIS) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			token = identifyNextToken();
			switch(dirtype) {
			
			case DIR_BANK:
			// determine bank
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
					uncalculable = 1;
					break;
					
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			
			case DIR_HIGH:
				switch(token.type) {
				case LABEL:
					expression[i].value = labels(token.content).value >> 8 & 0xFF;
					break;
				case VARIABLE:
					expression[i].value = varbls(token.content).value >> 8 & 0xFF;
					break;
				case CONSTANT:
					expression[i].value = varbls(token.content).value >> 8 & 0xFF;
					break;
				case ASSUMPTION:
					if(asmpts(token.content).m) {
						expression[i].value = asmpts(token.content).valuem;
						break;
					}
				case UNRECORDED_SYMBOL:
					uncalculable = 1;
					break;
				case NUMBER:
					expression[i].value = token.content >> 8 & 0xFF;
					break;
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			
			case DIR_LOW:
				switch(token.type) {
				case LABEL:
					expression[i].value = labels(token.content).value & 0xFF;
					break;
				case VARIABLE:
					expression[i].value = varbls(token.content).value & 0xFF;
					break;
				case CONSTANT:
					expression[i].value = varbls(token.content).value & 0xFF;
					break;
				case ASSUMPTION:
					if(asmpts(token.content).l) {
						expression[i].value = asmpts(token.content).valuel;
						break;
					}
				case UNRECORDED_SYMBOL:
					uncalculable = 1;
					break;
				case NUMBER:
					expression[i].value = token.content & 0xFF;
					break;
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				break;
				
			case DIR_DEF:
			// determine definition
				switch(token.type) {
				case CONSTANT:
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
			
			default:
				errorUnexpectedToken((struct token) { .type = DIRECTIVE, .content = dirtype });
				return -1;
			}
			
			token = identifyNextToken();
			if(token.type != OPERATOR || token.content != CLOSE_PARENTHESIS) {
				errorUnexpectedToken(token);
				return -1;
			}
			
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
						errorUnbalancedParentheses();
						return -1;
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
	
	// calculate
	unsigned int i = 0;
	while(true) {
		if(expression[i].unknown || (expression[i].operation != END_OF_EXPRESSION && expression[i+1].unknown)) {
			errorUnrecordedSymbol((char*)expression[i].value);
			return -1;
		}
		
		
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
				if(expression[0].not) *result = result == 0 ? 1 : 0;
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
				printf("unexpected operator %s", OPERATOR_STRINGS[expression[i].operation]);
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
				printf("unexpected operator %s", OPERATOR_STRINGS[expression[i].operation]);
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
			printf("unexpected operator %s", OPERATOR_STRINGS[expression[operationpos].operation]);
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
			if(numbits >= 16) {
				asmpts(assumptioncount).valuem = (assumption >> 8) & 0xFF;
				asmpts(assumptioncount).m = true;
				if(numbits >= 24) {
				asmpts(assumptioncount).valueh = (assumption >> 16) & 0xFF;
				asmpts(assumptioncount).h = true;
				} else asmpts(assumptioncount).h = false;
			} else asmpts(assumptioncount).m = false;
		} else asmpts(assumptioncount).l = false;
		incrementAssumptionlist;
	} else {
		if(numbits >= 8) {
			if(asmpts(expression[0].value).l) {
				if(assumption & 0xFF != asmpts(expression[0].value).valuel) {
					errorInconsistentAssumption((char*)expression[0].value, assumption, 0 | (asmpts(expression[0].value).l ? asmpts(expression[0].value).valuel : 0)
						| (asmpts(expression[0].value).m ? asmpts(expression[0].value).valuem << 8 : 0) | (asmpts(expression[0].value).h ? asmpts(expression[0].value).valueh << 16 : 0));
				}
			} else asmpts(expression[0].value).valuel = assumption & 0xFF;
			if(numbits >= 16) {
				if(asmpts(expression[0].value).m) {
					if((assumption >> 8) & 0xFF != asmpts(expression[0].value).valuem) {
						errorInconsistentAssumption((char*)expression[0].value, assumption, (0 | asmpts(expression[0].value).l ? asmpts(expression[0].value).valuel : 0)
							| (asmpts(expression[0].value).m ? asmpts(expression[0].value).valuem << 8 : 0) | (asmpts(expression[0].value).h ? asmpts(expression[0].value).valueh << 16 : 0));
					}
				} else asmpts(expression[0].value).valuem = (assumption >> 8) & 0xFF;
				if(numbits >= 24) {
					if(asmpts(expression[0].value).h) {
						if((assumption >> 16) & 0xFF != asmpts(expression[0].value).valueh) {
							errorInconsistentAssumption((char*)expression[0].value, assumption, 0 | (asmpts(expression[0].value).l ? asmpts(expression[0].value).valuel : 0)
								| (asmpts(expression[0].value).m ? asmpts(expression[0].value).valuem << 8 : 0) | (asmpts(expression[0].value).h ? asmpts(expression[0].value).valueh << 16 : 0));
						}
					} else asmpts(expression[0].value).valueh = (assumption >> 16) & 0xFF;
				}
			}
		}
	}
	
	free(expression);
	*result = romdata;
	return 0;
}



unsigned char* mapstring(char* string) {
	unsigned char data[1024];
	unsigned int datalength = 0;
	for(unsigned int i = 0; string[i] != '\0'; i++) {
		unsigned int longestmatch = 1024;
		unsigned int longestmatchlength = 0;
		for(unsigned int ii = 0; ii < charmapcount; ii++) {
			for(unsigned int iii = 0; string[i+iii] == charms(ii).chars[iii]; iii++) {
				if(charms(ii).chars[iii+1] == '\0' && iii+1 > longestmatchlength) {
					longestmatch = ii;
					longestmatchlength = iii+1;
					break;
				}
			}
		}
		if(longestmatchlength == 0) {
			data[datalength++] = string[i++];
		} else {
			data[datalength++] = charms(longestmatch).value;
			i += longestmatchlength;
		}
	}
	unsigned char* returndata = malloc(datalength+1);
	for(unsigned int i; i < datalength; i++) returndata[i] = data[i];
	returndata[datalength] = '\0';
	return returndata;
}




int processNextStatement() {
	struct token token = identifyNextToken();
	
  if(debug) {
	switch(token.type) {
	case OPCODE:
		printf("%s ", OPCODE_STRINGS[token.content]);
		break;
	case DIRECTIVE:
		printf("%s ", DIRECTIVE_STRINGS[token.content]);
		break;
	case PREDECLARED_SYMBOL:
		printf("%s ", PREDEF_STRINGS[token.content]);
		break;
	case REGISTER:
		printf("%s ", REGISTER_STRINGS[token.content]);
		break;
	case CONDITION:
		printf("%s ", CC_STRINGS[token.content]);
		break;
	case OPERATOR:
		printf("%s ", OPERATOR_STRINGS[token.content]);
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
		printf("%s ", TOKEN_STRINGS[token.type]);
		break;
	}
  }
	
	
	
	if(token.type == LABEL && labels(token.content).value != mempos) {
		errorLabelAlreadyDefined((char*)token.content);
		return -1;
	}
		
	if(token.type == ASSUMPTION) {
		if((asmpts(token.content).l && (mempos & 0xFF) != asmpts(token.content).valuel) ||
			(asmpts(token.content).m && ((mempos >> 8) & 0xFF) != asmpts(token.content).valuem) ||
			(asmpts(token.content).h && ((mempos >> 16) & 0xFF) != asmpts(token.content).valueh)) {
			errorIncorrectAssumption((char*)asmpts(token.content).name, mempos, 0 | (asmpts(token.content).l ? asmpts(token.content).valuel : 0)
				| (asmpts(token.content).m ? (asmpts(token.content).valuem << 8) : 0) | (asmpts(token.content).h ? (asmpts(token.content).valueh << 16) : 0));
			return -1;
		}
		labels(labelcount) = (struct label) { .name = asmpts(token.content).name, .value = mempos};
		assumptioncount--;
		for(unsigned int i = token.content; i < assumptioncount; i++) {
			asmpts(i) = asmpts(i+1);
		}
	}
	
	else if(token.type == UNRECORDED_SYMBOL) {
		labels(labelcount) = (struct label) { .name = strcpy(malloc(strlen((char*)token.content)+1), (char*)token.content), .value = mempos };
	}
	
	if(token.type == ASSUMPTION || token.type == UNRECORDED_SYMBOL || token.type == LABEL) {
		if(currentsection[0] == '\0') {
			errorLabelCannotExistOutsideOfSection();
			return -1;
		}
		
		if(token.type != LABEL) {
			if(!lastsymbolislocal) labelscope = labels(labelcount).name;
			incrementLabellist;
		} else if(!lastsymbolislocal) labelscope = labels(token.content).name;
		
		char colon = getNextChar();
		if(colon == ':') {
			colon = getNextChar();
			if(colon != ':') ungetChar(colon);
		} else ungetChar(colon);
		
		token = identifyNextToken();
	}
	
	
	
	switch(token.type) {
		
	case OPCODE: {
		if(mempos >= 0x8000) {
			errorSectionCannotContainData();
			return -1;
		}
		if(currentsection[0] == '\0') {
			errorDataCannotExistOutsideOfSection();
			return -1;
		}
		
		enum asmopcode opcode;
		enum opcodearg arg1 = NO;
		enum opcodearg arg2 = NO;
		int arg1content;
		int arg2content;
		
		// identify opcode
		opcode = token.content;
		
		// identify first argument
		saveAsmPos();
		token = identifyNextToken();
		switch(token.type) {
		case REGISTER:
			if(token.content == REG_C && (opcode == JP || opcode == JR || opcode == CALL || opcode == RET)) {
				// c can never be identified as a condition, so a catch is here
				arg1 = ccC;
				break;
			}
			if(token.content == REG_HLI || token.content == REG_HLD || token.content == INVALID_REGISTER) {
				errorUnexpectedToken(token);
				return -1;
			}
			arg1 = token.content - REG_A + A;
			break;
		
		case DIRECTIVE:
		case OPERATOR:
		case LABEL:
		case VARIABLE:
		case CONSTANT:
		case ASSUMPTION:
		case UNRECORDED_SYMBOL:
		case NUMBER:
			switch(opcode) {
				long romfilepos;
				unsigned long romdata;
				long result;
			case CALL:
			case JP:
				romfilepos = ftell(romfile);
				fgetc(romfile);
				romdata = fgetc(romfile) | (fgetc(romfile) << 8);
				fseek(romfile, romfilepos, SEEK_SET);
				restoreAsmPos();
				if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
				if(result > 0xFFFF) {
					errorMustBe16Bit(token);
					return -1;
				}
				if(result < 0) {
					result *= -1;
					if(result > 0xFFFF) {
						errorMustBe16Bit(token);
						return -1;
					}
					result = 0x10000 - result;
				}
				arg1 = n16;
				arg1content = result;
				break;
			case JR:
				romfilepos = ftell(romfile);
				fgetc(romfile);
				romdata = fgetc(romfile) + (mempos+2);
				fseek(romfile, romfilepos, SEEK_SET);
				restoreAsmPos();
				if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
				result -= (mempos+2);
				if(result > 128 || result < -129) {
					errorTargetOutOfReach(token);
					return -1;
				}
				arg1content = result;
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
				romfilepos = ftell(romfile);
				fgetc(romfile);
				romdata = fgetc(romfile);
				fseek(romfile, romfilepos, SEEK_SET);
				restoreAsmPos();
				if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
				if(result > 0xFF) {
					errorMustBe8Bit(token);
					return -1;
				}
				if(result < 0) {
					result *= -1;
					if(result > 0xFF) {
						errorMustBe8Bit(token);
						return -1;
					}
					result = 0x100 - result;
				}
				arg1 = A;
				arg2 = n8;
				arg2content = result;
				break;
			case RST:
				romfilepos = ftell(romfile);
				romdata = fgetc(romfile) & 0x38;
				fseek(romfile, romfilepos, SEEK_SET);
				restoreAsmPos();
				if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
				if(result & 0x7 != 0 || result > 0x38) errorUnexpectedToken(token); return -1;
				arg1 = v00 + (result >> 3);
				break;
			case BIT:
			case RES:
			case SET:
				romfilepos = ftell(romfile);
				fgetc(romfile);
				romdata = (fgetc(romfile) >> 3) & 0x7;
				fseek(romfile, romfilepos, SEEK_SET);
				restoreAsmPos();
				if(calculateExpressionWithAssumption(romdata, 3, &result) != 0) return -1;
				if(result < 0 || result > 7) { errorMustBe3Bit(token); return -1; }
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
					arg1 = m_HL;
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
			case DIRECTIVE:
			case OPERATOR:
			case LABEL:
			case VARIABLE:
			case CONSTANT:
			case ASSUMPTION:
			case UNRECORDED_SYMBOL:
			case NUMBER:
				if(opcode == LD) {
					romfilepos = ftell(romfile);
					fgetc(romfile);
					romdata = fgetc(romfile) | (fgetc(romfile) << 8);
					fseek(romfile, romfilepos, SEEK_SET);
					restoreAsmPos();
					if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
					if(result > 0xFFFF) {
						errorMustBe16Bit(token);
						return -1;
					}
					arg1 = m_a16;
					arg1content = result;
				} else if(opcode == LDH) {
					romfilepos = ftell(romfile);
					fgetc(romfile);
					romdata = fgetc(romfile) + 0xFF00;
					fseek(romfile, romfilepos, SEEK_SET);
					restoreAsmPos();
					if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
					if(result < 0xFF00 || result > 0xFFFF) {
						errorTargetOutOfReach(token);
						return -1;
					}
					arg1 = m_a8;
					arg1content = result - 0xFF00;
				} else {
					errorUnexpectedToken(token);
					return -1;
				}
				break;
			}
			token = identifyNextToken();
			if(token.type != MEMORY_CLOSE) {
				errorUnexpectedToken(token);
				return -1;
			}
			break;
		
		
		case CONDITION:
			arg1 = token.content - CC_Z + ccZ;
			break;
		
		
		case NEWLINE:
		case END_OF_FILE:
			arg1 = NO;
			break;
		
		
		default:
			errorUnexpectedToken(token);
			return -1;
		}
		
		
		// identify second argument
		if(arg1 != NO && arg2 == NO) {
			token = identifyNextToken();
			if(token.type == COMMA) {
				saveAsmPos();
				token = identifyNextToken();
				switch(token.type) {
					long romfilepos;
					unsigned long romdata;
					long result;
				case REGISTER:
					if(token.content == REG_HLI || token.content == REG_HLD || token.content == INVALID_REGISTER) {
						errorUnexpectedToken(token);
						return -1;
					}
					arg2 = token.content - REG_A + A;
					break;
					
					
				case DIRECTIVE:
				case OPERATOR:
				case LABEL:
				case VARIABLE:
				case CONSTANT:
				case ASSUMPTION:
				case UNRECORDED_SYMBOL:
				case NUMBER:
					switch(opcode) {
					case LD:
						if((arg1 >= A && arg1 < A+7) || arg1 == m_HL) goto opcodearg2n8;
						else if(!(arg1 >= BC && arg1 < BC+4)) {
							errorUnexpectedToken(token);
							return -1;
						}
					case JP:
					case CALL:
						romfilepos = ftell(romfile);
						fgetc(romfile);
						romdata = fgetc(romfile) | (fgetc(romfile) << 8);
						fseek(romfile, romfilepos, SEEK_SET);
						restoreAsmPos();
						if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
						if(result > 0xFFFF) {
							errorMustBe16Bit(token);
							return -1;
						}
						if(result < 0) {
							result *= -1;
							if(result > 0xFFFF) {
								errorMustBe16Bit(token);
								return -1;
							}
							result = 0x10000 - result;
						}
						arg2 = n16;
						arg2content = result;
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
						romfilepos = ftell(romfile);
						fgetc(romfile);
						romdata = fgetc(romfile);
						fseek(romfile, romfilepos, SEEK_SET);
						restoreAsmPos();
						if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
						if(result > 0xFF) {
							errorMustBe8Bit(token);
							return -1;
						}
						if(result < 0) {
							result *= -1;
							if(result > 0xFF) {
								errorMustBe8Bit(token);
								return -1;
							}
							result = 0x100 - result;
						}
						arg2 = n8;
						arg2content = result;
						break;
					case JR:
					opcdoearg2e8: // ADD:
						romfilepos = ftell(romfile);
						fgetc(romfile);
						romdata = fgetc(romfile);
						if(romdata >= 128) romdata = -((~romdata & 0xFF) + 1);
						romdata += (mempos+2);
						fseek(romfile, romfilepos, SEEK_SET);
						restoreAsmPos();
						if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
						result -= (mempos+2);
						if(result > 128 && result < -129) {
							errorTargetOutOfReach(token);
							return -1;
						}
						arg2 = e8;
						arg2content = result;
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
							arg2 = m_HL;
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
					case DIRECTIVE:
					case OPERATOR:
					case LABEL:
					case VARIABLE:
					case CONSTANT:
					case ASSUMPTION:
					case UNRECORDED_SYMBOL:
					case NUMBER:
						if(opcode == LD) {
							romfilepos = ftell(romfile);
							fgetc(romfile);
							romdata = fgetc(romfile) | (fgetc(romfile) << 8);
							fseek(romfile, romfilepos, SEEK_SET);
							restoreAsmPos();
							if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
							if(result > 0xFFFF) {
								errorMustBe16Bit(token);
								return -1;
							}
							arg2 = m_a16;
							arg2content = result;
						} else if(opcode == LDH) {
							romfilepos = ftell(romfile);
							fgetc(romfile);
							romdata = fgetc(romfile) + 0xFF00;
							fseek(romfile, romfilepos, SEEK_SET);
							restoreAsmPos();
							if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
							if(result < 0xFF00 || result > 0xFFFF) {
								errorTargetOutOfReach(token);
								return -1;
							}
							arg2 = m_a8;
							arg2content = result - 0xFF00;
						} else {
							errorUnexpectedToken(token);
							return -1;
						}
						break;
					}
					token = identifyNextToken();
					if(token.type != MEMORY_CLOSE) {
						errorUnexpectedToken(token);
						return -1;
					}
					break;
				default:
					errorUnexpectedToken(token);
					return -1;
				}
				token = identifyNextToken();
				if(token.type != NEWLINE && token.type != END_OF_FILE) {
					errorUnexpectedToken(token);
					return -1;
				}
			} else if(token.type == NEWLINE || token.type == END_OF_FILE) {
				if(opcode == ADD || opcode == ADC || opcode == SUB || opcode == SBC || opcode == AND || opcode == XOR || opcode == OR || opcode == CP) {
					arg2 = arg1;
					arg1 = A;
				} else arg2 = NO;
			} else {
				errorUnexpectedToken(token);
				return -1;
			}
		} else if(arg1 != NO) {
			token = identifyNextToken();
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				errorUnexpectedToken(token);
				return -1;
			}
		}
		
		// identify instruction
		{ unsigned int i;
		for(i = 0; i < sizeof(INSTRUCTIONS)/sizeof(struct instruction); i++) {
			if(opcode == INSTRUCTIONS[i].opcode && arg1 == INSTRUCTIONS[i].arg1 && arg2 == INSTRUCTIONS[i].arg2) {
				unsigned char rombyte;
				
				rombyte = (unsigned char) fgetc(romfile);
				if(rombyte != INSTRUCTIONS[i].byte) {
					errorInconsistentDataOpcode(rombyte, INSTRUCTIONS[i].byte);
					return -1;
				}
				
				mempos++;
				
				if(arg1 == n8 || arg1 == e8 || arg1 == m_a8) {
					if(arg1content < 0) arg1content = (~(-arg1content) & 0xFF) + 1;
					rombyte = fgetc(romfile);
					arg1content &= 0xFF;
					if(rombyte != (unsigned char) arg1content) {
						errorInconsistentDataFirstarg(rombyte, arg1content);
						return -1;
					}
					mempos++;
				} else if(arg1 == n16 || arg1 == m_a16) {
					if(arg1content < 0) arg1content = (~(-arg1content) & 0xFFFF) + 1;
					rombyte = (unsigned char) fgetc(romfile);
					unsigned int romword = rombyte | (fgetc(romfile) << 8);
					arg1content &= 0xFFFF;
					if(romword != arg1content) {
						errorInconsistentDataFirstarg(romword, arg1content);
						return -1;
					}
					mempos++;
					mempos++;
				}
				
				if(arg2 == n8 || arg2 == e8 || arg2 == m_a8) {
					if(arg2content < 0) arg2content = (~(-arg2content) & 0xFF) + 1;
					rombyte = fgetc(romfile);
					arg2content &= 0xFF;
					if(rombyte != (unsigned char) arg2content) {
						errorInconsistentDataSecondarg(rombyte, arg2content);
						return -1;
					}
					mempos++;
				} else if(arg2 == n16 || arg2 == m_a16) {
					if(arg2content < 0) arg2content = (~(-arg2content)) & 0xFFFF + 1;
					rombyte = (unsigned char) fgetc(romfile);
					unsigned int romword = rombyte | (fgetc(romfile) << 8);
					arg1content &= 0xFFFF;
					if(romword != arg2content) {
						errorInconsistentDataSecondarg(romword, arg2content);
						return -1;
					}
					mempos++;
					mempos++;
				}
				
				break;
			}
		}
		
		if(i >= sizeof(INSTRUCTIONS)/sizeof(struct instruction)) {
			for(i = 0; i < sizeof(PREFIXED_INSTRUCTIONS)/sizeof(struct instruction); i++) {
				if(opcode == PREFIXED_INSTRUCTIONS[i].opcode && arg1 == PREFIXED_INSTRUCTIONS[i].arg1 && arg2 == PREFIXED_INSTRUCTIONS[i].arg2) {
					if(fgetc(romfile) != 0xCB) {
						errorInconsistentDataOpcode(0xCB, PREFIXED_INSTRUCTIONS[i].byte);
						return -1;
					}
					unsigned char rombyte = (unsigned char) fgetc(romfile);
					if(rombyte != PREFIXED_INSTRUCTIONS[i].byte) {
						errorInconsistentDataOpcode(rombyte, PREFIXED_INSTRUCTIONS[i].byte);
						return -1;
					}
					
					mempos++;
					mempos++;
					
					break;
				}
			} if(i >= sizeof(PREFIXED_INSTRUCTIONS)/sizeof(struct instruction)) {
				errorUnknownInstruction(opcode, arg1, arg2);
				return -1;
			}
		}}
		
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
					if(mempos >= 0x8000) {
						errorSectionCannotContainData();
						return -1;
					}
					char* data = mapstring((char*)token.content);
					for(unsigned int i = 0; data[i] != '\0'; i++) {
						unsigned char romdata = fgetc(romfile);
						if(data[i] != romdata) {
							errorInconsistentData(romdata, data[i]);
							return -1;
						}
						mempos++;
					}
				} else if(token.type == NEWLINE) {
					if(mempos < 0x8000) {
						unsigned long romdata = fgetc(romfile);
						if(romdata != 0) {
							errorInconsistentData(romdata, 0);
							return -1;
						}
					}
					mempos++;
					break;
				} else {
					if(mempos >= 0x8000) {
						errorSectionCannotContainData();
						return -1;
					}
					long result;
					unsigned long romdata = fgetc(romfile);
					restoreAsmPos();
					if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
					if(result < 0) result = (~(-result) & 0xFF) + 1;
					if(result != romdata) {
						errorInconsistentData(romdata, result);
						return -1;
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
					if(mempos >= 0x8000) {
						errorSectionCannotContainData();
						return -1;
					}
					char* data = mapstring((char*)token.content);
					for(unsigned int i = 0; data[i] != '\0'; i++) {
						unsigned int romdata = fgetc(romfile) | (fgetc(romfile) << 8);
						if((unsigned int)data[i] != romdata) {
							errorInconsistentData(romdata, data[i]);
							return -1;
						}
						mempos++;
						mempos++;
					}
				} else if(token.type == NEWLINE) {
					if(mempos < 0x8000) {
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
					if(mempos >= 0x8000) {
						errorSectionCannotContainData();
						return -1;
					}
					long result;
					unsigned long romdata = fgetc(romfile) | (fgetc(romfile) << 8);
					restoreAsmPos();
					if(calculateExpressionWithAssumption(romdata, 16, &result) != 0) return -1;
					if(result < 0) result = (~(-result) & 0xFFFF) + 1;
					if(result != romdata) {
						errorInconsistentData(romdata, result);
						return -1;
					}
				}
				mempos++;
				mempos++;
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
					if(mempos >= 0x8000) {
						errorSectionCannotContainData();
						return -1;
					}
					char* data = mapstring((char*)token.content);
					for(unsigned int i = 0; data[i] != '\0'; i++) {
						unsigned long romdata = fgetc(romfile) | (fgetc(romfile) << 8) | (fgetc(romfile) << 16) | (fgetc(romfile) | 24);
						if((unsigned long)data[i] != romdata) {
							errorInconsistentData(romdata, data[i]);
							return -1;
						}
						mempos++;
						mempos++;
						mempos++;
						mempos++;
					}
				} else if(token.type == NEWLINE) {
					if(mempos < 0x8000) {
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
					if(mempos >= 0x8000) {
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
			if(calculateExpression(&numfill) != 0) return -1;
			saveAsmPos();
			token = identifyNextToken();
			if(token.type == COMMA) {
				saveAsmPos();
				token = identifyNextToken();
			}
			if(token.type != NEWLINE) {
				if(mempos >= 0x8000) {
printf("%s ", TOKEN_STRINGS[token.type]);
if(charisliteral) printf("literal ");
					errorSectionCannotContainData();
					return -1;
				}
				long romfilepos = ftell(romfile);
				restoreAsmPos();
				token.type = COMMA;
				while(token.type == COMMA) {
					saveAsmPos();
					token = identifyNextToken();
					if(token.type == STRING) {
						char* data = mapstring((char*)token.content);
						for(unsigned int i = 0; data[i] != '\0'; i++) {
							fillbytes[fillbytecount++] = data[i];
						}
					} else {
						restoreAsmPos();
						long result;
						unsigned long romdata = fgetc(romfile);
						if(calculateExpressionWithAssumption(romdata, 8, &result) != 0) return -1;
						if(result < 0) result = (~(-result) & 0xFF) + 1;
						fillbytes[fillbytecount++] = (unsigned char) (result & 0xFF);
					}
					saveAsmPos();
					token = identifyNextToken();
				}
				fseek(romfile, romfilepos, SEEK_SET);
			}
			restoreAsmPos();
			unsigned int ii = 0;
			for(unsigned int i = 0; i < numfill; i++) {
				if(mempos < 0x8000) {
					unsigned char romdata = fgetc(romfile);
					if(fillbytes[ii++] != romdata) {
						errorInconsistentData(romdata, romdata);
					}
					ii %= fillbytecount;
				}
				mempos++;
			}
			break;}
		
		
		
		case DIR_CHARMAP:
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			charms(charmapcount).chars = malloc(strlen((char*)token.content)+1);
			strcpy(charms(charmapcount).chars, (char*)token.content);
			token = identifyNextToken();
			if(token.type != COMMA) {
				errorUnexpectedToken(token);
				return -1;
			}
			long result;
			if(calculateExpression(&result) != 0) return -1;
			charms(charmapcount).value = result;
			incrementCharmaplist;
			break;
		
		
		
		case DIR_RSSET:{
			long result;
			if(calculateExpression(&result) != 0) return -1;
			_RS = result;
			break;}
		
		case DIR_RSRESET:
			_RS = 0;
			break;
		
		
		
		case DIR_MACRO:
			// macro name
			token = identifyNextToken();
			if(token.type != UNRECORDED_SYMBOL) {
				errorUnexpectedToken(token);
				return -1;
			}
			macros(macrocount).name = malloc(strlen((char*)token.content)+1);
			strcpy(macros(macrocount).name, (char*)token.content);
			
			token = identifyNextToken();
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			// macro content
			macros(macrocount).content = malloc(1024);
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
				if(isWhitespace(c) && (isWhitespace(macros(macrocount).content[i-1]) || macros(macrocount).content[i-1] == '\n' || i == 0)) continue;
				if(c == '\n' && (isWhitespace(macros(macrocount).content[i-1]) || macros(macrocount).content[i-1] == '\n')) i--;
				if((isWhitespace(c) || c == '\n') && dirwordi < 6) {
					dirword[dirwordi] = '\0';
					if(strcmp(dirword, DIRECTIVE_STRINGS[DIR_MACRO]) == 0) {
						errorNestedMacros();
						return -1;
					}
					if(strcmp(dirword, DIRECTIVE_STRINGS[DIR_ENDM]) == 0) break;
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
			i -= strlen(DIRECTIVE_STRINGS[DIR_ENDM]);
			while(isWhitespace(macros(macrocount).content[i-1])) i--;
			macros(macrocount).content[i] = '\0';
			macros(macrocount).content = realloc(macros(macrocount).content, i+1);
			
			incrementMacrolist;
			
			break;
		
		
		
		case DIR_DEF: {
			struct token subject = identifyNextToken();
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
						varbls(variablecount).name = malloc(strlen((char*)subject.content)+1);
						strcpy(varbls(variablecount).name, (char*)subject.content);
						incrementVariablelist;
					} else {
						errorUnexpectedToken(subject);
						return -1;
					}
				} else if(procresult == 1 && subject.type == VARIABLE) {
					free(varbls(subject.content).name);
					variablecount--;
					for(unsigned int i = subject.content; i < variablecount; i++) varbls(i) = varbls(i+1);
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQU) {
				if(subject.type != UNRECORDED_SYMBOL) {
					errorUnexpectedToken(subject);
					return -1;
				}
				consts(constantcount).name = malloc(strlen((char*)subject.content)+1);
				strcpy(consts(constantcount).name, (char*)subject.content);
				long result;
				if(calculateExpression(&result) != 0) return -1;
				consts(constantcount).value = result;
				incrementConstantlist;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQUS) {
				if(subject.type != UNRECORDED_SYMBOL) {
					errorUnexpectedToken(subject);
					return -1;
				}
				costrs(conststrcount).name = malloc(strlen((char*)subject.content)+1);
				strcpy(costrs(conststrcount).name, (char*)subject.content);
				token = identifyNextToken();
				if(token.type != STRING) {
					errorUnexpectedToken(token);
					return -1;
				}
				costrs(conststrcount).content = malloc(strlen((char*)token.content)+1);
				strcpy(costrs(conststrcount).content, (char*)token.content);
				incrementConststrlist;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_RB) {
				if(subject.type != UNRECORDED_SYMBOL) {
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
					if (calculateExpression(&result) != 0) return -1;
					constexpr = (unsigned int) result;
				}
				consts(constantcount).name = malloc(strlen((char*)subject.content)+1);
				strcpy(consts(constantcount).name, (char*)subject.content);
				consts(constantcount).value = constexpr;
				incrementConstantlist;
				_RS += constexpr;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_RW) {
				if(subject.type != UNRECORDED_SYMBOL) {
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
					if (calculateExpression(&result) != 0) return -1;
					constexpr = (unsigned int) result;
				}
				constexpr *= 2;
				consts(constantcount).name = malloc(strlen((char*)subject.content)+1);
				strcpy(consts(constantcount).name, (char*)subject.content);
				consts(constantcount).value = constexpr;
				incrementConstantlist;
				_RS += constexpr;
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_RL) {
				if(subject.type != UNRECORDED_SYMBOL) {
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
					if (calculateExpression(&result) != 0) return -1;
					constexpr = (unsigned int) result;
				}
				constexpr *= 4;
				consts(constantcount).name = malloc(strlen((char*)subject.content)+1);
				strcpy(consts(constantcount).name, (char*)subject.content);
				consts(constantcount).value = constexpr;
				incrementConstantlist;
				_RS += constexpr;
			}
			
			else {
				errorUnexpectedToken(token);
				return -1;
			}
			
			break;}
		
		case DIR_REDEF: {
			charisliteral = true;
			struct token subject = identifyNextToken();
			charisliteral = false;
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
						varbls(variablecount).name = malloc(strlen((char*)subject.content)+1);
						strcpy(varbls(variablecount).name, (char*)subject.content);
						incrementVariablelist;
					} else {
						errorUnexpectedToken(subject);
						return -1;
					}
				} else if(procresult == 1 && subject.type == VARIABLE) {
					free(varbls(subject.content).name);
					variablecount--;
					for(unsigned int i = subject.content; i < variablecount; i++) varbls(i) = varbls(i+1);
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQU) {
				if(subject.type == UNRECORDED_SYMBOL) {
					consts(constantcount).name = malloc(strlen((char*)subject.content)+1);
					strcpy(consts(constantcount).name, (char*)subject.content);
					long result;
					if(calculateExpression(&result) != 0) return -1;
					consts(constantcount).value = result;
					incrementConstantlist;
				} else if(subject.type == CONSTANT) {
					long result;
					if(calculateExpression(&result) != 0) return -1;
					consts(subject.content).value = result;
				} else {
					errorUnexpectedToken(subject);
					return -1;
				}
			}
			
			else if(token.type == DIRECTIVE && token.content == DIR_EQUS) {
				if(subject.type == UNRECORDED_SYMBOL) {
					costrs(conststrcount).name = malloc(strlen((char*)subject.content)+1);
					strcpy(costrs(conststrcount).name, (char*)subject.content);
					token = identifyNextToken();
					if(token.type != STRING) {
						errorUnexpectedToken(token);
						return -1;
					}
					costrs(conststrcount).content = malloc(strlen((char*)token.content)+1);
					strcpy(costrs(conststrcount).content, (char*)token.content);
					incrementConststrlist;
				} else if(subject.type == CONSTANT_STRING) {
					free(costrs(subject.content).content);
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
			
			
			
		case DIR_IF:
			while(true) {
				long result;
				if(calculateExpression(&result) != 0) return -1;
				if(result != 0) {
					ifdepth++;
					break;
				}
				token = identifyNextToken();
				if(token.type != NEWLINE && token.type != END_OF_FILE) {
					errorUnexpectedToken(token);
					return -1;
				}
				unsigned int localifdepth = 0;
				charisliteral = true;
				while(token.type != DIRECTIVE || (token.content != DIR_ELIF && token.content != DIR_ELSE && token.content != DIR_ENDC) || localifdepth != 0) {
					if(token.type == DIRECTIVE && token.content == DIR_IF) localifdepth++;
					if(token.type == DIRECTIVE && token.content == DIR_ENDC) localifdepth--;
					token = identifyNextToken();
				}
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
			while(token.type != DIRECTIVE || token.content != DIR_ENDC) token = identifyNextToken();
		case DIR_ENDC:
			ifdepth--;
			break;
			
			
			
		case DIR_FOR:
			token = identifyNextToken();
			if(token.type != UNRECORDED_SYMBOL) {
				errorUnexpectedToken(token);
				return -1;
			}
			unsigned char returnrepeatsymbol[32];
			strcpy(returnrepeatsymbol, repeatsymbol);
			strcpy(repeatsymbol, (char*)token.content);
			unsigned long returnrepeatcount = repeatcount;
			
			token = identifyNextToken();
			if(token.type != COMMA) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			unsigned long repeatstart;
			unsigned long repeatstop = 0;
			unsigned long repeatstep = 1;
			if(calculateExpression(&repeatstop) != 0) return -1;
			
			token = identifyNextToken();
			if(token.type == COMMA) {
				repeatstart = repeatstop;
				if(calculateExpression(&repeatstop) != 0) return -1;
				
				token = identifyNextToken();
				if(token.type == COMMA) {
					if(calculateExpression(&repeatstep) != 0) return -1;
					token = identifyNextToken();
				}
			}
			
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				errorUnexpectedToken(token);
				return -1;
			}
			long repeatasmfilepos = ftell(asmfile);
			unsigned int repeatlinenumber = linenumber;
			unsigned int repeatungottencharcount = ungottencharcount;
			
			bool wasrepeating = repeating;
			repeating = true;
			for(repeatcount = repeatstart; repeatcount < repeatstop; repeatcount += repeatstep) {
				fseek(asmfile, repeatasmfilepos, SEEK_SET);
				linenumber = repeatlinenumber;
				ungottencharcount = repeatungottencharcount;
				
				int statementresult = 0;
				while(statementresult == 0) {
				  if(debug) printf("\n%d. ", linenumber);
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
			if(calculateExpression(&repeatnum) != 0) return -1;
			
			token = identifyNextToken();
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				errorUnexpectedToken(token);
				return -1;
			}
			long repeatasmfilepos = ftell(asmfile);
			unsigned int repeatlinenumber = linenumber;
			unsigned int repeatungottencharcount = ungottencharcount;
			
			bool wasrepeating = repeating;
			repeating = true;
			for(repeatcount = 0; repeatcount < repeatnum; repeatcount++) {
				fseek(asmfile, repeatasmfilepos, SEEK_SET);
				linenumber = repeatlinenumber;
				ungottencharcount = repeatungottencharcount;
				
				int statementresult = 0;
				while(statementresult == 0) {
				  if(debug) printf("\n%d. ", linenumber);
					statementresult = processNextStatement();
				}
				if(statementresult == 1) {
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
		
		
		
		case DIR_INCLUDE:
			if(currentsection[0] == '\0' && currentsection[1] == '\1') {
				return 2;
			}
			
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			FILE* returnasmfile = asmfile;
			long returnasmfilepos = ftell(asmfile);
			char* initialsection = malloc(strlen(currentsection)+1);
			strcpy(initialsection, currentsection);
			unsigned int returnlinenumber = linenumber;
			asmfile = fopen((char*)token.content, "rb");
			if(asmfile == NULL) {
				printf("Cannot find assembly file \"%s\".\n", (char*)token.content);
				return -1;
			}
			linenumber = 1;
			
		  if(debug) printf("\n\n%s:", (char*)token.content);
			int statementresult = 0;
			while(statementresult == 0) {
		  if(debug) printf("\n%d. ", linenumber);
				statementresult = processNextStatement();
				if(strcmp(initialsection, currentsection) != 0) return 0;
			}
			
			if(statementresult != 1) return statementresult;
			
			fclose(asmfile);
			asmfile = returnasmfile;
			fseek(asmfile, returnasmfilepos, SEEK_SET);
			linenumber = returnlinenumber;
		  if(debug) printf("\n\n:");
			
			break;
		
		
		
		case DIR_SECTION:
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			strcpy(currentsection, (char*)token.content);
			
			token = identifyNextToken();
			if(token.type != COMMA) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			token = identifyNextToken();
			if(token.type != REGION_TYPE) {
				errorUnexpectedToken(token);
				return -1;
			}
			currentregion = token.content;
			
			break;
		
		
		
		case DIR_UNION:
			token = identifyNextToken();
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				errorUnexpectedToken(token);
				return -1;
			}
			
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
			mempos = unionend;
			
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
			if(inunion) return 3;
			errorUnexpectedToken(token);
			return -1;
		
		
		
		case DIR_FAIL:
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			errorFail((char*)token.content);
			return -1;
			
		case DIR_WARN:
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			errorWarn((char*)token.content);
			break;
			
		case DIR_ASSERT: {
			long result;
			if(calculateExpression(&result) != 0) return -1;
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
				if(token.type != STRING) {
					errorUnexpectedToken(token);
					return -1;
				}
				if(result == 0) {
					errorAssertWithMessage((char*)token.content);
					return -1;
				}
			}
			break;}
			
		
		
		
		default:
			errorUnexpectedToken(token);
			return -1;
		}
		
		token = identifyNextToken();
		if(token.type != NEWLINE && token.type != END_OF_FILE) {
			errorUnexpectedToken(token);
			return -1;
		}
		
		break;
	
	
	
	case MACRO: {
		unsigned int macronum = token.content;
		
		// process arguments
		unsigned char* macroargs[64];
		unsigned int macroargcount = 0;
		char nextchar = getNextChar(asmfile);
		while(isWhitespace(nextchar)) nextchar = getNextChar(asmfile);
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
					macroargs[macroargcount][ii] = '\0';
					macroargs[macroargcount] = realloc(macroargs[macroargcount], ii+1);
					break;
				} else if(nextchar == ',') {
					macroargs[macroargcount][ii] = '\0';
					macroargs[macroargcount] = realloc(macroargs[macroargcount], ii+1);
					macroargcount++;
					ii = 0;
					macroargs[macroargcount] = malloc(1024);
					nextchar = getNextChar(asmfile);
					while(isWhitespace(nextchar)) nextchar = getNextChar(asmfile);
				} else {
					macroargs[macroargcount][ii++] = nextchar;
					nextchar = getNextChar(asmfile);
				}
			}
			macroargcount++;
		}
		
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
		
		unsigned int returnlinenumber = linenumber;
		
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
	// process pre-section
	while(currentsection[0] == '\0') {
	  if(debug) printf("\n%d. ", linenumber);
		int statementresult = processNextStatement();
		if(statementresult == 1) break;
		if(statementresult == 2) {
			// include
			struct token token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			FILE* returnasmfile = asmfile;
			long returnasmfilepos = ftell(asmfile);
			unsigned int returnlinenumber = linenumber;
			asmfile = fopen((char*)token.content, "rb");
			if(asmfile == NULL) {
				printf("Cannot find assembly file \"%s\".\n", (char*)token.content);
				return -1;
			}
			linenumber = 1;
			
		  if(debug) printf("\n\n%s:", token.content);
			int findresult = 0;
			while(findresult == 0) findresult = findSection(region, section);
			
			fclose(asmfile);
			asmfile = returnasmfile;
			fseek(asmfile, returnasmfilepos, SEEK_SET);
			linenumber = returnlinenumber;
			
			if(findresult != 1) return findresult;
		  if(debug) printf("\n\n:");
			
			token = identifyNextToken();
			if(token.type != NEWLINE && token.type != END_OF_FILE) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			continue;
		}
		if(statementresult != 0) return -1;
	}
	
	// skip over unrelated sections
	if(strcmp(currentsection, section) != 0) {
		charisliteral = true;
		
	  if(debug) printf("\n%d. ", linenumber);
		struct token token = identifyNextToken();
		while((token.type != DIRECTIVE || (token.content != DIR_SECTION && token.content != DIR_INCLUDE)) && token.type != END_OF_FILE) {
		  if(debug) if(token.type == NEWLINE) printf("\n%d. ", linenumber);
			token = identifyNextToken();
		}
		
		
		if(token.type == END_OF_FILE) return 1;
		
		
		// include
		if(token.content == DIR_INCLUDE) {
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			FILE* returnasmfile = asmfile;
			long returnasmfilepos = ftell(asmfile);
			unsigned int returnlinenumber = linenumber;
			asmfile = fopen((char*)token.content, "rb");
			if(asmfile == NULL) {
				printf("Cannot find assembly file \"%s\".\n", (char*)token.content);
				return -1;
			}
			linenumber = 1;
			
		  if(debug) printf("\n\n%s:", token.content);
			int findresult = 0;
			while(findresult == 0) findresult = findSection(region, section);
				
			fclose(asmfile);
			asmfile = returnasmfile;
			fseek(asmfile, returnasmfilepos, SEEK_SET);
			linenumber = returnlinenumber;
			
			if(findresult != 1) return findresult;
		  if(debug) printf("\n\n:");
			
			token = identifyNextToken();
		}
		
		
		// section
		else {
			charisliteral = false;
			
			token = identifyNextToken();
			if(token.type != STRING) {
				errorUnexpectedToken(token);
				return -1;
			}
			strcpy(currentsection, (char*)token.content);
			
			token = identifyNextToken();
			if(token.type != COMMA) {
				errorUnexpectedToken(token);
				return -1;
			}
			
			token = identifyNextToken();
			if(token.type != REGION_TYPE) {
				errorUnexpectedToken(token);
				return -1;
			}
			currentregion = token.content;
			
			token = identifyNextToken();
			if(token.type == MEMORY_OPEN) {
				long result;
				if(calculateExpression(&result) != 0) return -1;
				if(strcmp(currentsection, section) == 0) {
					mempos = result;
					// romfilepos?
				}
				token = identifyNextToken();
				if(token.type != MEMORY_CLOSE) {
					errorUnexpectedToken(token);
					return -1;
				}
				token = identifyNextToken();
			}
		}
		
		if(token.type != NEWLINE && token.type != END_OF_FILE) {
			errorUnexpectedToken(token);
			return -1;
		}
		
		return 0;
	}
	
	
	// process section
	else {
		charisliteral = false;
	  printf("Found %s!!\n", section);
	  bool wasdebug = debug;
	  debug = true;
		int statementresult = 0;
		while(statementresult == 0) {
		  if(debug) printf("\n%d. ", linenumber);
			statementresult = processNextStatement();
			if(strcmp(currentsection, section) != 0) {printf("\n\n"); debug = wasdebug; return 2;}
		}
	  if(debug) printf("\n\n");
	  debug = wasdebug;
		if(statementresult == 1) return statementresult;
		else return -1;
	}
}

int processSection(enum asmregiontype region, char* section) {
	for(unsigned int i = 0; i < sizeof(MAIN_FILES)/sizeof(MAIN_FILES[0]); i++) {
		while(macrocount > localmacrostart) {
			if(macrocount % 1024 == 0) free(macrolist[macrocount/1024]);
			macrocount--;
			free(macros(macrocount).name);
			free(macros(macrocount).content);
		}
		while(variablecount > localvariablestart) {
			if(variablecount % 1024 == 0) free(variablelist[variablecount/1024]);
			variablecount--;
			free(varbls(variablecount).name);
		}
		while(constantcount > localconstantstart) {
			if(constantcount % 1024 == 0) free(constantlist[constantcount/1024]);
			constantcount--;
			free(consts(constantcount).name);
		}
		while(conststrcount > localconststrstart) {
			if(conststrcount % 1024 == 0) free(conststrlist[conststrcount/1024]);
			conststrcount--;
			free(costrs(conststrcount).name);
			free(costrs(conststrcount).content);
		}
		charisliteral = false;
		instring = false;
		lastsymbolislocal = false;
		
		asmfile = fopen(MAIN_FILES[i], "rb");
		if(asmfile == NULL) {
			printf("Cannot find \"%s\"\n", MAIN_FILES[i]);
		}
		linenumber = 1;
		currentsection[0] = '\0';
		currentsection[1] = '\1';
		
		int findresult = 0;
	  if(debug) printf("\n\n%s:", MAIN_FILES[i]);
		while(findresult == 0) findresult = findSection(region, section);
		if(findresult == 2) return 0;
		if(findresult != 1) return -1;
	}
	
	errorSectionNotFound(section);
	return -1;
}

int processRegion(FILE* layoutfile, enum asmregiontype region, unsigned char bank, unsigned int address) {
	rewind(layoutfile);
	
	// find region
	while(true) {
		if(feof(layoutfile) != 0) return -1;
		
		char c;
		c = fgetc(layoutfile);
		while(isWhitespace(c)) c = fgetc(layoutfile);
		
		unsigned int ii;
		for(ii = 0; ii < strlen(REGION_STRINGS[region]); ii++) {
			if(c != REGION_STRINGS[region][ii]) break;
			c = fgetc(layoutfile);
		} if(ii == strlen(REGION_STRINGS[region])) {
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
				while(isBinary(c)) {
					value <<= 1;
					value += c - '0';
					c = fgetc(layoutfile);
				}
			} else if(c == '$') {
				c = fgetc(layoutfile);
				while(isHexadecimal(c)) {
					value *= 0x10;
					if(isHexadecimal(c)) value += c - '0';
					else if(isUppercase(c)) value += c - 'A' + 0xA;
					else if(isLowercase(c)) value += c - 'a' + 0xa;
					c = fgetc(layoutfile);
				}
			}
			
			while(isWhitespace(c)) c = fgetc(layoutfile);
			
			if(value == bank && c == '\n') break;
		}
		
		while(c != '\n' && feof(layoutfile) == 0) c = fgetc(layoutfile);
	}
	
	mempos = bank << 16 | (address & 0xFFFF);
	currentregion = region;
	
	
	
	// process region
	while(true) {
		char token[32];
		unsigned int i = 0;
		
		token[i] = fgetc(layoutfile);
		while(isWhitespace(token[i])) token[i] = fgetc(layoutfile);
		
		
		// process section
		if(token[i] == '\"') {
printf("%0X ", mempos);
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
			if(c != '\n') break;
			
			if(processSection(region, token) != 0) return -1;
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
				while(isBinary(token[i])) {
					value <<= 1;
					value += token[i] - '0';
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
			continue;
		}
		
		break;
	}
	
	printf("\nEnded processing of %s at %0X\n\n", REGION_STRINGS[region], mempos);
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
	
	mempos = 0;
	
	// open rom
	romfile = fopen(ROM_FILE, "rb");
	if(romfile == NULL) {
		printf("Cannot find ROM file \"%s\".\n", ROM_FILE);
		return -1;
	}
	
	
	
	// includes
	bool returndebug = debug;
	debug = false;
	for(unsigned int i = 0; i < sizeof(INCLUDE_FILES)/sizeof(INCLUDE_FILES[0]); i++) {
		asmfile = fopen(INCLUDE_FILES[i], "rb");
		if(asmfile == NULL) {
			printf("Cannot find \"%s\".\n", INCLUDE_FILES[i]);
			return -1;
		}
		linenumber = 1;
		
		int statementresult = 0;
		while(statementresult == 0) {
			
			statementresult = processNextStatement();
			
		}
		if(statementresult != 1) return statementresult;
		
		fclose(asmfile);
	}
	debug = returndebug;
	
	printf("\nProcessed include files\n");
	
	
	
	localmacrostart = macrocount;
	localvariablestart = variablecount;
	localconstantstart = constantcount;
	localconststrstart = conststrcount;
	
	
	
	// layout
	FILE* layoutfile = fopen(LAYOUT_FILE, "rb");
	if(layoutfile == NULL) {
		printf("Cannot find \"%s\".\n", LAYOUT_FILE);
		return -1;
	}
	if(processRegion(layoutfile, WRAM0, 0, 0xC000) != 0) return -1;
	if(processRegion(layoutfile, VRAM,  0, 0x8000) != 0) return -1;
	//if(processRegion(layoutfile, SRAM,  0, 0xA000) != 0) return -1;
	
	return 0;
	
	
	
	
	
	// process assembly file
	asmfile = fopen(argv[1], "rb"); // open the asm file
	if(asmfile == NULL) {
		printf("Cannot find assembly file \"%s\".\n", argv[1]);
		return -1;
	}
	
	linenumber = 1;
	
	
	
	// process game file
	romfile = fopen(argv[2], "rb"); // open the rom file
	if(romfile == NULL) {
		printf("Cannot find ROM file \"%s\".\n", argv[2]);
		return -1;
	}
	
	
	
	// validate the address argument, process it, and locate the address in rom file
	if(argc == 4) {
		if(strlen(argv[3]) != 7 || argv[3][2] != ':') {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
		
		if(argv[3][6] >= '0' && argv[3][6] <= '9')      mempos += (argv[3][6] - '0'     ) * 0x1;
		else if(argv[3][6] >= 'a' && argv[3][6] <= 'f') mempos += (argv[3][6] - 'a' + 10) * 0x1;
		else if(argv[3][6] >= 'A' && argv[3][6] <= 'F') mempos += (argv[3][6] - 'A' + 10) * 0x1;
		else {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
		
		if(argv[3][5] >= '0' && argv[3][5] <= '9')      mempos += (argv[3][5] - '0'     ) * 0x10;
		else if(argv[3][5] >= 'a' && argv[3][5] <= 'f') mempos += (argv[3][5] - 'a' + 10) * 0x10;
		else if(argv[3][5] >= 'A' && argv[3][5] <= 'F') mempos += (argv[3][5] - 'A' + 10) * 0x10;
		else {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
		
		if(argv[3][4] >= '0' && argv[3][4] <= '9')      mempos += (argv[3][4] - '0'     ) * 0x100;
		else if(argv[3][4] >= 'a' && argv[3][4] <= 'f') mempos += (argv[3][4] - 'a' + 10) * 0x100;
		else if(argv[3][4] >= 'A' && argv[3][4] <= 'F') mempos += (argv[3][4] - 'A' + 10) * 0x100;
		else {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
		
		if(argv[3][3] >= '0' && argv[3][3] <= '9')      mempos += (argv[3][3] - '0'     ) * 0x1000;
		else if(argv[3][3] >= 'a' && argv[3][3] <= 'f') mempos += (argv[3][3] - 'a' + 10) * 0x1000;
		else if(argv[3][3] >= 'A' && argv[3][3] <= 'F') mempos += (argv[3][3] - 'A' + 10) * 0x1000;
		else {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
		
		if(argv[3][1] >= '0' && argv[3][1] <= '9')      mempos += (argv[3][1] - '0'     ) * 0x10000;
		else if(argv[3][1] >= 'a' && argv[3][1] <= 'f') mempos += (argv[3][1] - 'a' + 10) * 0x10000;
		else if(argv[3][1] >= 'A' && argv[3][1] <= 'F') mempos += (argv[3][1] - 'A' + 10) * 0x10000;
		else {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
		
		if(argv[3][0] >= '0' && argv[3][0] <= '9')      mempos += (argv[3][0] - '0'     ) * 0x100000;
		else if(argv[3][0] >= 'a' && argv[3][0] <= 'f') mempos += (argv[3][0] - 'a' + 10) * 0x100000;
		else if(argv[3][0] >= 'A' && argv[3][0] <= 'F') mempos += (argv[3][0] - 'A' + 10) * 0x100000;
		else {
			printf("Invalid address \"%s\". Correct format is bb:aaaa.\n", argv[3]);
			return -1;
		}
	}
	
	fseek(romfile, mempos, SEEK_SET);
    mempos = ftell(romfile);
	
	
	
	// success
	// printf("Opened assembly file and rom file successfully.\n");
	// printf("Address: %02X:%04X\n\n", mempos>>16, mempos & 0xFFFF);
	
	
	
	// create new asm file, confirming it doesn't already exist (but only up to ten times)
	FILE* newasmfile;
	char newasmfilepath[strlen(argv[1])+2];
	newasmfilepath[0] = '\0';
	strcat(newasmfilepath,argv[1]);
	newasmfilepath[strlen(argv[1])] = '.';
	newasmfilepath[strlen(argv[1])+1] = '0';
	newasmfilepath[strlen(argv[1])+2] = '\0';
	// while((newasmfile = fopen(newasmfilepath, "r")) != NULL) {
	// 	fclose(newasmfile);
	// 	newasmfilepath[strlen(argv[1])+1]++;
	// 	if(newasmfilepath[strlen(argv[1])+1] > '9') {
	// 		printf("Too many %s.# exist. Clean up the directory before proceeding.\n", argv[1]);
	// 		return -1;
	// 	}
	// } fclose(newasmfile);
	
	newasmfile = fopen(newasmfilepath, "wb");
	if(newasmfile == NULL) {
		printf("Failed to create %s.\n", newasmfilepath);
		return -1;
	}
	
	
	// compare rom with asm
printf("\n\n%s:", argv[1]);
	while(feof(asmfile) == 0) {
printf("\n%d. ", linenumber);
		
		if(processNextStatement() != 0) return -1;
	
	}
	
	printf("Reached the end of the file at %02X:%04X. \n", mempos>>16, mempos & 0xFFFF);
	return 0;
}