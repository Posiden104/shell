
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

int flag = 0;
std::vector<std::string> arguement;

void expandWildcard(char* prefix, char* suffix){
	flag = 0;
	if (suffix[0] == 0) {
		arguments.push_back(strdup(prefix));
		return;
	}
	char * s = NULL;
	if (suffix[0] == '/') {
		s = strchr((char*) (suffix+1), '/');
		flag = 1;
	} else {
		s = strchr(suffix, '/');
	}
	char component[1024] = "";
	char newPrefix[1024];

	if (s != NULL) {
		if (suffix[0] == '/') {
			strncpy(component,((char*)(suffix+1)), s-suffix-1);
			suffix = s + 1;
		} else {
			strncpy(component, suffix, s-suffix);
			suffix = s + 1;
		}
	} else if (suffix[0] == '/') {
		strcpy(component, ((char*) (suffix + 1)));
		suffix = suffix + strlen(suffix);
	} else {
		strcpy(component, suffix);
		suffix = suffix + strlen(suffix);
	}

	if (strchr(component, '*') == NULL && strchr(component, '?') == NULL) {
		if (flag == 0) {
			if (prefix == NULL || prefix[0] == 0) {
				sprintf(newPrefix, "%s", component);
			} else {
				sprintf(newPrefix, "%s/%s", prefix, component);
			}
			expandWildcard(newPrefix, suffix);
			return;
		} else {
			if (prefix == NULL || prefix[0] == 0) {
				sprintf(newPrefix, "/%s", component);
			} else {
				sprintf(newPrefix, "/%s/%s", prefix, component);
			}
			expandWildcard(newPrefix, suffix);
			return;
		}
	}
	char * reg = (char*)malloc(2*strlen(component)+10);
	char * a = component;
	char * r = reg;
	*r = '^'; r++;
	while (*a) {
		if (*a == '*') {
			*r = '.';
			r++;
			*r = '*';
			r++;
		} else if (*a == '?') {
			*r = '.';
			r++;
		} else if (*a == '.') {
			*r = '\\';
			r++;
			*r = '.';
			r++;
		} else {
			*r = *a;
			r++;
		}
		a++;
	}
	*r = '$';
	r++;
	*r = 0;


	regex_t re;
	int result = regcomp(&re, reg, REG_EXTENDED|REG_NOSUB);
	if (result != 0) {
		perror("compile\n");
		return;
	}

	struct dirent * ent;
	char * dir;
	if (flag) {
		const char * slash = "/";
		dir = strdup(slash);
	} else if (prefix == NULL) {
		const char * dot = ".";
		dir = strdup(dot);	
	} else { 
		dir = prefix;
	}
	DIR * d = opendir(dir);
	if (d==NULL) return;
	
	while ((ent = readdir(d)) != NULL) {
		if (regexec(&re, ent->d_name, (size_t) 0, NULL, 0) == 0) {
			if (flag == 0) {
				if (prefix == NULL || prefix[0] == 0) {
					sprintf(newPrefix, "%s", ent->d_name);
				} else {
					sprintf(newPrefix, "%s/%s", prefix, ent->d_name);
				}
			} else {
				if (prefix == NULL || prefix[0] == 0) {
					sprintf(newPrefix, "/%s", ent->d_name);
				} else {
					sprintf(newPrefix, "/%s/%s", prefix, ent->d_name);
				}
			}
			if (ent->d_name[0] == '.') {
				if (component[0] == '.') {
					expandWildcard(newPrefix, suffix);
				}
			} else {
				expandWildcard(newPrefix, suffix);
			}
		}
	}
	closedir(d);

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
			if(strchr($1, '*') == NULL && strchr($1, '?') == NULL) {
				Command::_currentSimpleCommand->insertArgument( $1 );
			} else {
				expandWildcard(NULL, $1);
			}
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
