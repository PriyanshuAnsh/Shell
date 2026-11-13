#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>

#define INIT_BUFF_SIZE 1024
#define SHELL_TOKEN_BUFF_SIZE 64
#define SHELL_TOKEN_DELIMITER " \t\r\n\a"


int cd(char ** args);
int help(char ** args);
int shell_exit(char ** args);

char *built_in_cmds[] = {
    "cd",
    "help",
    "exit"
};

int (*built_in_func[]) (char **) = {
    &cd,
    &help,
    &shell_exit
};
int num_built_ins() {
    return sizeof(built_in_cmds) / sizeof(char *);
}

int cd(char ** args) {
    if(args[1] == NULL) {
        fprintf(stderr, "shell: Unexpected Argument: %d\n", *args[1]);
    } else {
        if(chdir(args[1]) != 0) {
            perror("Error changing directory\n");
        }
    }
    return 1;
}

int help(char ** args) {
int i;
  printf("Priyanshu's Custom Shell Implementation\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < num_built_ins(); i++) {
    printf("  %s\n", built_in_cmds[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int shell_exit(char **args)
{
  return 0;
}

int _executeHelper(char ** args) {

    pid_t pid, wpid;
    int status;

    pid = fork();

    if(pid == 0) {
        //Child Process
        if(execvp(args[0], args) == -1) {
            perror("shell: Error executing command.");
        }
        exit(EXIT_FAILURE);
    } else if(pid < 0) {
        //Error
        perror("shell: Error Forking.");
    } else {
        //Parent
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while(!WIFEXITED(status) && !WIFEXITED(status));
    }

    return 1;
}

int execute(char ** args) {
    if(args[0] == NULL) {
        return 1;
    }

    for(int i = 0; i < num_built_ins(); i++) {
        if(strcmp(built_in_cmds[i], args[0]) == 0) {
            return (*built_in_func[i])(args);
        }
    }

    return _executeHelper(args);
}
char* readLine() {
    int buffSize = INIT_BUFF_SIZE;
    int position = 0;
    int ch;

    //Initialize a buffer to read the command.
    char *buffer = malloc(sizeof(char) * buffSize);

    //error handling if the the buffer fails to allocate memory.
    if(!buffer) {
        printf("shell: Fail to allocate memory.");
        exit(EXIT_FAILURE);
    }
    // a while loop that reads each character from the stdin and puts into the buffer.
    while(1) {
        // read character from the standard input
        ch = getchar();
        
        // if ch is EOF or a new line, return the buffer.
        if(ch == EOF || ch == '\n') {
            buffer[position] = '\0';
            return buffer;
        }
        buffer[position++] = ch;
        

        // if position is greater than the buffer size, then reallocate memory.
        if(position >= buffSize) {
            buffSize = buffSize + INIT_BUFF_SIZE;
            buffer = realloc(buffer, buffSize * sizeof(char));
            
            // check for failed allocation
            if(!buffer) {
                printf("shell: Fail to allocate memory.");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **readArgs(char *line) {
    int buffSize = SHELL_TOKEN_BUFF_SIZE;
    int position = 0;

    char ** tokens = malloc(sizeof(char *) * buffSize);

    if(!tokens) {
        printf("shell: Fail to allocate memory.");
        exit(EXIT_FAILURE);
    }

    char *token;

    token = strtok(line, SHELL_TOKEN_DELIMITER);
    while(token != NULL) {
        tokens[position++] = token;

        if(position >= buffSize) {
            buffSize = buffSize + SHELL_TOKEN_BUFF_SIZE;
            tokens = realloc(tokens, buffSize * sizeof(char *));
             if(!tokens) {
                printf("shell: Fail to allocate memory.");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, SHELL_TOKEN_DELIMITER);
    }
    tokens[position] = NULL;
    return tokens;
}
void shellLoop() {
    char *line;
    char **args;
    int status;

    do {
        printf("> ");
        line = readLine();
        args = readArgs(line);
        status = execute(args);


        free(args);
        free(line);

    } while(status);
}

int main(int argc, char** argv) {
    shellLoop();

    return EXIT_SUCCESS;
}