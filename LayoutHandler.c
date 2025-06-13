


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
					printf("\nError: Section \"%s\" was not found\n", token);
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