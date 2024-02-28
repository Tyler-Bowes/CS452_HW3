#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "Command.h"
#include "error.h"
#include "Tree.h"

typedef struct {
  char *file;
  char **argv;
  char* input_file;  // The file to redirect stdin from, or NULL if there is no input redirection
  char* output_file; // The file to redirect stdout to, or NULL if there is no output redirection
  // Command *next; // The next command in the pipeline
  int fd[2]; // The file descriptors for the pipe
} *CommandRep;

#define BIARGS CommandRep r, int *eof, Jobs jobs
#define BINAME(name) bi_##name
#define BIDEFN(name) static void BINAME(name) (BIARGS)
#define BIENTRY(name) {#name,BINAME(name)}

static char *owd=0;
static char *cwd=0;

static void builtin_args(CommandRep r, int n) {
  char **argv=r->argv;
  for (n++; *argv++; n--);
  if (n)
    ERROR("wrong number of arguments to builtin command"); // warn
}

BIDEFN(exit) {
  builtin_args(r,0);
  *eof=1;
}

BIDEFN(pwd) {
  builtin_args(r,0);
  if (!cwd)
    cwd=getcwd(0,0);
  printf("%s\n",cwd);
}

BIDEFN(cd) {
  builtin_args(r,1);
  if (strcmp(r->argv[1],"-")==0) {
    char *twd=cwd;
    cwd=owd;
    owd=twd;
  } else {
  if (owd) free(owd);
    owd=cwd;
    if (chdir(r->argv[1])) {
      ERROR("chdir() failed"); // warn
      return;
    }
    cwd = getcwd(NULL, 0); // get the current working directory
  }
  
}

//implement history as a builtin
BIDEFN(history) {
  builtin_args(r,0);
  //prints everything inside the .history file to standard output
  FILE *file = fopen(".history", "r");
  if (file == NULL) {
    ERROR("file does not exist");
  }
  char c;
  while ((c = fgetc(file)) != EOF) {
    printf("%c", c);
  }
  fclose(file);
}

static int builtin(BIARGS) {
  typedef struct {
    char *s;
    void (*f)(BIARGS);
  } Builtin;
  static const Builtin builtins[]={
    BIENTRY(exit),
    BIENTRY(pwd),
    BIENTRY(cd),
    BIENTRY(history),
    {0,0}
  };
  int i;
  for (i=0; builtins[i].s; i++)
    if (!strcmp(r->file,builtins[i].s)) {
      builtins[i].f(r,eof,jobs);
      return 1;
    }
  return 0;
}

static char **getargs(T_words words) {
  int n=0;
  T_words p=words;
  while (p) {
    p=p->words;
    n++;
  }
  char **argv=(char **)malloc(sizeof(char *)*(n+1));
  if (!argv)
    ERROR("malloc() failed");
  p=words;
  int i=0;
  while (p) {
    argv[i++]=strdup(p->word->s);
    p=p->words;
  }
  argv[i]=0;
  return argv;
}

//creates a new CommandRep struct and allocates memory for it
//copies the input and output file from the T_redirIO struct to the CommandRep struct
//returns the CommandRep struct
extern Command newCommand(T_words words, T_redirIO redir) { 
  CommandRep r=(CommandRep)malloc(sizeof(*r));
  if (!r)
    ERROR("malloc() failed");
  
  if(redir->input_file == NULL){
    r->input_file= NULL;
  }else{
    r->input_file = strdup(redir->input_file->s);
    // r->input_file = redir->input_file->s;
  }

  if(redir->output_file == NULL){
    r->output_file= NULL;
  }else{
  r->output_file = strdup(redir->output_file->s);
  // r->output_file = redir->output_file->s;
  }
  r->argv=getargs(words);
  r->file=r->argv[0];
  return r;
}

static void child(CommandRep r, int fg) {
  int eof=0;
  Jobs jobs=newJobs();
  if (builtin(r,&eof,jobs))
    return;
  execvp(r->argv[0],r->argv);
  ERROR("execvp() failed");
  exit(0);
}

//executes the command in the pipeline
//if the command is a builtin command, it will be executed, else it will be executed in a child process
extern void execCommand(Command command, Pipeline pipeline, Jobs jobs,
			int *jobbed, int *eof, int fg) {
  CommandRep r=command;
  if (fg && builtin(r,eof,jobs))
    return;
  if (!*jobbed) {
    *jobbed=1;
    addJobs(jobs,pipeline);
  }
  
  int pid=fork();
  if (pid==-1)
    ERROR("fork() failed");
  if (pid==0) {
      // dup2(r->fd[1], STDOUT_FILENO); // redirect stdout to the write end of the pipe
      if (r->input_file) {
        // Redirect stdin to the specified file
        freopen(r->input_file, "r", stdin);
      }
      if (r->output_file) {
        // Redirect stdout to the specified file
        freopen(r->output_file, "w", stdout);
      }
    child(r,fg);
  } 
  // else {
  //   close(pipefd[1]); // close the write end of the pipe in the parent
  //   dup2(pipefd[0], STDIN_FILENO); // redirect stdin to the read end of the pipe
  //   close(pipefd[0]); // close the read end of the pipe in the parent
  //   if (fg) {
  //     waitpid(pid, NULL, 0); // wait for the command to finish
  //   }
  // }
  else if (fg) {// if the command should be run in the foreground
    waitpid(pid, NULL, 0); // wait for the command to finish
  }
}

//frees the memory allocated for the CommandRep struct
extern void freeCommand(Command command) { 
  CommandRep r=command;
  char **argv=r->argv;
  while (*argv)
    free(*argv++);
  free(r->argv);
  free(r);
}

extern void freestateCommand() {
  if (cwd) free(cwd);
  if (owd) free(owd);
}
