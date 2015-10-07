/****************************/
/* CS 481                   */
/* Programming Assignment 4 */
/* 7 October 2015           */
/* Authors:                 */
/*    Paolo Macias          */
/*    Erin Sosebee          */
/****************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/resource.h>

#define MINI437_RL_BUFSIZE 80
#define MINI437_TOK_BUFSIZE 80
#define MINI437_TOK_DELIM " \t\r\n\a"

char *history[10];
int histIt = 0; //history iterator
int background = 0; // flag for background processes

/* Function declarations for built-in shell commands */
int cd(char **args);
int help(char **args);
int last10();
int mini437_exit(char **args);

/* List of builtin commands, followed by their corresponding functions. */
char *builtin_str[] = {
  "cd",
  "help",
  "last10",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &cd,
  &help,
  &last10,
  &mini437_exit
};

int mini437_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*******************/
/* Signal handler */
/*******************/

// Ctrl+C signal handler
void sigint_handler(int signal)
{
  printf("\n");
  last10();
}

/************************************/
/* Bult-in function implementations */
/************************************/

// Change Directory 
int cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "mini437: expected argument to \"cd\"\n");
  } 
  else {
    if (chdir(args[1]) != 0) {
      perror("mini437");
    }
  }
  return 1;
}


// Help
int help(char **args)
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

// Shows the last 10 nonempty entered commands
int last10()
{
  int i;
  for (i = 0; i < histIt; i++) {
    printf("%4d %s", (i+1), history[i]);
    printf("\n");
  }
  return 1;
}

//exits
int mini437_exit(char **args)
{
  // TODO: Kill all background processes. 
  int i;
  for(i=0;i<histIt;i++){
    free(history[i]);
  }

  return 0;
}

/*********/
/* Shell */
/*********/

//Launch a program and wait for it to terminate.
int mini437_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  struct rusage usage;
  struct timeval startUsrTime, endUsrTime;
  struct timeval startSysTime, endSysTime;

  // Start timing process
  getrusage(RUSAGE_SELF, &usage);
  startUsrTime = usage.ru_utime;
  startSysTime = usage.ru_stime;

  // Print pre-run command line information
  printf("PreRun: %s ", args[0]);
  int i = 1;
  while (args[i] != NULL) {
    printf("%d:%s, ", i, args[i]);
    i++;
  }
  printf("\n");
  
  // pid = waitpid(-1, &status, WNOHANG);
  // printf("pid: %d\n", pid);
  // if (pid > 0) {
  //   printf("waitpid reaped child pid %d\n", pid);
  // }

  // Begin executing process
  pid = fork();
  if (pid < 0) {
    perror("mini437sh");
    exit(EXIT_FAILURE);
  } 
  else if (pid == 0) {
    execvp(args[0], args);
    exit(EXIT_FAILURE);
  }
  else if (!background) {
    pid = waitpid(pid, &status, 0);
    if (pid > 0) {
      getrusage(RUSAGE_SELF, &usage);
      endUsrTime = usage.ru_utime;
      endSysTime = usage.ru_stime;

      float userTime = ((endUsrTime.tv_usec - startUsrTime.tv_usec));
      float sysTime = ((endSysTime.tv_usec - startSysTime.tv_usec));

      printf("PostRun(PID:%d): %s -- user time %2.4f system time %2.4f\n",
        pid, args[0], 
        userTime, 
        sysTime);
    }
  }
/*
  // Begin executing process
  pid = fork();
  if (pid == 0) { // Child process
    if (execvp(args[0], args) == -1) {
      perror("mini437");
    }
    exit(EXIT_FAILURE);
  } 
  else if (pid < 0) { // Error forking
    perror("mini437");
  } 
  else { // Parent process
    
      if (background) { 
        // printf("in background\n");
        // signal(SIGINT, sigchld_handler);
        return 1;
      }
      else {
        // printf("in waitpid\n");
        do {
          wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
      } 
  }
  
  // End process timing
  getrusage(RUSAGE_SELF, &usage);
  endUsrTime = usage.ru_utime;
  endSysTime = usage.ru_stime;

  long int userTime = ((endUsrTime.tv_usec - startUsrTime.tv_usec)/1000);
  long int sysTime = ((endSysTime.tv_usec - startSysTime.tv_usec)/1000);

  printf("PostRun(PID:%d): %s -- user time %ld system time %ld\n",
    pid, args[0], 
    userTime, 
    sysTime);*/
  
  
  return 1;
}


//Execute shell built-in or launch program.
int mini437_execute(char **args)
{
  int i;

  // An empty command was entered.
  if (args[0] == NULL) return 1;

  for (i = 0; i < mini437_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return mini437_launch(args);
}


//Read a line of input from stdin.
char *mini437_read_line(void)
{
  char *line = NULL;
  ssize_t bufsize = 0; // have getline allocate a buffer for us
  getline(&line, &bufsize, stdin);

  // Check for '&'
  char *ampersand = strchr(line, '&');
  if (ampersand != NULL) {
    background = 1;

    // Subtract string address from character address to get index
    int andIdx = (int) (ampersand - line); 
    line[andIdx] = ' '; // Replace '&' with nothing so that command can run
  }
  else background = 0;

  // printf("background = %d\n", background);
  return line;
}



//Split a line into tokens (very naively).
char **mini437_split_line(char *line)
{
  int bufsize = MINI437_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "mini437: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MINI437_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += MINI437_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "mini437: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, MINI437_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}


//Loop getting input and executing it.
void mini437_loop(void)
{
  char *line;
  char **args;
  int shellStatus, status;
  pid_t pid;

  do {
    pid = waitpid(-1, &status, WNOHANG);
    // printf("pid: %d\n", pid);
    if (pid > 0) {
      printf("waitpid reaped child pid %d\n", pid); 
    }

    printf("mini437-PM-ES > ");
    line = mini437_read_line();
    args = mini437_split_line(line);
    shellStatus = mini437_execute(args);

    if (histIt < 10) {
      if (line != NULL && line[0] != '\n') {
        history[histIt] = malloc(80);
        strcpy(history[histIt], line);
        histIt = histIt + 1;
      }
    }
    else if(histIt >= 10) {
      if (line != NULL && line[0] != '\n') {
        int i;
        for(i=1;i<10;i++){
          strcpy(history[i-1], history[i]);
        }
        strcpy(history[9], line);
      }
    }

    free(line);
    free(args);

  } while (shellStatus);
}


int main(int argc, char **argv)
{
  // signal(SIGINT, last10); //Ctrl + c handler
  signal(SIGINT, sigint_handler);

  // Run command loop.
  mini437_loop();

  // Perform any shutdown/cleanup.
  return EXIT_SUCCESS;
}
