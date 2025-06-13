
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

void errorSectionCannotContainData() {
	printLocation();
	printf("\nError: Section cannot contain code or data\n");
}

void errorDataCannotExistOutsideOfSection() {
	printLocation();
	printf("\nError: Code or data cannot exist outside of a section\n");
}

void errorUnableToCalculate() {
	printLocation();
	printf("\nError: Unable to calculate expression\n");
}
