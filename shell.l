
/*
 *
 * CS-252 Fall 2013
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "y.tab.h"

extern "C" char * read_line();

int mygetc(FILE * f){
	static char *p;
	char ret;

	if(!isatty(0)){
		return getc(f);
	}

	// is tty
	if (p == NULL || *p == 0){
		char * s = read_line();
		p = s;
	}

	ret = *p;
	p++;
	return ret;
}

#undef getc
#define getc(f) mygetc(f)

static  void yyunput (int c,char *buf_ptr  );

void myunputc(int c) {
    unput(c);
}

%}

%%

\n 	{
		return NEWLINE;
	}

[ \t] 	{
		/* Discard spaces and tabs */
     	}	

">" 	{
		return GREAT;
	}
"<"		{
		return LESS;
	}
"|"		{
		return PIPE;
	}
"&"		{
		return AMP;
	}
">>"		{
		return DGREAT;
	}
"<<"		{
		return DLESS;
	}
">&"		{
		return GREATAMP;
	}
">>&"		{
		return DGREATAMP;
	}
"exit"		{
		return EXIT;
	}

[^ \t\n&<>\|]*\\[^\t\n]*	{
		char * buff = (char*)malloc(1024*sizeof(char));
		int i = 0;
		int j = 0;
		for(i = 0; i != strlen(yytext); i++){
			if(yytext[i] == 92){
				if(yytext[i+1] == 92){
					buff[j] = yytext[i+1];
					i++;
					j++;
				}
			} else {
				buff[j] = yytext[i];
				j++;
			}
		} 
		yylval.string_val = strdup(buff);
		return WORD;
}

[^ \t\n][^ \t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

\"[^\n\"]*\"	{
	char s[100];
	for(int i = 0; i < strlen(yytext)-2; i++){
		s[i] = yytext[i+1];
	}
	s[strlen(yytext-2)] = '\0';
	yylval.string_val = strdup(s);
	return WORD;
}

%%
