


struct token {
	enum asmtoken type;
	unsigned int content;
};



bool charisliteral = false;
bool firsttoken = false;
bool indef = false;
bool newuniqueaffix = false;

unsigned char ungottenchars[1024];
unsigned int ungottencharcount = 0;
unsigned char** expandedstrs;
unsigned int expandedstrcount = 0;
unsigned int* expandedstrspos;

unsigned char* currentmacro = 0;
unsigned int currentmacropos;
unsigned char** currentargs = 0;

char symbolstr[1024];
unsigned char labelscope[1024];
unsigned char* locallabelscope;

char bin0 = '0';
char bin1 = '1';

unsigned long repeatcount = 0;
unsigned char repeatsymbol[32];
bool repeating = false;



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
				// handle unique affix
				if(uniqueaffixpos < 256) {
					// looking for new symbol
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
					}
					// looking for last symbol
					else {
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
