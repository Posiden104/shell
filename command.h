
#ifndef command_h
#define command_h

// Command Data Structure
struct SimpleCommand {
	// Available space for arguments currently preallocated
	int _numOfAvailableArguments;

	// Number of arguments
	int _numOfArguments;
	char ** _arguments;
	std::vector<std::string> arguments;
	int flag;

	SimpleCommand();
	void insertArgument( char * argument );
	void expandWildcardsIfNessessary(char* prefix, char* suffix); // , std::vector<std::string> arguements);
	char* expandTilde(char* argument);
};

struct Command {
	int _numOfAvailableSimpleCommands;
	int _numOfSimpleCommands;
	SimpleCommand ** _simpleCommands;
	char * _outFile;
	char * _inFile;
	char * _errFile;
	int _outAppend;
	int  _errAppend;
	int _background;

	void prompt();
	void print();
	void execute();
	void clear();
	
	Command();
	void insertSimpleCommand( SimpleCommand * simpleCommand );
	void killzombie();

	static Command _currentCommand;
	static SimpleCommand *_currentSimpleCommand;
};

#endif
