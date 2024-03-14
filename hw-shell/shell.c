#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include "tokenizer.h"

/* Convenience macro to silence compiler warnings about unused function parameters. */
#define unused __attribute__((unused))

/* Whether the shell is connected to an actual terminal or not. */
bool shell_is_interactive;

/* File descriptor for the shell input */
int shell_terminal;
/* Terminal mode settings for the shell */
struct termios shell_tmodes;

/* Process group id for the shell */
pid_t shell_pgid;

int cmd_exit(struct tokens* tokens);
int cmd_help(struct tokens* tokens);
int cmd_pwd(struct tokens* tokens);
int cmd_cd(struct tokens* tokens);

/* Built-in command functions take token array (see parse.h) and return int */
typedef int cmd_fun_t(struct tokens* tokens);

/* Built-in command struct and lookup table */
typedef struct fun_desc {
  cmd_fun_t* fun;
  char* cmd;
  char* doc;
} fun_desc_t;

fun_desc_t cmd_table[] = {{cmd_help, "?", "show this help menu"},
                          {cmd_exit, "exit", "exit the command shell"},
                          {cmd_pwd, "pwd", "print working directory"},
                          {cmd_cd, "cd", "change directory"}};

/* Prints a helpful description for the given command */
int cmd_help(unused struct tokens* tokens) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    printf("%s - %s\n", cmd_table[i].cmd, cmd_table[i].doc);
  return 1;
}

/* Exits this shell */
int cmd_exit(unused struct tokens* tokens) { exit(0); }

/* Print Working Directory of the Shell */
int cmd_pwd(unused struct tokens* tokens) {
  char buff[1024];
  getcwd(buff, 1024);
  printf("%s\n", buff);
  return 1;
}

/* Change the directory */
int cmd_cd(struct tokens* tokens) {
  if (tokens_get_length(tokens) < 2) {
    return -1;
  }
  if (chdir(tokens_get_token(tokens, 1)) == -1) {
    return -1;
  }
  return 0;
}
/* Looks up the built-in command, if it exists. */
int lookup(char cmd[]) {
  for (unsigned int i = 0; i < sizeof(cmd_table) / sizeof(fun_desc_t); i++)
    if (cmd && (strcmp(cmd_table[i].cmd, cmd) == 0))
      return i;
  return -1;
}

/* Intialization procedures for this shell */
void init_shell() {
  /* Our shell is connected to standard input. */
  shell_terminal = STDIN_FILENO;

  /* Check if we are running interactively */
  shell_is_interactive = isatty(shell_terminal);

  if (shell_is_interactive) {
    /* If the shell is not currently in the foreground, we must pause the shell until it becomes a
     * foreground process. We use SIGTTIN to pause the shell. When the shell gets moved to the
     * foreground, we'll receive a SIGCONT. */
    while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp()))
      kill(-shell_pgid, SIGTTIN);

    /* Saves the shell's process id */
    shell_pgid = getpid();

    /* Take control of the terminal */
    tcsetpgrp(shell_terminal, shell_pgid);

    /* Save the current termios to a variable, so it can be restored later. */
    tcgetattr(shell_terminal, &shell_tmodes);
  }
}

char* find_binary(char* binary_name) {
  if (binary_name[0] == '/') {
    return binary_name;
  }
  char* path_env = getenv("PATH");
  if (path_env == NULL) {
    fprintf(stderr, "PATH environment variable not set.\n");
    return NULL;
  }

  char* path_copy = strdup(path_env);
  if (path_copy == NULL) {
    fprintf(stderr, "Memory allocation error.\n");
    return NULL;
  }

  char* dir = strtok(path_copy, ":");
  while (dir != NULL) {
    char full_path[1024];
    snprintf(full_path, sizeof(full_path), "%s/%s", dir, binary_name);

    if (access(full_path, X_OK) == 0) {
      free(path_copy);
      return strdup(full_path);
    }

    dir = strtok(NULL, ":");
  }

  free(path_copy);
  return NULL;
}
/**
 * Redirects stdin from a file.
 * 
 * @param fileName the file to redirect from
 */
void redirectIn(char *fileName)
{
    int in = open(fileName, O_RDONLY);
    dup2(in, STDIN_FILENO);
    close(in);
}

/**
 * Redirects stdout to a file.
 * 
 * @param fileName the file to redirect to
 */
void redirectOut(char *fileName)
{
    int out = open(fileName, O_WRONLY | O_TRUNC | O_CREAT, 0644);
    dup2(out, STDOUT_FILENO);
    close(out);
}
void set_default_signals(){
  struct sigaction sa;
  sa.sa_handler = SIG_DFL;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  for(int signo = 1; signo < NSIG; signo++)
    sigaction(signo, &sa, NULL);

  return;
}

void ignore_signals(){
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  sigemptyset(&sa.sa_mask);

  for(int signo = 1; signo < NSIG; signo++)
    sigaction(signo, &sa, NULL);

  return;
}
void ctrl_d_handler(){
  return;
}
void run(char *cmd_path, char *args[]){
  pid_t pid = fork();
	if (pid < 0) { 
		fprintf(stderr, "Fork Failed");
	}else if ( pid == 0) { /* child process */
    
    if(setpgid(pid, pid) == -1){
      perror("SetPgid Error");
      exit(-1);
    }
    set_default_signals();
		execv(cmd_path, args);
	}else { /* parent process */
    redirectOut("/dev/tty");
    signal(EOF, ctrl_d_handler);
		waitpid(pid, NULL, 0);
	}
}

void createPipe(char* cmd_path, char *args[]){
	int fd[2];
  pipe(fd);
  dup2(fd[1], STDOUT_FILENO);
  close(fd[1]);

  run(cmd_path, args);

  dup2(fd[0], STDIN_FILENO);
  close(fd[0]);
}


int main(unused int argc, unused char* argv[]) {
  init_shell();
  static char line[4096];
  int line_num = 0;

  /* Please only print shell prompts when standard input is not a tty */
  if (shell_is_interactive)
    fprintf(stdout, "%d: ", line_num);
  
  ignore_signals();
  while (fgets(line, 4096, stdin)) {
    /* Split our line into words. */
    
    struct tokens* tokens = tokenize(line);

    /* Find which built-in function to run. */
    int fundex = lookup(tokens_get_token(tokens, 0));

    if (fundex >= 0) {
      cmd_table[fundex].fun(tokens);
    } else {
		
      int n = tokens_get_length(tokens);
      int i = 0;
      
      while (i < n) {
        bool OUTPUT_REDIRECT = false, INPUT_REDIRECT = false, PIPED = false;
        int j = 0;
        char *cmd_path = find_binary(tokens_get_token(tokens, i));
        char *args[1024];

        while (i < n) {
          if (strcmp(tokens_get_token(tokens, i), "<") == 0) {
            INPUT_REDIRECT = true;
            break;
          }
          if (strcmp(tokens_get_token(tokens, i), ">") == 0) {
            OUTPUT_REDIRECT = true;
            break;
          }
          if (strcmp(tokens_get_token(tokens, i), "|") == 0) {
            PIPED = true;
            break;
          }

          args[j++] = tokens_get_token(tokens, i);
          i++;
        }
        args[j++] = NULL; // end of the args string

        if (OUTPUT_REDIRECT) {
          redirectOut(tokens_get_token(tokens, i+1));
          i++;
        } else if (INPUT_REDIRECT) {
          redirectIn(tokens_get_token(tokens, i+1));
          i++;
        }else if (PIPED) {
          createPipe(cmd_path, args);
        }

        if(!PIPED)
          run(cmd_path, args);
        i++;
      }
		
	}
    
    if (shell_is_interactive)
      /* Please only print shell prompts when standard input is not a tty */
      fprintf(stdout, "%d: ", ++line_num);
    /* Clean up memory */
    tokens_destroy(tokens);

    /* Set the file descriptors to default mode */
    redirectIn("/dev/tty");
	  redirectOut("/dev/tty");
  }

  return 0;
}
