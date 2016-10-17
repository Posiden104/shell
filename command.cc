
/*
 * CS252: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <regex.h>
#include <dirent.h>
#include <pwd.h>
#include <algorithm.h>
#include <vector.h>
#include <string.h>

#include "command.h"

extern "C" char * get_command();

SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numOfAvailableArguments = 5;
	_numOfArguments = 0;
	_arguments = (char **) malloc( _numOfAvailableArguments * sizeof( char * ) );
}

int StringCompare(const void*a, const void*b){
	char const *char_a = *(char const**)a;
	char const *char_b = *(char const**)b;
	return strcmp(char_a, char_b);
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numOfAvailableArguments == _numOfArguments  + 1 ) {
		// Double the available space
		_numOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numOfAvailableArguments * sizeof( char * ) );
	}
	
	const char * buffer = "^.*${[^}][^}]*}.*$";
	regex_t re;
	if(regcomp(&re, buffer, 0)){
		perror("REGCOMP\n");
		return;
	}

	if(!regexec(&re, argument, (size_t)0, NULL, 0)){
		char * longArg = (char *)calloc(1, 1024*sizeof(char*));

		int i = 0;
		int m = 0;
		while(argument[i] != 0 && i < 1024){
			if(argument[i] != '$'){
				longArg[m] = argument[i];
				longArg[m+1] = '\0';
				i++;
				m++;
			} else {
				char * start = strchr((char*)(argument+i),'{');
				char * end = strchr((char*)(argument+i), '}');

				char * v = (char*)calloc(1, strlen(argument) * sizeof(char));
				strncat(v, start+1, end-start-1);
				char * value = getenv(v);
				if(value == NULL){
					strcat(longArg, "");
				} else {
					strcat(longArg, value);
				}
				i += strlen(v) + 3;
				if(value != NULL){
					m += strlen(value);
				}
				free(v);
			}
		}
		argument = strdup(longArg);
	}

	if(argument[0] == '~'){
		argument = expandTilde(arguement);
	}

	_arguments[ _numOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numOfArguments + 1] = NULL;
	
	_numOfArguments++;
}

void
SimpleCommand::expandWildcardsIfNessessary(char* prefix, char* suffix){
	int flag = 0;
	std::vector<std::string> arguments;

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
			expandWildcardIfNessessary(newPrefix, suffix);
			return;
		} else {
			if (prefix == NULL || prefix[0] == 0) {
				sprintf(newPrefix, "/%s", component);
			} else {
				sprintf(newPrefix, "/%s/%s", prefix, component);
			}
			expandWildcardIfNessessary(newPrefix, suffix);
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
					expandWildcardIfNessessary(newPrefix, suffix);
				}
			} else {
				expandWildcardIfNessessary(newPrefix, suffix);
			}
		}
	}
	closedir(d);

}

char*
SimpleCommand::expandTilde(char* arguement){
	int f = 0;
	char * st = NULL;
	struct passwd * pw = NULL;
	if(argument[1] == '/' || argument [1] == '\0'){
		pw = getpwuid(getuid());
	} else {
		st = strchr((char*)(argument+1), '/');
		if(st != NULL){
			char * t = NULL;
			strncpy(t, (char*)(argument+1), st - argument);
			pw = getpwnam(t);
		} else {
			pw = getpwnam((char*)(argument+1));
			f = 1;
		}
	}
		const char* homedir = pw->pw_dir;
	// free space for exp ~ and orig arg
	char* newArg = (char*)malloc((strlen(homedir)+ strlen(argument))*sizeof(char));
	newArg[0] = '\0';
	strcat(newArg, homedir);
	strcat(newArg, "/");
	if(f == 0){
		strcat(newArg, (char*)(argument + 1));
	}
	argument = newArg;
}

Command::Command()
{
	// Create available space for one simple command
	_numOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_outAppend = 0;
	_errAppend = 0;
	_background = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numOfAvailableSimpleCommands == _numOfSimpleCommands ) {
		_numOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numOfSimpleCommands ] = simpleCommand;
	_numOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _inFile ) {
		free( _inFile );
	}

	if( _outFile == _errFile) {
		free( _outFile);	
	} else {

		if ( _outFile ) {
			free( _outFile );
		}
	
		if ( _errFile ) {
			free( _errFile );
		}
	}

	_numOfSimpleCommands = 0;
	_outFile = 0;
	_inFile = 0;
	_errFile = 0;
	_background = 0;
	_outAppend = 0;
	_errAppend = 0;

	return;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
		printf("\n");
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default", 
		_inFile?_inFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void
Command::execute()
{
	// Don't do anything if there are no simple commands
	if ( _numOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	//print();

	// Setup i/o redirection
	
	// save default file descript 
	int defaultin = dup(0);
	int defaultout = dup(1);
	int defaulterr = dup(2);
	int inF;
	int outF;
	int errF;
	int fdpipe[2];

	// redirect i/o
	if(_inFile){
		inF = open(_inFile, O_RDONLY);
		if(inF < 0){
			perror("In file");
			return;
		}
	} else {
		inF = dup(defaultin);
	}
	if(_outFile){
		//
	}
	if(_errFile){
		if(_errAppend == 0){
			errF = open(_errFile, O_CREAT | O_WRONLY, 0666);
		} else {
			errF = open(_errFile, O_CREAT | O_APPEND | O_WRONLY, 0666);
		}
		if(errF < 0){
			perror("Error File");
			return;
		}
	} else {
		errF = dup(defaulterr);
	}

	int pid = 0;
	int status;

	SimpleCommand *curSimCmd;
	for(int i = 0; i < _numOfSimpleCommands; i++){

		dup2(inF, 0);
		close(inF);
		dup2(errF, 2);
		close(errF);

		if(i == _numOfSimpleCommands - 1) {
			if(_outFile){
				if(_outAppend == 0){
					outF = open(_outFile, O_CREAT | O_WRONLY, 0666);
				} else {
					outF = open(_outFile, O_CREAT | O_APPEND | O_WRONLY, 0666);
				}

				if(outF < 0){
					perror("Out File");
					return;
				}
			} else {
				outF = dup(defaultout);
			}
		} else {
			pipe(fdpipe);
			outF = fdpipe[1];
			inF = fdpipe[0]; 
		}

		dup2(outF, 1);
		close(outF);

		if(!strcmp(_simpleCommands[i]->_arguments[0], "setenv")){
			// set environment vars
			setenv(_simpleCommands[i]->_arguments[1], _simpleCommands[i]->_arguments[2], 1);
		} else if(!strcmp(_simpleCommands[i]->_arguments[0], "unsetenv")){
			// unset environment vars
			unsetenv(_simpleCommands[i]->_arguments[1]);
		} else if(!strcmp(_simpleCommands[i]->_arguments[0], "cd")){
			int ret;
			if(_simpleCommands[i]->_arguments[1] != NULL){
				ret = chdir(_simpleCommands[i]->_arguments[1]);
			} else {
				ret = chdir(getenv("HOME"));
			}
			if(ret != 0){
				fprintf(stderr, "No such file or directory\n");
			}
		} else {
			// for every simple command, fork new process
			pid = fork();
	
			// quit on fork error
			if(pid == -1) {
				perror( "ERROR: fork");
				exit(2);
			}
		
			if(pid == 0){
				// child
				// grab next command
				curSimCmd = _simpleCommands[i];
				
				// Execute command and args
				execvp(curSimCmd->_arguments[0], curSimCmd->_arguments);
				perror("Error: execvp");
				exit(2);
			} else {
				// parent
				// do parenty things
	
				// ground_child();
				// pay_bills();
				// gripe_about_money();
			}
		}
	}

	// restore in, out, err
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);

	// close file descriptors
	close(defaultin);
	close(defaultout);
	close(defaulterr);

	if(!_background){
		waitpid(pid, &status, 0);
		char str[10] = "";
		sprintf(str, "%d", WEXITSTATUS(status));
		setenv("?", str, 1);
		if(WEXITSTATUS(status) != 0){
			if(getenv("ON_ERROR") != NULL){
				printf(getenv("ON_ERROR"));
			}
		}
	}

	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	if(isatty(0)){
		const char* p = getenv("PROMPT");
		if(p != NULL){
			printf("%s> ", p);
		}
		else printf("myShell> ");
	}

	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

void handle_interrupt(int signal){
	Command::_currentCommand.clear();
	fprintf(stderr, "\n");
	Command::_currentCommand.prompt();
}

void handle_child(int signal){
	while(waitpid(-1, NULL, WNOHANG) > 0){
		//do nothing
	}
}

int yyparse(void);

main(int argc, char* argv[])
{

	setenv("SHELL", argv[0], 1);
	char pid[5] = "";
	snprintf(pid, 5, "%ld", (long)getpid());
	setenv("$", pid, 1);
	int err;
	struct sigaction sa_int;
	sa_int.sa_handler = handle_interrupt;
	sa_int.sa_flags = SA_RESTART;
	sigemptyset(&sa_int.sa_mask);
	err = sigaction(SIGINT,&sa_int, NULL);
	if(err == -1) {
		perror("sigint action");
		exit(1);
	}

	struct sigaction signalAction;
	signalAction.sa_handler = handle_child;
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_flags = SA_RESTART;
	err = sigaction(SIGCHLD, &signalAction, NULL);
	if(err == -1){
		perror("sigaction");
		exit(1);
	}

	int p = fork();
	if(p == 0){
		char** s;
		s = new char*[8];
		strcpy(*s, ".shellrc");
		execvp("source", s);
		//source(".shellrc");
	} else if(p < 0){
		perror("fork");
		exit(2);
	}

	Command::_currentCommand.prompt();
	yyparse();
}

