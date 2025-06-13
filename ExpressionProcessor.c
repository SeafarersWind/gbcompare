




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
		printLocation();
		printf("\nError: Unbalanced parentheses\n");
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
				
					if((*result) >= (1 << numbits)) {
						printf("\nError: Value must be %d-bit\n", numbits);
						return -1;
					}
					if((*result) < 0) {
						(*result) *= -1;
						if((*result) >= (1 << numbits)) {
							printf("\nError: Value must be %d-bit\n", numbits);
							return -1;
						}
						(*result) = (1 << numbits) - (*result);
					}
					
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
