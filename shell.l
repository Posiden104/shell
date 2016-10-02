
/*
 *
 * CS-252 Fall 2013
 * shell.l: lexical analyzer for shell
 * You have to extend it.
 *
 */

%{

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "y.tab.h"

extern "C" {char * read_line();}

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



[^ \t\n][^ \t\n]*  {
		/* Assume that file names have only alpha chars */
		yylval.string_val = strdup(yytext);
		return WORD;
	}

