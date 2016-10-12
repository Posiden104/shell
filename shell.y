
/*
 * CS-252
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN GREAT NEWLINE PIPE LESS DGREAT DLESS
%token  AMP GREATAMP DGREATAMP EXIT

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <dirent.h>
#include <vector>
#include <string> 
#include <algorithm>
#include "command.h"
void yyerror(const char * s);
int yylex();

int f = 0;
std::vector<std::string> args;

void expandWildcard(char* prefix, char*suffix){
	f = 0;
	if(suffix[0] == 0){
		args.push_back(strdup(prefix));
		return;
	}
	char * s = NULL;
	if(suffix[0] == '/'){
		s = strchr((char*) (suffix+1), '/');
		f = 1;
	} else {
		s = strchr(suffix, '/');
	}

	char component[1024] = "";
	char newPrefix[1024];

	if(s != NULL){
		if(suffix[0] == '/'){
			strncpy(component, ((char*)(suffix+1)), s-suffix-1);
			suffix = s + 1;
		} else {
			strncpy(component, suffix, s-suffix);
			suffix = s + 1;
		}
	} else if(suffix[0] == '/'){
		strcpy(component, ((char*)(suffix + 1)));
		suffix = suffix + strlen(suffix);
	} else {
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}
	//73
}

%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command
	;

command: simple_command
        ;

simple_command:	
	pipe_args iomodifier_list bg_opt NEWLINE {
		/*printf("   Yacc: Execute command\n");*/
		Command::_currentCommand.execute();
	}
	| NEWLINE {
		Command::_currentCommand.execute();
	}
	| error NEWLINE { 
		Command::_currentCommand.clear();
		yyerrok; 
		printf("\n");
		Command::_currentCommand.prompt();
	}
	;

command_and_args:
	command_word argument_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

argument_list:
	argument_list argument
	| /* can be empty */
	;

argument:
	WORD {
               /*printf("   Yacc: insert argument \"%s\"\n", $1);*/

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               /*printf("   Yacc: insert command \"%s\"\n", $1);*/
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	| EXIT {
			exit(2);
	}
	;

pipe_args:
	pipe_args PIPE command_and_args
	| command_and_args
	;

iomodifier_list:
	iomodifier_list iomodifier_opt
	| /* can be empty */
	;

iomodifier_opt:
	GREAT WORD {
		/*printf("   Yacc: insert output \"%s\"\n", $2);*/
		Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
		/*printf("   Yacc: insert input \"%s\"\n", $2);*/
		Command::_currentCommand._inFile = $2;
	}
	| GREATAMP WORD{
		/*printf("   Yacc: insert output \"%s\"\n", $2);
		printf("   Yacc: insert error \"%s\"\n", $2);*/
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = Command::_currentCommand._outFile;
	}
	| DGREAT WORD {
		/*printf("   Yacc: insert appended output \"%s\"\n", $2);*/
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._outAppend = 1;
	}
	| DGREATAMP WORD {
		/*printf("   Yacc: insert appended output \"%s\"\n", $2);
		printf("   Yacc: insert appended error \"%s\"\n", $2);*/
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = Command::_currentCommand._outFile;
		Command::_currentCommand._outAppend = 1;
		Command::_currentCommand._errAppend = 1;
	}
	;

bg_opt:
	AMP {
		/*printf("   Yacc: run in background\n");*/
		Command::_currentCommand._background = 1;
	}
	| /* can be empty*/
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
