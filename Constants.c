


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

long predeclaredsymbols[PREDEFS_COUNT];
#define _NARG predeclaredsymbols[NARG]
#define _RS predeclaredsymbols[RS]



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



enum asmtoken {
	OPCODE, DIRECTIVE, PREDECLARED_SYMBOL, REGISTER, CONDITION, NUMBER, STRING, OPERATOR, COMMA, MEMORY_OPEN, MEMORY_CLOSE,
	CONSTANT, CONSTANT_STRING, VARIABLE, MACRO, LABEL, ASSUMPTION, UNRECORDED_SYMBOL, TEXT_MACRO, NEWLINE, REGION_TYPE, END_OF_FILE, UNKNOWN_TOKEN
};
const char* TOKENS[] = {
	"opcode", "directive", "predefined symbol", "register", "condition", "number", "string", "operator", ",", "[", "]",
	"constant", "constant string", "variable", "macro", "label", "assumption", "unrecorded symbol", "text macro", "newline", "region type", "end of file", "?????"
};
#define TOKENS_COUNT sizeof(TOKENS)/sizeof(TOKENS[0])

#define isExpression(t) (t == DIRECTIVE || t == NUMBER || t == STRING || t == OPERATOR || t == CONSTANT || t == VARIABLE || t == LABEL || t == ASSUMPTION || t == UNRECORDED_SYMBOL)
#define case_SCALAR case DIRECTIVE: case NUMBER: case STRING: case OPERATOR: case CONSTANT: case VARIABLE: case LABEL: case ASSUMPTION: case UNRECORDED_SYMBOL


