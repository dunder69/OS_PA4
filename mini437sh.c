/*
  Paolo Macias
  CS 481 PA4
  10/7/15
*/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define MINI437_RL_BUFSIZE 1024
#define MINI347_TOK_BUFSIZE 64
#define MINI347_TOK_DELIM " \t\r\n\a"

char *history[10];
int histIt = 0; //history iterator

/*
  Function Declarations for builtin shell commands:
 */
int mini437_cd(char **args);
int mini437_help(char **args);
int mini437_last10(char **args);
int mini437_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "last10",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &mini437_cd,
  &mini437_help,
  &mini437_last10,
  &mini437_exit
};

int mini437_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

/**
   @brief Bultin command: change directory.
   @param args List of args.  args[0] is "cd".  args[1] is the directory.
   @return Always returns 1, to continue executing.
 */
int mini437_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "mini437: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("mini437");
    }
  }
  return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args.  Not examined.
   @return Always returns 1, to continue executing.
 */
int mini437_help(char **args)
{
  int i;
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < mini437_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}


int mini437_last10(char **args)
{
  printf("history[0] = %s \n", history[0]);
  printf("history[1] = %s \n", history[1]);
  int i;
  for(i=0;i<10;i++){
    printf("%s", history[i]);
    printf("\n");
  }
}

/**
   @brief Builtin command: exit.
   @param args List of args.  Not examined.
   @return Always returns 0, to terminate execution.
 */
int mini437_exit(char **args)
{
  // TODO: Kill all background processes. 
  return 0;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1, to continue execution.
 */
int mini437_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("mini437");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("mini437");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate
 */
int mini437_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < mini437_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return mini437_launch(args);
}


/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *mini437_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);
  return line;
}


/**
   @brief Split a line into tokens (very naively).
   @param line The line.
   @return Null-terminated array of tokens.
 */
char **mini437_split_line(char *line)
{
  int bufsize = MINI347_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "mini437: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MINI347_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += MINI347_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "mini437: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, MINI347_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void mini437_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("mini437-PM: ");
    line = mini437_read_line();
    printf("line = %s \n", line);
    args = mini437_split_line(line);
    status = mini437_execute(args);

    if(histIt < 10){
      if(line != NULL){
        printf("in for loop, line = %s \n", line);
        history[histIt] = line;
        printf("in for loop, history[%d] = %s \n", histIt, history[histIt]);
        histIt = histIt + 1;
        printf("after +1, histIt = %d \n", histIt);
        printf("in for loop, history[0] = %s \n", history[0]);
      }
    }
    else if(histIt == 10){
      if(line != NULL){
        int i;
        for(i=1;i<10;i++){
          history[(histIt-1)] = history[histIt];
        }
        history[10] = line;
      }
    }

    free(line);
    free(args);
  } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code
 */
int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  mini437_loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
