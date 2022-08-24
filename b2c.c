/*
 * Parser for BASIC like language
 * 
 * Copyright 2014 Ashok Shankar Das <ashok.s.das@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
//#include "symtab.c"

FILE *source=NULL, *target=NULL, *subf=NULL;

/* ============= Boolean Defination =============== */
typedef enum { false=0,true} bool;

/* ==============Token types ====================== */
typedef enum { 	t_invalid_token=0, t_ident,t_number, t_literal, 
		t_addop, t_mulop, t_lparen, t_rparen, t_less, 
		t_leq, t_neq, t_eql, t_geq, t_gt, 
		t_punctuation, t_keyword,t_dtyp, t_eol,t_skip	} type_of_token;

char *tok_typ[]={"INVALID","IDENTIFIER","NUMBER","LITERAL",
		  "ADDOP", "MULOP", "LPAREN", "RPAREN", "LESS",
		 "LESSEQL", "NOTEQL", "EQUAL", "GREATEQ", "GREATER",
			"PUNCT", "KEYWORD", "DTYP", "EOL", "skip"};
/* ================== Keywords ==================== */

char *keywords[] = { "LET", "FOR", "TO", "STEP", "NEXT", "DIM",
	       "IF", "THEN", "ELSE", "ENDIF", "READ",
	       "DATA", "END", "FUNCTION", "RETURN", "PROGRAM",
	       "INPUT", "PRINT", "INT", "LEFT$", "RIGHT$",
	       "MID$", "CHR", "ASC", "SUB", "RANDOMIZE",
	       "REM", "RND", "AS"};
#define keyword_count sizeof(keywords)/sizeof(char *)

char *type_specifiers[]={"BYTE","INTEGER","LONG","DOUBLE"};
#define dtype_count sizeof(type_specifiers)/sizeof(char *)

/* ========= Double linklist Implementation ====== */
// define a doubly linked list to represent a token.
typedef struct token
{
	char value[50];		// string value
	type_of_token t_type;	// type
	struct token *prev;	// previous token to see previous
	struct token *next;	// next token 
}token;
token *token_list = NULL;		// list which represents the tokens of the program
// Function create_token takes a string and tokentype
// returns pointer to token
token *create_token(char *val, type_of_token t_typ)
{
	token *t =NULL;
	t=(token*)malloc(sizeof(struct token));
	if(t == NULL)
	{
		printf("Error: Unable to allocate memory for token\n");
		exit(-1);
	}
	bzero(t,sizeof(struct token));
	strcpy(t->value,val);
	t->t_type = t_typ;
	t->prev = NULL;
	t->next = NULL;
	return(t);
}

// function push_back puts a token in token_list
// accepts a pointer to a token
// returns a pointer to a token
token *push_back(token *t)
{
	token *temp = token_list;
	// add at the end
	if (token_list)
	{
		temp = token_list;
		while(temp->next != NULL)
			temp=temp->next;
		temp->next=t;
		t->prev = temp;
	}
	else
	{
		token_list = t;
	}
	return (token_list);
}
// Function push_front puts a token at the front of a list
// accepts a pointer to token
// returns a pointer to token
token *push_front(token *t)
{
	if(token_list)
	{
		t->next=token_list;
		token_list->prev = t;
		t->prev = NULL;
		token_list = t;
	}
	else
	{
		token_list = t;
	}
	return(token_list);
}

// Function display_list traverses the token_list and prints the token information
void display_list()
{
	token *list;
	list = token_list;
	int line = 1;
	//printf("DEBUG: token_list at %p\n",token_list);
	while(list!=NULL)
	{
		printf("%s [ %s ]\n",list->value,tok_typ[list->t_type]);
		if(list->t_type == t_eol)
			line +=1;
		list = list->next;
	}
	printf("Total Lines scanned = %d\n",line-1);
}

/* ================ lookahead ==================== */
// Function look look for the next character in the stream
// accepths a FILE pointer
// return a character
char look(FILE *source)
{
	char ch='\0';
	if(!feof(source))
	{
		ch = fgetc(source);
		ungetc(ch,source);
	}
	return ch;
}
/* ============== sacnner Proper =================== */
// Function Parse, actual parser implementation actually it is lexer and parser combined
// accepts FILE pointer
// returns boolean value success or failure
bool scanner(FILE *source)
{
	char peekc, inpc;
	char tmpstr[50];
	bzero(tmpstr,sizeof(tmpstr));
	token *curr_token =NULL;
	type_of_token t_typ = 0; //invalid token 
	
	while(!feof(source))
	{
		peekc= look(source);
		//while(peekc == ' ' || peekc == 0x09)
		//	peekc = (char)fgetc(source);
		inpc = (char)fgetc(source);
		while(true)
		{
			if(isalpha(inpc))	/* symbol */
			{
				int i=0;
				tmpstr[i]=inpc;
				i++;
				bool kw = false;
				while(true)
				{
					peekc = look(source);
					if(isalnum(peekc) || peekc == '_')
					{
						inpc = (char)fgetc(source);
						tmpstr[i] = inpc;
						i++;
					}
					else
					{
						tmpstr[i]='\0';
						break;
					}
				}
				
				for(i =0; i< keyword_count; i++)
				{
					if(strcasecmp(tmpstr,keywords[i])==0)
					{
						kw = true;
						break;
					}
				}
				if(kw == false)
				{
					t_typ = t_ident;
					if(strcasecmp(tmpstr,"AS")==0)
						t_typ = t_skip;
					for(i=0;i<dtype_count;i++)
					{
						if(strcasecmp(tmpstr,type_specifiers[i])==0)
						{
							t_typ = t_dtyp;
							break;
						}
					}
				}
				else
				{
					t_typ = t_keyword;
				}
				break;
			}
			if(isdigit(inpc)) // number 
			{
				int i = 0;
				tmpstr[i] = inpc;
				i++;
				while(true)
				{
					peekc=look(source);
					if(isdigit(peekc) || peekc == '.')
					{
						inpc = (char)fgetc(source);
						tmpstr[i] = inpc;
						i++;
					}
					else
					{
						tmpstr[i] = '\0';
						break;
					}
				}
				t_typ = t_number;
				break;
			}
			if(inpc == '\"')	//literal
			{
				int i=0;
				while(look(source) != '\"')
				{
					tmpstr[i] = (char)fgetc(source);
					i++;
				}
				tmpstr[i] = '\0';
				fgetc(source);
				t_typ = t_literal;
				break;
			}
			if(inpc == '\n')	// end of line
			{
				t_typ = t_eol;
				strcpy(tmpstr,"eol");
				break;
			}
			if(ispunct(inpc))	// punctuation
			{
				int i = 0;
				tmpstr[i] = inpc;
				i++;
				t_typ = t_punctuation;
				switch(inpc)
				{
					case '<':	t_typ = t_less;
							peekc = look(source);
							if(peekc == '>')
							{
								t_typ = t_neq;
								inpc = fgetc(source);
								tmpstr[i] = inpc;
								i++;
							}
							else if( peekc == '=')
							{
								t_typ = t_leq;
								inpc = fgetc(source);
								tmpstr[i] = inpc;
								i++;
							}
							break;

					case '>':	t_typ = t_gt;
							peekc = look(source);
							if(peekc == '=')
							{
								t_typ = t_geq;
								inpc = fgetc(source);
								tmpstr[i] = inpc;
								i++;
							}
							break;
					case '+':
					case '-':	t_typ = t_addop;
							break;
					case '*':
					case '/':	t_typ = t_mulop;
							break;
					case '=':	t_typ = t_eql;
							break;
					case '(':	t_typ = t_lparen;
							break;
					
				}
				
				break;
			}			
			break;
		}
		if(t_typ != 0)
		{
			curr_token = create_token(tmpstr,t_typ);
			token_list = push_back(curr_token);
			bzero(tmpstr,sizeof(tmpstr));
			curr_token = NULL;
			t_typ = 0;
		}
	}
	fclose(source);
	return true;
}
void do_header(FILE *target)
{
	if(target == NULL)
		exit(-1);
 
	fprintf(target,"#include <stdio.h>\n");
	fprintf(target,"#include <stdlib.h>\n");
	fprintf(target,"#include <string.h>\n");
	fprintf(target,"#include <math.h>\n");
	fprintf(target,"#include \"subrtn.c\"\n");
	fprintf(target,"int main(int argc, char *argv[])\n{\n"); 
}
void do_rem(FILE *target, token *rt)
{
	fprintf(target,"/* ");
	while(token_list->t_type != t_eol)
	{
		fprintf(target,"%s ",token_list->value);
		token_list = token_list->next;
	}
	fprintf(target,"*/\n");
}
void do_dim(FILE *target, token *tmtk)
{
	token *temp = token_list;
	char btype[20]="";
	while(temp->next->t_type!=t_eol)
		temp = temp->next;
	if(strcasecmp(temp->value,"byte")==0)
		strcpy(btype,"unsigned char");
	else if(strcasecmp(temp->value,"integer")==0)
		strcpy(btype,"int");
	else if(strcasecmp(temp->value,"long")==0)
		strcpy(btype,"long");
	else if(strcasecmp(temp->value,"double")==0)
		strcpy(btype,"double");
	else if(strcasecmp(temp->value,"string")==0)
		strcpy(btype,"char*");
	else
	{
		printf("Error in DIM statement found %s expecting valid datatype\n",temp->value);
		fclose(target);
		exit(0);
	}
	// now extract the variable name
	char varnam[20]="";
	token_list = token_list->next;
	if(token_list->t_type!=t_ident)
	{
		printf("Error in DIM statement found %s need variable\n",token_list->value);
		fclose(target);
		exit(0);
	}
	strcpy(varnam,token_list->value);
	token_list = token_list->next;
	if(token_list->t_type == t_lparen)
	{
		strcat(varnam,"[");
		token_list = token_list->next;
		if(token_list->t_type != t_number)
		{
			printf("Error in DIM statement found %s need integer\n",temp->value);
			fclose(target);
			exit(0);
		}
		strcat(varnam,token_list->value);
		strcat(varnam,"]");
	}
	fprintf(target,"%s %s;\n",btype,varnam);
	while(token_list->t_type != t_eol)
		token_list = token_list->next;
}
void do_if(FILE *target, token *iftk)
{
	char sym[20]="";	
	fprintf(target,"if( ");
	token_list = token_list->next;
	if(token_list->t_type != t_ident)
	{
		fclose(target);
		printf("expected symbol found %s \n",token_list->value);
		exit(0);
	}
	strcpy(sym,token_list->value);
	fprintf(target,"%s ",sym);
	token_list = token_list->next;
	if((token_list->t_type != t_less) && (token_list->t_type != t_leq) && (token_list->t_type != t_eql) && (token_list->t_type != t_geq)&&(token_list->t_type != t_gt)&&(token_list->t_type != t_neq))
	{
		fclose(target);
		printf("expected a logical expr but found %s type %s\n ",token_list->value,tok_typ[token_list->t_type]);
		exit(0);
	}
	if(token_list->t_type == t_neq)
	{
		fprintf(target,"%s ","!= ");
	}
	else if(token_list->t_type == t_eql)
	{
		fprintf(target,"%s ","==");
	}
	else
	{
		fprintf(target,"%s ",token_list->value);
	}
	token_list = token_list->next;
	if(token_list->t_type != t_number)
	{
		fclose(target);
		printf("expected a numeric value but found %s\n",token_list->value);
		exit(0);
	}
	fprintf(target,"%s )\n{\n",token_list->value);
	// search for then token
	token_list = token_list->next;
	if(token_list->t_type != t_keyword)
	{
		fclose(target);
		printf("Expected \'then\' found %s\n",token_list->value);
		exit(0);
	}
	if(strcasecmp(token_list->value,"then")!=0)
	{
		fclose(target);
		printf("expected \'then\' \n");
		exit(0);
	}
	token_list = token_list->next;
	// process expression
	// currently just copy the rest
	while(token_list->t_type != t_eol)
	{
		fprintf(target,"%s ",token_list->value);
		token_list = token_list->next;
	}
	fprintf(target,"%s",";\n}\n");
}
void do_for(FILE *target, token *fortk)
{
	char sym[20]="";	
	fprintf(target,"for( int ");
	token_list = token_list->next;	
	if(token_list->t_type != t_ident)
	{
		fclose(target);
		printf("expected symbol found %s \n",token_list->value);
		exit(0);
	}
	strcpy(sym,token_list->value);
	fprintf(target,"%s ",sym);
	token_list = token_list->next;
	
	if(token_list->t_type != t_eql)
	{
		fclose(target);
		printf("expected punctuation ");
		exit(0);
	}
	if( strcmp(token_list->value,"=")!= 0)
	{
		fclose(target);
		printf("= \n");
		exit(0);
	}
	fprintf(target,"= ");
	token_list = token_list->next;
	
	if(token_list->t_type != t_number)
	{
		fclose(target);
		printf("expected integer \n");
		exit(0);
	}
	fprintf(target,"%s; ",token_list->value);
	token_list = token_list->next;
	
	if(strcasecmp(token_list->value,"to")!=0)
	{
		fclose(target);
		printf("expected to \n");
		exit(0);
	}
	token_list = token_list->next;
	
	fprintf(target,"%s <= %s; ",sym,token_list->value);
	if(strcasecmp(token_list->next->value,"step")==0)
	{
		token_list = token_list->next;
		
		token_list = token_list->next;
		
		fprintf(target,"%s += %s)\n{\n",sym,token_list->value);
	}
	else
	{
		fprintf(target,"%s++)\n{\n",sym);
	}
	
	// till now we completed the for( id = init; id cond final; change id value)
	// now we will search for keyword token "NEXT"
	
}

void do_next(FILE *target, token *ntk)
{
	fprintf(target,"\n}\n"); //close the for loop
	token_list=token_list->next;
}
void do_eol(FILE *target,token *ltk)
{
	fprintf(target,";\n");
	token_list = token_list->next;
}
void do_print(FILE *target, token *ptk)
{
	char fmtstr[100]="";
	char prmstr[100]="";
	fprintf(target,"printf(\"");
	token_list = token_list->next;
	while(true)
	{
		if(token_list->t_type == t_literal)
		{
			strcat(fmtstr,"%s ");
			strcat(prmstr,"\"");
			strcat(prmstr,token_list->value);
			strcat(prmstr,"\"");
		}

		if(token_list->t_type == t_ident)
		{
			strcat(fmtstr,"%f ");
			strcat(prmstr,token_list->value);
			strcat(prmstr,"*1.0");
		}
		token_list = token_list->next;
		if(strcasecmp(token_list->value,",")==0)
			strcat(prmstr,",");
		if(token_list->t_type == t_eol)
			break;
	}
	fprintf(target,"%s\",%s",fmtstr,prmstr);
	fprintf(target,");");
	fprintf(target,"printf(\"\\n\");\n");
}

/*void do_sub(FILE *target, token *subtk)
{
	char subdcl[256]="";
	strcpy(subdcl,"void ");
	subtk = subtk->next;
	if(subtk->t_type != t_ident)
	{
		printf("Error: expected subroutine name, found %s\n",subtk->value);
		close(target);
		exit(-1);
	}
	strcat(subdcl,subtk->value);
	subtk = subtk->next; // this must be a punctuation "("
	if(subtk->t_type != t_punctuation)
	{
		printf("Error: expected (  found %s\n",subtk->value);
		close(target);
		exit(-1);
	}
	strcat(subdcl,"(");
	subtk = subtk->next;
	// get the parameter list
	// here  there are 2 possibilities either we encounter a punctuation ')' or we encounter several parameters and punctuations
	// till we get ')' we should traverse the list
	char paramstr[100]="";
	while(	subtk->t_type !=t_eol)
	{
		if(strcasecmp(subtk->next->value,"integer")==0)
			strcat(paramstr,"int ");
		else if(strcasecmp(subtk->next->value,"byte")==0)
			strcat(paramstr,"unsigned char ");
		else if(strcasecmp(subtk->next->value,"string")==0)
			strcat(paramstr,"char* ");
		else
			strcat(paramstr,subtk->next->value);
	}
	
}*/
void generate(FILE *target)
{
	do_header(target);
	token *list;
	list = token_list;
	type_of_token type=0;
	int lin_cnt = 1;
	while(token_list)
	{
		type = token_list->t_type;
		printf("processing \" %s \"token\n",token_list->value);
		if(type == t_keyword)
		{
			if(strcasecmp(token_list->value, "REM")==0)
			{
				// remark comment
				do_rem(target,token_list);
			}
			if(strcasecmp(token_list->value, "for")==0)
			{
				do_for(target,token_list);
			}
			if(strcasecmp(token_list->value, "print")==0)
			{
				do_print(target,token_list);
			}
			if(strcasecmp(token_list->value, "next")==0)
			{
				do_next(target,token_list);
			}
			if(strcasecmp(token_list->value, "dim")== 0)
			{
				do_dim(target,token_list);
			}
			if(strcasecmp(token_list->value, "sub")==0)
			{
//				do_sub(subf,token_list);
			}
			if(strcasecmp(token_list->value, "if")==0)
			{
				do_if(target,token_list);
			}
		}
		if(type == t_eol)
		{
			//do_eol(target,list);
			lin_cnt++;
			//token_list = token_list->next;
		}
		token_list = token_list->next;
	}
	fprintf(target,"}");
	close(target);
	//printf(">>>> Number of lines Processed = %d\n",lin_cnt-1);
}
	
// cradel function to test our parser
int main(int argc, char *argv[])
{
	if(argc<3)
	{
		printf("Usage: %s <infile[.bas]> <outfile[.c]>\n",argv[0]);
		exit(0);
	}
	
	
	source = fopen(argv[1],"rw"); // open as readonly
	target = fopen(argv[2],"w+"); // open for write only
	//subf = fopen("subrtn.c","w+"); // open for write the subroutines and functions
	if(source == NULL)
	{
		printf("Unable to open input file: [%s]\n",argv[1]);
		exit(-1);
	}
	scanner(source);
	display_list();
	fprintf(target,"/*File %s converted with %s from %s dated %s*/\n",argv[2],argv[0],argv[1],__DATE__);
	generate(target);
	return 0;
}

