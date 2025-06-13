
int recordLabel(char* name) {
// if(strcmp(name, "SFX_Caught_Mon") == 0) {
// 	printLocation();
// 	printf("\n%s is defined as %0X\n", name, mempos);
// 	return -1;
// }
	labels(labelcount) = (struct label) { .name = name, .value = mempos};
	incrementLabellist;
}


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



enum asmregiontype currentregion = INVALID_REGION_TYPE;
unsigned char currentsection[1024];
unsigned char metasection[1024];



unsigned int ifdepth = 0;

unsigned long unionstart;
unsigned long unionend;
bool inunion = false;

bool loaddata = false;
bool loadlabels = false;




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
				printLocation();
				printf("\nError: Label cannot exist outside of a section\n");
				return -1;
			}
			
			unsigned int labelindex;
			
			if(token.type == LABEL) {
				if(labels(token.content).value != mempos) {
					printLocation();
					printf("\nError: Second definition of %s\n", labels(token.content).name);
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
		if(currentsection[0] == '\0') {
			errorDataCannotExistOutsideOfSection();
			return -1;
		}
		if((mempos & 0xFFFF) >= 0x8000 && !loadlabels) {
			errorSectionCannotContainData();
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
						if((mempos & 0xFFFF) >= 0x8000 && false) {
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
							if(currentmacro == 0) {
								
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
						printLocation();
						printf("\nError: Cannot nest macros\n");
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
					printLocation();
					printf("\nError: Macro started without end\n");
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
				printLocation();
				printf("\nError: Shift cannot exist outside of a macro\n");
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
						incrementVariablelist;
						if(assumptioncount % 1024 == 0) free(assumptionlist[assumptioncount/1024]);
						assumptioncount--;
						for(unsigned int i = subject.content; i < assumptioncount; i++) asmpts(i) = asmpts(i+1);
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
					incrementConstantlist;
				} else if(consts(subject.content).value != constexpr) {
					printLocation();
					printf("\nError: Redefinition of constant %s\n", consts(subject.content).name);
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
					incrementConstantlist;
				} else if(consts(subject.content).value != constexpr) {
					printLocation();
					printf("\nError: Redefinition of constant %s\n", consts(subject.content).name);
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
					incrementConstantlist;
				} else if(consts(subject.content).value != constexpr) {
					printLocation();
					printf("\nError: Redefinition of constant %s\n", consts(subject.content).name);
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
					printLocation();
					printf("\nError: FOR declared without matching ENDR\n");
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
					printLocation();
					printf("\nError: REPT declared without matching ENDR\n");
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
				printLocation();
				printf("\nError: UNION declared without matching ENDU\n");
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
			printLocation();
			printf("\nFailure: %s\n", (char*)token.content);
			return -1;
			
		case DIR_WARN:
			token = assertNextTokenType(STRING)
			printLocation();
			printf("\nWarning: %s\n", (char*)token.content);
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
					printLocation();
					printf("\nFailure: Assertion failed\n");
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
						printLocation();
						printf("\nFailure(%d)(%02X:%04X): %s\n", str2);
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
