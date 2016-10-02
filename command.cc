
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include "command.h"

extern "C" char * get_command();

SimpleCommand::SimpleCommand()
{
	// Create available space for 5 arguments
	_numOfAvailableArguments = 5;
	_numOfArguments = 0;
	_arguments = (char **) malloc( _numOfAvailableArguments * sizeof( char * ) );
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
	
	_arguments[ _numOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numOfArguments + 1] = NULL;
	
	_numOfArguments++;
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
			inF = dup(defaultout); 
		}

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

	// restore in, out, err
	dup2(defaultin, 0);
	dup2(defaultout, 1);
	dup2(defaulterr, 2);

	// close file descriptors
	close(defaultin);
	close(defaultout);
	close(defaulterr);

	if(!_background){
		waitpid(pid, 0, 0);
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
	printf("yass>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

main()
{
	Command::_currentCommand.prompt();
	yyparse();
}

