#ifndef TREE_H
#define TREE_H

typedef struct T_sequence *T_sequence;
typedef struct T_pipeline *T_pipeline;
typedef struct T_command  *T_command;
typedef struct T_words    *T_words;
typedef struct T_word     *T_word;
typedef struct T_redirIO  *T_redirIO;


struct T_sequence {
  T_pipeline pipeline;
  char *op;			/* ; or & */
  T_sequence sequence;
};

struct T_pipeline {
  T_command command;
  T_pipeline pipeline;  //recusion ends when pipeline is null
};

// another pipeline struct to be able to call recusivly

struct T_redirIO {
  T_word input_file;
  T_word output_file;
};

struct T_command {
  T_words words;
  T_redirIO redir;
};

struct T_words {
  T_word word;
  T_words words;
};

struct T_word {
  char *s;
};


extern T_sequence new_sequence();
extern T_pipeline new_pipeline();
extern T_command  new_command();
extern T_words    new_words();
extern T_word     new_word();
extern T_redirIO    new_redir();

#endif
