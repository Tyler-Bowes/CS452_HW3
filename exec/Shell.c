/**
 * The shell program allows users to interact with the operating system by executing commands.
 * It provides a command prompt where users can enter commands, and then executes those commands
 * by creating child processes. The shell program also supports basic command line features such
 * as input/output redirection and piping.
 */
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "Jobs.h"
#include "Parser.h"
#include "Interpreter.h"
#include "error.h"

int main() {
  int eof=0;
  Jobs jobs=newJobs();
  char *prompt=0;

// checks whether the standard input (stdin) is associated with a terminal using the isatty() function. 
// The isatty() function returns a non-zero value if the file descriptor refers to a terminal, and 0 otherwise
  if (isatty(fileno(stdin))) {
    using_history();
    read_history(".history");
    prompt="$ ";
  } else {
    rl_bind_key('\t',rl_insert);
    rl_outstream=fopen("/dev/null","w");
  }
  
  while (!eof) { // enters loop to read commands from the user and execute them
    char *line=readline(prompt);
    if (!line) // is it null (error?)
      break;
    if (*line)
      add_history(line);
    Tree tree=parseTree(line); // parse the command line into a tree which is built by sequences of pipelines and commands
    free(line);
    interpretTree(tree,&eof,jobs); // interpret the tree and execute the commands
    freeTree(tree);
  }

  if (isatty(fileno(stdin))) {
    write_history(".history");
    rl_clear_history();
  } else {
    fclose(rl_outstream);
  }
  freestateCommand();
  return 0;
}
