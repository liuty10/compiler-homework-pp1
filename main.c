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
#define true 1
#define false 0
#define bool int

#define T_NULL	 	0
#define T_IntConstant 	1
#define T_String 	2
#define T_Void		3
#define T_Int		4
#define T_DoubleConstant 5
#define T_While		6
#define T_If		7
#define T_Else		8
#define T_Return	9
#define T_Break		10
#define T_BoolConstant	11
#define T_Or		12
#define T_LessEqual	13
#define T_GreaterEqual	14
#define T_Equal		15
#define T_NotEqual	16
#define T_StringConstant 17
#define T_Identifier 	18
#define T_Others	19
#define T_Unknown	20

int possible_category = 0;
int deterministic_category = 0;
bool hex_flag = false;
bool science_flag = false;
bool space_flag = false;

void print_usage(){
	printf("Usage: ./dcc -t -i inputfile -o [outputfile]\n");
	printf("\n");
	return;
}
void print_errors(){
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
	    ch == '.' || ch == '"' || ch == ':' || ch == '\n')
		return true;
	return false;
}

bool newTokenEnd(char* tokenBuffer, char *ch, int left, int *right, int len){
	deterministic_category = T_NULL;
	if(possible_category == T_NULL){
		hex_flag = false;
		science_flag = false;
		if(*ch >= '0' && *ch <='9')
			possible_category = T_IntConstant;
		else if(*ch >= 'A' && *ch <= 'z' || *ch == '_')
			possible_category = T_Identifier;
		else if(isDelimiter(*ch)==true){	//we stop here
			if(*ch == '!' || *ch == '>' || *ch == '<' || *ch == '='){
				if(*right==len || *(ch+1)!='='){
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='\0';
					deterministic_category = T_Others;
				}else{
					tokenBuffer[0]=*ch;
					tokenBuffer[1]='=';
					tokenBuffer[2]='\0';
					(*right)++;
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
			}else if(*ch == ' '){
				return true;
			}else if(*ch == '\n'){return false;}
			else{
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
			print_errors();
			return true;
		}
		return false;	
	}
	if(possible_category == T_IntConstant){
		if(*ch >= '0' && *ch <='9'){
			return false;
		}else if(*ch >= 'A' && *ch <= 'z' || *ch == '_'){
			if(hex_flag == false && (*ch == 'x' || *ch == 'X')){
				hex_flag = true;//do not change category yet.
				return false;
			}else if(hex_flag == true && ((*ch >='a' && *ch <= 'f')||(*ch >='A' && *ch <='F')||(*ch>='0' && *ch <='9'))){
				return false;//DO not change category
			}else{//need to change category
				if(hex_flag==true && (*(ch-1)=='x' || *(ch-1)=='X')){
					tokenBuffer[*right-left-1]='\0';
					*right=*right - 2;
				}else{
					tokenBuffer[*right-left]='\0';
					(*right)--;
				}
				deterministic_category = T_IntConstant;
				possible_category = T_NULL;
				return true;
			}
		}else if(isDelimiter(*ch)==true){	//we stop here
			if(*ch == '.'){
				tokenBuffer[*right-left]=*ch;
				possible_category = T_DoubleConstant;
				return false;
			}else if(*ch == '"'){// continuing until the end of line or next quote. Or, we need T_String
				tokenBuffer[*right-left] = '\0';
				(*right)--;
				deterministic_category = T_IntConstant;
				possible_category = T_NULL;
				return true;
			}else{
				tokenBuffer[*right - left]='\0';
				//if(*ch != ' '&& *ch !='\n')
					(*right)--;
				deterministic_category = T_IntConstant;
				possible_category = T_NULL;
				return true;
			}
		}else{
			print_errors();
			tokenBuffer[*right-left]='\0';
			(*right)--;
			deterministic_category = T_IntConstant;
			possible_category = T_NULL;
			return true;
		}
		return false;	
	}

	if(possible_category == T_DoubleConstant){
		if(*ch >= '0' && *ch <='9'){
			return false;
		}else if(*ch >= 'A' && *ch <= 'z' || *ch == '_'){
			if(science_flag == true){
				//*ch-1 is e or E, back 1 split
				if(*(ch-1) == 'e' || *(ch-1) == 'E'){
					tokenBuffer[*right-left-1]='\0';
					*right = *right - 2;
					deterministic_category = T_DoubleConstant;
					possible_category = T_NULL;
					return true;//double
				}
				//*ch-1 is + or -, back 2 split
				if(*(ch-1) == '+' || *(ch-1) == '-'){
					tokenBuffer[*right-left-2]='\0';
					*right = *right - 3;
					deterministic_category = T_DoubleConstant;
					possible_category = T_NULL;
					return true;//double
				}
				//*ch-1 is digit,  split
				if(*(ch-1) >= '0' && *(ch-1) <= '9'){
					tokenBuffer[*right-left]='\0';
					*right = *right - 1;
					deterministic_category = T_DoubleConstant;
					possible_category = T_NULL;
					return true;//double
				}
			}else if(*ch == 'e' || *ch == 'E'){
				science_flag = true;//do not change category yet.
				return false;
			}else{//need to change category
				tokenBuffer[*right-left]='\0';
				(*right)--;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}
		}else if(isDelimiter(*ch)==true){	//we stop here
			if(science_flag == true){
			if((*ch == '+' || *ch == '-') && (*(ch-1) == 'e' || *(ch-1)=='E')){
				tokenBuffer[*right-left]=*ch;
				possible_category = T_DoubleConstant;
				return false;
			}else if(*(ch-1)=='e'||*(ch-1)=='E'){// continuing until the end of line or next quote. Or, we need T_String
				tokenBuffer[*right-left-1] = '\0';
				*right = *right-2;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}else if(*(ch-1)=='+' || *(ch-1)=='-'){
				tokenBuffer[*right - left -2]='\0';
				*right = (*right)-3;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}}else{
				tokenBuffer[*right-left] = '\0';
				(*right)--;
			        deterministic_category = T_DoubleConstant;
				possible_category = T_NULL;
				return true;
			}
		}else{
			print_errors();
			tokenBuffer[*right-left]='\0';
			(*right)--;
			deterministic_category = T_DoubleConstant;
			possible_category = T_NULL;
			return true;
		}
		return false;	

	}

	if(possible_category == T_Identifier){
		if(isDelimiter(*ch)==true){
			tokenBuffer[*right-left] = '\0';
			(*right)--;
			deterministic_category = T_Identifier;
			possible_category = T_NULL;
			return true;
		}else{
			tokenBuffer[*right - left] = *ch;
			return false;
		}	
	}
	
	if(possible_category == T_StringConstant){
		if(*ch == '"'){
			tokenBuffer[*right-left] = *ch;
			tokenBuffer[*right-left+1] = '\0';
			deterministic_category = T_StringConstant;
			possible_category = T_NULL;
			return true;
		}else if(*right==len){
			print_errors();
			deterministic_category = T_StringConstant;//need further discuss
			possible_category = T_NULL;
			return true;
		}else
			return false;
	}

	return false;
}

int getTokens(char *inputLine, int cur_row, FILE* outputfile){
	char tokenBuffer[MAX_TOKEN_SIZE + 1];
	int left = 0, right = 0, i = 0;
	int stringLen = strlen(inputLine);
	
	while(right < stringLen && left <=right){
		if(newTokenEnd(tokenBuffer,&inputLine[right],left, &right, stringLen-1)){
			fputs(tokenBuffer, outputfile);
			printf("%s\t, row:%d\t, start:%d\t, end:%d\t\t, category:%d\n", tokenBuffer, cur_row, left, right, deterministic_category);
			right++;
			left = right;
		}else{//no new token, we should increase
			tokenBuffer[right-left]=inputLine[right];
			right++;
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

int main(int argc, char* argv[]){
	int o,i,row_num;
	int token_flag = 0;
	char szLineBuffer[MAX_LINE_SIZE + 1];//input buffer for fgets
	char *source_file_name = "";
	char *output_file_name = "a.out";
	FILE *source_file = NULL;
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
    	printf("start to process the source file\n");
	source_file = fopen(source_file_name, "r");
	output_file = fopen(output_file_name, "w+");
        if(token_flag == 1) printf("token flag is 1\n");
	else printf("token flag is 0\n");
	row_num = 0;
	while(fgets(szLineBuffer, MAX_LINE_SIZE, source_file)!=NULL){
		row_num++;
		if(szLineBuffer[0] == '\n') continue;
		if(getTokens(szLineBuffer, row_num, output_file)== -1){
			print_errors();
			break;
		}
	}
	fclose(source_file);
	fclose(output_file);
    	return 0;
}

