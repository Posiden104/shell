
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
%token  AMP GREATAMP DGREATAMP

%union	{
		char   *string_val;
	}

%{
//#define yylex yylex
#include <stdio.h>
#include "command.h"
void yyerror(const char * s);
int yylex();

%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands pipe_opt command
	;

command: simple_command
        ;

simple_command:	
	command_and_args iomodifier_opt bg_opt NEWLINE {
		printf("   Yacc: Execute command\n");
		Command::_currentCommand.execute();
	}
	| NEWLINE 
	| error NEWLINE { yyerrok; }
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
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inFile = $2;
	}
	| GREATAMP WORD{
		printf("   Yacc: insert output \"%s\"\n", $2);
		printf("   Yacc: insert error \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = Command::_currentCommand._outFile;
	}
	| /* can be empty */ 
	;

pipe_opt:
	PIPE {
		printf("   Yacc: insert pipe\n");
	}
	;

bg_opt:
	AMP {
		printf("   Yacc: run in background\n");
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
