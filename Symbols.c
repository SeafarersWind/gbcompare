

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
