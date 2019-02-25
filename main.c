/* This is main file for decaf compiler: dcc
 * Author: Tianyi Liu
 * Email: liuty10@gmail.com
*/

/*
 Goal : Design a scanner to deal with decaf source code into TOKON.
 Task1: read source code from files line by line.
 Task2: for each line, read characters one by one to split the whole line into pre-TOKONS.
 Task3: For each pre-token, you need further process to get the final TOKON.
 Task4: Write the corresponding tokon back to a result file.
*/

/*
 How to use dcc (decaf gcc)?
 Goal: it should be simallar to gcc.
 Example: dcc -t input -o [output]

*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define MAX_LINE_SIZE 1000
#define MAX_TOKEN_SIZE 1000
#define MACRO_TABLE_SIZE 1000
#define true 1
#define false 0
#define bool int

// Define token types
#define T_NULL	 	0
#define T_BoolConstant	1
#define T_IntConstant 	2
#define T_DoubleConstant 3
#define T_StringConstant 4
#define T_Identifier 	25

#define T_Void		5
#define T_Int		6
#define T_Double	7
#define T_Bool		28
#define T_String 	8
#define T_Null	 	9
#define T_For		10
#define T_While		11
#define T_If		12
#define T_Else		13
#define T_Return	14
#define T_Break		15
#define T_Print		16
#define T_ReadInteger	17
#define T_ReadLine	18

#define T_Logic_Or	19
#define T_Logic_And	20
#define T_LessEqual	21
#define T_GreaterEqual	22
#define T_Equal		23
#define T_NotEqual	24

#define T_Others	26
#define T_Unknown	27

#define T_Bitwise_Or		29
#define T_Bitwise_And		30

#define T_Comment_Line		31
#define T_Comment_Multiple	32

//Define Error types
#define ERR_NULL		0
#define ERR_Unterminated	1
#define ERR_TooLongVariable  	2
#define ERR_UnknownSymbol  	3
#define ERR_InvalidDirective  	4
#define ERR_Others	  	5

int  possible_category = 0;
int  deterministic_category = 0;
bool hex_flag = false;
bool science_flag = false;
bool space_flag = false;
int  err_num = 0;

void print_usage(){
	printf("Usage: ./dcc -t -i inputfile -o [outputfile]\n");
	printf("\n");
	return;
}
void print_errors(int num_err, char* bufferStr, int row){
	switch(num_err){
		case ERR_Unterminated:
			printf("\n*** Error line %d.\n", row);
			printf("*** Unterminated string constant: %s\n\n", bufferStr);
			break;
		case ERR_TooLongVariable:
			printf("\n*** Error line %d.\n", row);
			printf("*** Inditifier too long: \"%s\".\n\n", bufferStr);
			break;
		case ERR_UnknownSymbol:
			printf("\n*** Error line %d.\n", row);
			printf("*** Unterminated recognized symbol: %s\n\n", bufferStr);
			break;
		case ERR_InvalidDirective:
			printf("\n*** Error line %d.\n", row);
			printf("*** Invalid # directive\n\n");
			break;
		default:
			break;
	}
	//printf("There is an error\n");
	return;
}

bool isDelimiter(char ch){
	if (ch == ' ' || ch == '+' || ch == '-' || ch == '*' ||
	    ch == '/' || ch == ',' || ch == ';' || ch == '>' ||
	    ch == '<' || ch == '=' || ch == '(' || ch == ')' ||
	    ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
	    ch == '!' || ch == '@' || ch == '#' || ch == '$' ||
	    ch == '^' || ch == '?' || ch == '|' || ch == '~' ||
	    ch == '.' || ch == '"' || ch == ':' || ch == '\n'||
	    ch == '&' || ch == '%' || ch == '`')
		return true;
	return false;
}

bool newTokenEnd(char* tokenBuffer, char *ch, int *pleft, int *pright, int len){
	deterministic_category = T_NULL;
	if(possible_category == T_NULL){
		hex_flag = false;
		science_flag = false;
		space_flag = false;
		if(*ch >= '0' && *ch <='9')
			possible_category = T_IntConstant;
		else if((*ch >= 'A' && *ch <= 'Z') || (*ch>='a' && *ch<='z') || *ch == '_')
			possible_category = T_Identifier;
		else if(isDelimiter(*ch)==true){	//we stop here
			if(*ch == '!' || *ch == '>' || *ch == '<' || *ch == '='){
				if(*pright==len || *(ch+1)!='='){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='\0';
					deterministic_category = T_Others;
				}else{
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='=';
					tokenBuffer[2]='\0';
					(*pright)++;
					if(*ch == '!') deterministic_category = T_NotEqual;
					if(*ch == '>') deterministic_category = T_GreaterEqual;
					if(*ch == '<') deterministic_category = T_LessEqual;
					if(*ch == '=') deterministic_category = T_Equal;
				}
				possible_category = T_NULL;
				return true;
			}else if(*ch == '.'){
				tokenBuffer[0]=*ch;
				tokenBuffer[1]='\0';
				deterministic_category = T_Others;
				possible_category = T_NULL;
				return true;
			}else if(*ch == '"'){// continuing until the end of line or next quote. Or, we need T_String
				possible_category = T_StringConstant;
				return false;
			}else if(*ch == ' '){
				(*pleft)++;
				(*pright)++;
				possible_category = T_NULL;
				space_flag = true;
				return false;
			}else if(*ch == '\n'){
				return false;
			}else if(*ch=='|'){
				if((*pright)==len || *(ch+1)!='|'){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='\0';
					deterministic_category = T_Bitwise_Or;
					possible_category = T_NULL;
					return true;
				}
				if(*(ch+1) == '|'){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]=*ch;
					tokenBuffer[2]='\0';
					deterministic_category = T_Logic_Or;
					possible_category = T_NULL;
					(*pright)++;
					return true;
				}else{
					print_errors(ERR_Others, tokenBuffer, 0);
				}
				
			}else if(*ch=='&'){
				if((*pright)==len || *(ch+1)!='&'){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='\0';
					deterministic_category = T_Bitwise_And;
					possible_category = T_NULL;
					return true;
				}
				if(*(ch+1) == '&'){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]=*ch;
					tokenBuffer[2]='\0';
					deterministic_category = T_Logic_And;
					possible_category = T_NULL;
					(*pleft)++;
					return true;
				}else{
					print_errors(ERR_Others, tokenBuffer,0);
				}
			}else if(*ch=='/'){
				if((*pright)<len){
					if(*(ch+1)=='/'){
						possible_category = T_Comment_Line;
						return false;
					}else if(*(ch+1)=='*'){
						possible_category = T_Comment_Multiple;
						(*pright)++;
						return false;
					}else{
						tokenBuffer[0]=*ch;
						tokenBuffer[1]='\0';
						deterministic_category = T_Others;
						possible_category = T_NULL;
						return true;
					}
				}else{
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='\0';
					deterministic_category = T_Others;
					possible_category = T_NULL;
					return true;
				}
				if((*pright)==len || *(ch+1)!='&'){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='\0';
					deterministic_category = T_Bitwise_And;
					possible_category = T_NULL;
					return true;
				}
				if(*(ch+1) == '&'){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]=*ch;
					tokenBuffer[2]='\0';
					deterministic_category = T_Logic_And;
					possible_category = T_NULL;
					(*pleft)++;
					return true;
				}else{
					print_errors(ERR_Others, tokenBuffer,0);
				}
			}else{
				tokenBuffer[0]=*ch;
				tokenBuffer[1]='\0';
				deterministic_category = T_Others;
				possible_category = T_NULL;
				return true;
			}
		}else{
			//TODO: T_Unknown
			possible_category = T_NULL;
			tokenBuffer[0]=*ch;
			tokenBuffer[1]='\0';
			deterministic_category = T_Unknown;
			err_num = ERR_UnknownSymbol;
			//print_errors();
			return true;
		}
		return false;	
	}
	if(possible_category == T_IntConstant){
		if(*ch >= '0' && *ch <='9'){
			return false;
		}else if((*ch >= 'A' && *ch <= 'Z') || (*ch>='a' && *ch<='z') || *ch == '_'){
			if(hex_flag == false && (*ch == 'x' || *ch == 'X')&&(*(ch-1) == '0')){
				hex_flag = true;//do not change category yet.
				return false;
			}else if(hex_flag == true && ((*ch >='a' && *ch <= 'f')||(*ch >='A' && *ch <='F')||(*ch>='0' && *ch <='9'))){
				return false;//DO not change category
			}else{//need to change category
				if(hex_flag==true && (*(ch-1)=='x' || *(ch-1)=='X')){
					tokenBuffer[(*pright)-(*pleft)-1]='\0';
					(*pright)=(*pright) - 2;
				}else{
					tokenBuffer[(*pright)-(*pleft)]='\0';
					(*pright)--;
				}
				deterministic_category = T_IntConstant;
				possible_category = T_NULL;
				return true;
			}
		}else if(isDelimiter(*ch)==true){	//we stop here
			if(*ch == '.'){
				tokenBuffer[(*pright)-(*pleft)]=*ch;
				possible_category = T_DoubleConstant;
				return false;
			}else if(*ch == '"'){// continuing until the end of line or next quote. Or, we need T_String
				tokenBuffer[(*pright)-(*pleft)] = '\0';
				(*pright)--;
				deterministic_category = T_IntConstant;
				possible_category = T_NULL;
				return true;
			}else{
				if(hex_flag==true && (*(ch-1)=='x' || *(ch-1)=='X')){
					tokenBuffer[(*pright) - (*pleft)-1]='\0';
					(*pright)-=2;
				}else{
					tokenBuffer[(*pright) - (*pleft)]='\0';
					//if(*ch != ' '&& *ch !='\n')
					(*pright)--;
				}
				deterministic_category = T_IntConstant;
				possible_category = T_NULL;
				return true;
			}
		}else{
			//print_errors();
			err_num = ERR_UnknownSymbol;
			tokenBuffer[(*pright)-(*pleft)]='\0';
			(*pright)--;
			deterministic_category = T_IntConstant;
			possible_category = T_NULL;
			return true;
		}
		return false;	
	}

	if(possible_category == T_DoubleConstant){
		if(*ch >= '0' && *ch <='9'){
			return false;
		}else if((*ch >= 'A' && *ch <= 'Z')|| (*ch >='a'&&*ch<='z') || *ch == '_'){
			if(science_flag == true){
				//*ch-1 is e or E, back 1 split
				if(*(ch-1) == 'e' || *(ch-1) == 'E'){
					tokenBuffer[(*pright)-(*pleft)-1]='\0';
					(*pright) = (*pright) - 2;
					deterministic_category = T_DoubleConstant;
					possible_category = T_NULL;
					return true;//double
				}
				//*ch-1 is + or -, back 2 split
				if(*(ch-1) == '+' || *(ch-1) == '-'){
					tokenBuffer[(*pright)-(*pleft)-2]='\0';
					(*pright) = (*pright) - 3;
					deterministic_category = T_DoubleConstant;
					possible_category = T_NULL;
					return true;//double
				}
				//*ch-1 is digit,  split
				if(*(ch-1) >= '0' && *(ch-1) <= '9'){
					tokenBuffer[(*pright)-(*pleft)]='\0';
					(*pright) = (*pright) - 1;
					deterministic_category = T_DoubleConstant;
					possible_category = T_NULL;
					return true;//double
				}
			}else if(*ch == 'e' || *ch == 'E'){
				science_flag = true;//do not change category yet.
				return false;
			}else{//need to change category
				tokenBuffer[(*pright)-(*pleft)]='\0';
				(*pright)--;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}
		}else if(isDelimiter(*ch)==true){	//we stop here
			if(science_flag == true){
			if((*ch == '+' || *ch == '-') && (*(ch-1) == 'e' || *(ch-1)=='E')){
				tokenBuffer[(*pright)-(*pleft)]=*ch;
				possible_category = T_DoubleConstant;
				return false;
			}else if(*(ch-1)=='e'||*(ch-1)=='E'){// continuing until the end of line or next quote. Or, we need T_String
				tokenBuffer[(*pright)-(*pleft)-1] = '\0';
				*pright = *pright-2;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}else if(*(ch-1)=='+' || *(ch-1)=='-'){
				tokenBuffer[(*pright) - (*pleft) -2]='\0';
				(*pright) = (*pright)-3;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}else{
				tokenBuffer[(*pright)-(*pleft)] = '\0';
				(*pright)--;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}

			}else{
				tokenBuffer[(*pright)-(*pleft)] = '\0';
				(*pright)--;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}
		}else{
			//print_errors();
			err_num = ERR_UnknownSymbol;
			tokenBuffer[(*pright)-(*pleft)]='\0';
			(*pright)--;
			deterministic_category = T_DoubleConstant;
			possible_category = T_NULL;
			return true;
		}
		return false;	

	}

	if(possible_category == T_Identifier){
		if(isDelimiter(*ch)==true){
			tokenBuffer[(*pright)-(*pleft)] = '\0';
			if(strlen(tokenBuffer)>32) err_num=ERR_TooLongVariable;
			(*pright)--;
			deterministic_category = T_Identifier;
			possible_category = T_NULL;
			return true;
		}else{
			tokenBuffer[(*pright) - (*pleft)] = *ch;
			return false;
		}	
	}
	
	if(possible_category == T_StringConstant){
		if(*ch == '"'){
			tokenBuffer[(*pright)-(*pleft)] = *ch;
			tokenBuffer[(*pright)-(*pleft)+1] = '\0';
			deterministic_category = T_StringConstant;
			possible_category = T_NULL;
			return true;
		}else if(*ch=='\n'||(*pright)==len){
			tokenBuffer[(*pright)-(*pleft)] = '\0';
			err_num = ERR_Unterminated;
			//print_errors();
			deterministic_category = T_StringConstant;//need further discuss
			possible_category = T_NULL;
			return true;
		}else
			return false;
	}

	if(possible_category == T_Comment_Multiple){
		if(*ch == '*' && *(ch+1) == '/'){
			deterministic_category = T_Comment_Multiple;
			possible_category = T_NULL;
			return true;
		}else
			return false;
	}

	return false;
}

int check_keyword(char* tokenBuffer){
	if(strcmp(tokenBuffer, "void")==0) 	return T_Void;
	if(strcmp(tokenBuffer, "int")==0) 	return T_Int;
	if(strcmp(tokenBuffer, "double")==0) 	return T_Double;
	if(strcmp(tokenBuffer, "bool")==0) 	return T_Bool;
	if(strcmp(tokenBuffer, "string")==0) 	return T_String;
	if(strcmp(tokenBuffer, "null")==0) 	return T_Null;
	if(strcmp(tokenBuffer, "for")==0) 	return T_For;
	if(strcmp(tokenBuffer, "while")==0) 	return T_While;
	if(strcmp(tokenBuffer, "if")==0) 	return T_If;
	if(strcmp(tokenBuffer, "else")==0) 	return T_Else;
	if(strcmp(tokenBuffer, "return")==0) 	return T_Return;
	if(strcmp(tokenBuffer, "break")==0) 	return T_Break;
	if(strcmp(tokenBuffer, "Print")==0) 	return T_Print;
	if(strcmp(tokenBuffer, "ReadInteger")==0) return T_ReadInteger;
	if(strcmp(tokenBuffer, "ReadLine")==0) 	return T_ReadLine;
	if(strcmp(tokenBuffer, "false")==0) 	return T_BoolConstant;
	if(strcmp(tokenBuffer, "true")==0) 	return T_BoolConstant;
	return T_Identifier;	
}

int getTokens(char *inputLine, int cur_row, FILE* outputfile){
	char tokenBuffer[MAX_TOKEN_SIZE + 1];
	int left = 0, right = 0, i = 0;
	int stringLen = strlen(inputLine);
	
	while(right < stringLen && left <=right){
		if(newTokenEnd(tokenBuffer,&inputLine[right],&left, &right, stringLen-1)){
			if(deterministic_category == T_Comment_Multiple){
				possible_category = T_NULL;
				right+=2;
				left = right;
				err_num = ERR_NULL;
				continue;
			}
			//fputs(tokenBuffer, outputfile);
			if(err_num!=ERR_NULL && err_num != ERR_TooLongVariable){
				print_errors(err_num, tokenBuffer, cur_row);
			}else{
			if(deterministic_category == T_Identifier){
				deterministic_category = check_keyword(tokenBuffer);
			}
			switch(deterministic_category){
				case T_IntConstant:
					printf("%s\t\tline %d cols %d-%d is T_IntConstant (value = %s)\n", tokenBuffer, cur_row, left+1, right+1, tokenBuffer);
					break;
				case T_String:
					printf("%s\t\tline %d cols %d-%d is T_String\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Void:
					printf("%s\t\tline %d cols %d-%d is T_Void\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Int:
					printf("%s\t\tline %d cols %d-%d is T_Int\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Double:
					printf("%s\t\tline %d cols %d-%d is T_Double\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Bool:
					printf("%s\t\tline %d cols %d-%d is T_Bool\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Null:
					printf("%s\t\tline %d cols %d-%d is T_Null\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_For:
					printf("%s\t\tline %d cols %d-%d is T_For\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Print:
					printf("%s\t\tline %d cols %d-%d is T_Print\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_ReadInteger:
					printf("%s\t\tline %d cols %d-%d is T_ReadInteger\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_ReadLine:
					printf("%s\t\tline %d cols %d-%d is T_ReadLine\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_BoolConstant:
					printf("%s\t\tline %d cols %d-%d is T_BoolConstant (value = %s)\n", tokenBuffer, cur_row, left+1, right+1, tokenBuffer);
					break;
				case T_DoubleConstant:
					printf("%s\t\tline %d cols %d-%d is T_DoubleConstant (value = %s)\n", tokenBuffer, cur_row, left+1, right+1, tokenBuffer);
					break;
				case T_While:
					printf("%s\t\tline %d cols %d-%d is T_While\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_If:
					printf("%s\t\tline %d cols %d-%d is T_If\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Else:
					printf("%s\t\tline %d cols %d-%d is T_Else\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Return:
					printf("%s\t\tline %d cols %d-%d is T_Return\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Break:
					printf("%s\t\tline %d cols %d-%d is T_Break\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Logic_And:
					printf("%s\t\tline %d cols %d-%d is T_Logic_And\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Logic_Or:
					printf("%s\t\tline %d cols %d-%d is T_Logic_Or\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Bitwise_And:
					printf("%s\t\tline %d cols %d-%d is T_Bitwise_And\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Bitwise_Or:
					printf("%s\t\tline %d cols %d-%d is T_Bitwise_Or\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_LessEqual:
					printf("%s\t\tline %d cols %d-%d is T_LessEqual\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_GreaterEqual:
					printf("%s\t\tline %d cols %d-%d is T_GreaterEqual\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_Equal:
					printf("%s\t\tline %d cols %d-%d is T_Equal\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_NotEqual:
					printf("%s\t\tline %d cols %d-%d is T_NotEqual\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				case T_StringConstant:
					printf("%s\t\tline %d cols %d-%d is T_StringConstant (value = %s)\n", tokenBuffer, cur_row, left+1, right+1, tokenBuffer);
					break;
				case T_Identifier:
					if(err_num == ERR_TooLongVariable){
						printf("\n*** Error line %d.\n", cur_row);
						printf("*** Inditifier too long: \"%s\".\n\n", tokenBuffer);
						printf("%s\t\tline %d cols %d-%d is T_Identifier ", tokenBuffer, cur_row, left+1, right+1);
						tokenBuffer[31]='\0';
						printf("(truncated to %s)\n", tokenBuffer);
					}else{
						printf("%s\t\tline %d cols %d-%d is T_Identifier\n", tokenBuffer, cur_row, left+1, right+1);
					}
					break;
				case T_Others:
					printf("%s\t\tline %d cols %d-%d is '%s'\n", tokenBuffer, cur_row, left+1, right+1, tokenBuffer);
					break;
				case T_Unknown:
					printf("%s\t\tline %d cols %d-%d is T_Unknown\n", tokenBuffer, cur_row, left+1, right+1);
					break;
				default:
					break;
			}}
			right++;
			left = right;
			err_num = ERR_NULL;
		}else{//no new token, we should increase
			if(possible_category == T_Comment_Line){
				possible_category = T_NULL;
				break;
			}
			if(possible_category == T_Comment_Multiple){
				right++;
			}else if(space_flag == false){
				tokenBuffer[right-left]=inputLine[right];
				right++;
			}else{;}
		}
	}
	return 0;
}

struct option long_options[] = {
	{"tokon", 	no_argument, 	  0, 't'}, // generate token file
	{"input", 	required_argument, 0, 'i'}, // input file
	{"output",	required_argument, 0, 'o'}, // output file
	{"help",	no_argument, 	  0, 'h'}, // output file
	{0, 0, 0, 0}
};

struct MacroDefination{
        int valid;
	char macro[MAX_LINE_SIZE];
	char value[MAX_LINE_SIZE];
};

struct MacroDefination MacroTable[MACRO_TABLE_SIZE];

void insertMacroTable(char* symbol, char * value){
	int i=0;
	while(i<MACRO_TABLE_SIZE && MacroTable[i].valid == true){
		i++;
	}
	if(i< MACRO_TABLE_SIZE){
		MacroTable[i].valid = true;
		strcpy(MacroTable[i].macro, symbol);
		strcpy(MacroTable[i].value, value);
		
	}else{
		printf("macro table length too small\n");
	}
	
}

bool checkMacroTable(char *symbol){
	int i=0;
	while(i<MACRO_TABLE_SIZE && MacroTable[i].valid == true){
		if(strcmp(symbol, MacroTable[i].macro)==0){
			//symbol = MacroTable[i].value;
			strcpy(symbol, MacroTable[i].value);
			return true;
		}else{
			i++;
		}
	}
	if(i>= MACRO_TABLE_SIZE){
		symbol[0]='\0';
	}
	return false;

}


void findMacroAndReplace(char* input, char* output, int row){
	int len = strlen(input);
	int i = 0;
	int state = 0;
	int index = 0;
	int left = 0;
	int right = 0;
	char macro_symbol[MAX_LINE_SIZE];
	char macro_value[MAX_LINE_SIZE];
	char *macro_new_line = output;
	while(left<=right && right < len){
		if(state == 0){
			if(input[right]=='\n'){
				macro_new_line[index] = '\0';
				break;
			}
			if(input[right] != '#'){
				macro_new_line[index] = input[right];
				index++;
				right++;
				left = right;
				continue;
			}else{
				right++;
				left = right;
				state = 1;
			}
		}
		if(state == 1){
			//if(input[right]!=' ' && input[right]!='\n'){
			if(isDelimiter(input[right])==false){
				macro_symbol[right-left] = input[right];
				right++;
				continue;
			}else{//from left to right-1
				macro_symbol[right-left] = '\0';
				if(strcmp(macro_symbol, "define") == 0){
					right++;
					left = right;
					state = 2;
				}else{
					if(checkMacroTable(macro_symbol)==true){//replace
						int i=0;
						while(macro_symbol[i]!='\0'){
							macro_new_line[index] = macro_symbol[i];
							index++;
							i++;
						}
					}else{
						print_errors(ERR_InvalidDirective,NULL,row);
						macro_new_line[0]='\0';
						break;
					}
					state = 0;
				}
			}
		}
		if(state == 2){
			if(input[right]==' ' && right==left){
				right++;
				left=right;
				continue;
			}else if(	
				(input[right]>='0'&&input[right]<='9') ||
				(input[right]>='A'&&input[right]<='Z') || 
				(input[right]>='a'&&input[right]<='z') || 
				input[right] == '_'){
				macro_symbol[right-left] = input[right];
				right++;
			}else if(input[right]==' '){
				macro_symbol[right-left] = '\0';
				right++;
				left = right;
				if(macro_symbol[0]>='0' && macro_symbol[0]<='9'){
					print_errors(ERR_InvalidDirective,NULL,row);
					macro_new_line[0]='\0';
					break;
				}else{
					state = 3;
				}
			}else{
				print_errors(ERR_InvalidDirective,NULL,row);
				macro_new_line[0]='\0';
				break;
			}
		}
		if(state == 3){
			if(input[right]!='\n' && input[right]!='\0'){
				macro_value[right-left] = input[right];
				right++;
			}else{
				macro_value[right-left] = '\0';
				insertMacroTable(macro_symbol, macro_value);
				macro_new_line[0]='\0';
				break;
			}
		}
	}
}

void pre_processor(FILE* source, FILE* dest){
	char szLineBuffer[MAX_LINE_SIZE + 1];   //input buffer for fgets
	char afterLineBuffer[MAX_LINE_SIZE + 1];//output buffer for fputs
	int row = 0;
	while(fgets(szLineBuffer, MAX_LINE_SIZE, source)!=NULL){
		row++;
		if(szLineBuffer[0] == '\n') {
			fputs(szLineBuffer, dest);
		continue;}
		findMacroAndReplace(szLineBuffer, afterLineBuffer, row);
		fputs(afterLineBuffer, dest);
		fputs("\n\0", dest);
	}
	return;
}

int main(int argc, char* argv[]){
	int o,i,row_num;
	int token_flag = 0;
	char szLineBuffer[MAX_LINE_SIZE + 1];//input buffer for fgets
	char *source_file_name = "";
	char *processed_file_name = "tmp.out";
	char *output_file_name = "a.out";

	FILE *source_file = NULL;
	FILE *processed_file = NULL;
	FILE *output_file = NULL;

    	while(1){
		o = getopt_long(argc, argv, "ht:i:o:", long_options, NULL);
		if(o == -1) break;
		switch(o){
		case 'h':
			print_usage();
			exit(0);
		case 'i':
			source_file_name = strdup(optarg);
			break;
		case 'o': 
			output_file_name = strdup(optarg);
			break;
		case 't':
			token_flag = 1;
			printf("token_flag is 1\n");
			break;
		default:
			printf("Unknow argument: %d\n", o);
			print_usage();
			exit(0);
		}
    	}
	source_file 	= fopen(source_file_name, "r");
	processed_file 	= fopen(processed_file_name, "w+");
	pre_processor(source_file, processed_file);
	row_num = 0;
	rewind(processed_file);
	while(fgets(szLineBuffer, MAX_LINE_SIZE, processed_file)!=NULL){
		row_num++;
		if(szLineBuffer[0] == '\n') continue;
		if(getTokens(szLineBuffer, row_num, output_file)== -1){
			break;
		}
	}
	fclose(source_file);
	fclose(processed_file);
    	return 0;
}

