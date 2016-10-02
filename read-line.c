/*
 * CS354: Operating Systems. 
 * Purdue University
 * Example that shows how to read one line with simple editing
 * using raw terminal.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MAX_BUFFER_LINE 2048

// Buffer where line is stored
int line_length;;
char line_buffer[MAX_BUFFER_LINE];
int linePos;

// Simple history array
// This history does not change. 
// Yours have to be updated.
int history_index = 0;
char ** history = NULL;
int historyMax = 2;
int history_length = 0;

void read_line_print_usage()
{
  char * usage = "\n"
    " ctrl-?       Print usage\n"
    " Backspace    Deletes last character\n"
    " up arrow     See last command in the history\n";

  write(1, usage, strlen(usage));
}

void add_history(const char * input){
	char * line = strdup(input);
	if(history_length == historyMax +1){
		historyMax *= 2;
		history = (char**) realloc(history, historyMax * sizeof(char*));
	}

	line[strlen(line)] = '\0';
	history[history_length] = line;
	history_length++;
	history_index++;
}

void initHistory(){
	history = (char**)malloc(historyMax * sizeof(char*));
	const char * empty = "";
	add_history(empty);
}

char * get_command(){
	return history[history_index - 1];
}

void backspace(int n){
	int i;
	char ch = 8;
	for(i=0; i<n; i++){
		write(1, &ch, 1);
	}
}

void right(){
	
}

void left(){
	
}

/* 
 * Input a line with some basic editing.
 */
char * read_line() {

  // Set terminal in raw mode
  tty_raw_mode();

  line_length = 0;
  linePos = 0;

  // Read one line until enter is typed
  while (1) {
	// init history
	if(history == NULL)
		initHistory();

    // Read one character in raw mode.
    char ch;
    read(0, &ch, 1);

    if (ch>=32 && ch < 127) {
      // It is a printable character. 
	  if(linePos == line_length){
		 // Do echo
		  write(1,&ch,1);
	
	      // If max number of character reached return.
	      if (line_length==MAX_BUFFER_LINE-2) break; 
	
	      // add char to buffer.
	      line_buffer[line_length]=ch;
	      line_length++;
		  linePos ++;
	  } else {
			char * tmp = (char*)malloc(MAX_BUFFER_LINE*sizeof(char));
			int i;
			for(i = 0; i < MAX_BUFFER_LINE; i++){
				if(line_buffer[line_pos + i] == '\0'){
					break;
				}
				tmp[i] = line_buffer[linePos + i];
			}
			write(1, &ch, 1);
	
			if(line_length ==MAX_BUFFER_LINE -2) break;
	
			line_buffer[linePos] = ch;
			line_length ++;
			linePos++;
		
			int charsAdded = 0;
			for(int i=0; i < MAX_BUFFER_LINE; i++){
				charsAdded += 1;
				write(1, &tmp[i], 1);
				if(line_buffer[i] == '\0'){
					break;
				}
			}
			backspace(charsAdded);
			free(tmp);
		}
	}else if (ch==10) {
      // <Enter> was typed. Return line
      
	  if(strlen(line_buffer) != 0){
		add_history(line_buffer);
	  }

      // Print newline
      write(1,&ch,1);

      break;
    }
    else if (ch == 31) {
      // ctrl-?
      read_line_print_usage();
      line_buffer[0]=0;
      break;
    }
    else if (ch == 8) {
      // <backspace> was typed. Remove previous character read.

      // Go back one character
      ch = 8;
      write(1,&ch,1);

      // Write a space to erase the last character read
      ch = ' ';
      write(1,&ch,1);

      // Go back one character
      ch = 8;
      write(1,&ch,1);

      // Remove one character from buffer
      line_length--;
	  linePos--;
    }
    else if (ch==27) {
      // Escape sequence. Read two chars more
      //
      // HINT: Use the program "keyboard-example" to
      // see the ascii code for the different chars typed.
      //
      char ch1; 
      char ch2;
      read(0, &ch1, 1);
      read(0, &ch2, 1);
      if (ch1==91 && ch2==65) {
		// Up arrow. Print next line in history.
	
		// Erase old line
		// Print backspaces
		int i = 0;
		for (i =0; i < line_length; i++) {
		  ch = 8;
		  write(1,&ch,1);
		}
	
			// Print spaces on top
		for (i =0; i < line_length; i++) {
		  ch = ' ';
		  write(1,&ch,1);
		}
	
		// Print backspaces
		for (i =0; i < line_length; i++) {
		  ch = 8;
		  write(1,&ch,1);
		}	
	
		// Copy line from history
		strcpy(line_buffer, history[history_index]);
		line_length = strlen(line_buffer);
		history_index=(history_index+1)%history_length;
	
		// echo line
		write(1, line_buffer, line_length);
      } else if(ch1==91 && ch2==66){
		//Down Arrow
		backspace(line_length);
		int i = 0;
		for(i = 0; i < line_length; i++){
			ch = ' ';
			write(1, &ch, 1);
		}
		backspace(line_length);
		if(history_index <= 0){
			history_index = 0;
		} else {
			history_index += 1;
			if(history_index >= history_length){
				history_index = 0;
			}
		}
		if(history[history_index] == NULL){
			const char * not = "";
			strcpy(line_buffer, not);
		} else {
			strcpy(line_buffer, history[history_index]);
		}
		
		line_length = strlen(line_buffer);
		//linePos = strlen(line_buffer);

		write(1, line_buffer, line_length);
	  } else if(ch1 == 91 && ch2 == 68){
		// Left
		right();
	  } else if(ch1 == 91 && ch2 == 67){
		// right
		right();
	  } else if(ch1 == 91 && ch2 == 70){
		// end
		while(linePos < line_length){
			// right
			right();
		}
	  } else if(ch1 == 91 && ch2 == 72){
		// home
		while(linePos > 0){
			// left
			left();
		}
	  }


      
    }

  }

  // Add eol and null char at the end of string
  line_buffer[line_length]=10;
  line_length++;
  line_buffer[line_length]=0;

  return line_buffer;
}

