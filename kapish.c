/* 
 * File: kapish.c
 * Name: Li Ce (Shawn) Wang
 * ID: V00878878
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // Definition for fork() & execve()
#include <errno.h>      // Definition for error handling 
#include <sys/wait.h>   // Definition for wait() 
#include <sys/types.h>  // Defintion for pid_t
#include <signal.h>     // Defintiion for Control-C signal() 

#define LINE_BUFSIZE 512
#define HIST_BUFSIZE 100
#define TOK_DELIM " "
#define DIRECTORY "/.kapishrc.txt" //configuration file


// Functions declaration
int kapish_setenv(char** args); 
int kapish_unsetenv(char** args); 
int kapish_cd(char** args); 
int kapish_exit(char** args); 
int kapish_history(char** args);
char* kapish_read_line(void);
char* kapish_read_line_rc(FILE* fp);
char** kapish_make_tokens(char* line);
int kapish_execute(char* line); 
int kapish_launch(char** args); 
void read_rcfile(void);
void kapish_loop(void);
//int setenv(char* name, char* value, int overwrite); 
//int unsetenv (char* name); 
void SIGINT_handler(int); 
int setenv_var_handler(char** args);
int setenv_val_handler(char** args);


// List of builtin commands 
char* builtin_str[] = {
    "setenv", 
    "unsetenv", 
    "cd", 
    "exit",
}; 
int num_builtin(){
    return sizeof(builtin_str) / sizeof(char*); 
}
int (*builtin_func[]) (char**) = { // int array of builtin function memory addresses
    &kapish_setenv,
    &kapish_unsetenv, 
    &kapish_cd, 
    &kapish_exit, 
}; 

// KAPISH builtin setenv_var command 
int kapish_setenv(char** args){
    // existing environment variable 
    if (args[1] == NULL){
        return setenv_var_handler(args);  
    }
    else if (getenv(args[1])){ 
        if(args[2] == NULL){
            setenv_val_handler(args);
        }
        else{
            if (setenv(args[1], args[2], 1) != 0){
                perror("KAPISH");
            }
            return 1; 
        }
    }
    // new environment variable 
    else {
        if(args[2] == NULL){
            setenv_val_handler(args);
        }
        else{
            if (setenv(args[1], args[2], 0) != 0){
                perror("KAPISH");
            }
            return 1; 
        }
    }
    return 1;
}

// KAPISH builtin unsetenv_var command 
int kapish_unsetenv(char** args){
    if (args[1] == NULL){
        return setenv_var_handler(args); 
    }
    if (unsetenv(args[1]) != 0){ // unsetenv(args[1]) return 0 upon success
        perror("KAPISH"); // produce last encountered error message ending with ":"
    }
    return 1;
}

// KAPISH builtin cd command 
int kapish_cd(char** args){
    if (args[1] == NULL || strcmp(args[1], "~") == 0){
        chdir(getenv("HOME")); 
        return 1; 
    }
    else{
        chdir(args[1]); 
        if (chdir(args[1]) != 0){ // chdir(args[1]) return 0 upon success 
            perror("KAPISH"); // produce last encountered error message ending with ":"
        }
        return 1; 
    }
}

// KAPISH builtin exit command 
int kapish_exit(char** args){
    printf("KAPISH EXIT\n");
    return 0; 
}

// Signal handler terminate child process 
void SIGINT_handler(int sig){ 
    signal(SIGINT, SIGINT_handler);
}

// Error handler for setenv or unsetenv variable missing
int setenv_var_handler(char** args){
    printf("Missing environment variable: var\n");
    return 1;
}

// Error handler for setenv value missing 
int setenv_val_handler(char** args){
    args[2] = " "; 
    if (setenv(args[1], args[2], 1) != 0){
        perror("KAPISH");
    }    
    return 1; 
}

// Read line characters 
char* kapish_read_line(void){
    int bufsize = LINE_BUFSIZE; 
    int i = 0; 
    char* buffer = malloc(sizeof(char) * bufsize); 
    int c; 
    
    if (!buffer){
        fprintf(stderr, "KAPISH: allocation error\n"); 
        exit(EXIT_FAILURE); 
    }

    while (1){
        c = getchar(); 
        if (c == EOF){
            printf("\nKAPISH EXIT\n");
            exit(EXIT_SUCCESS); 
        }
        else if (c == '\n'){
            buffer[i] = '\0'; 
            return buffer; 
        }
        else{
            buffer[i] = c; 
        }
        i++;       
        if (i >= bufsize){
            bufsize += LINE_BUFSIZE; 
            buffer = realloc(buffer, bufsize); 
            if (!buffer){
                fprintf(stderr, "KAPISH: allocation error\n"); 
                exit(EXIT_FAILURE); 
            }
        }
    }
}

char* kapish_read_line_rc(FILE* fp){
    int bufsize = LINE_BUFSIZE; 
    int i = 0; 
    char* buffer = 0;
    buffer =  malloc(sizeof(char) * bufsize); 
    int c; 
    
    if (!buffer){
        fprintf(stderr, "KAPISH: allocation error\n"); 
        exit(EXIT_FAILURE); 
    }

    while (1){
        c = getc(fp); 
        if (c == EOF){
            return buffer; 
        }
        if (c == '\n'){
            buffer[i] = '\0'; 
            return buffer; 
        }
        else{
            buffer[i] = c; 
        }
        i++;       
        if (i >= bufsize){
            bufsize += LINE_BUFSIZE; 
            buffer = realloc(buffer, bufsize); 
            if (!buffer){
                fprintf(stderr, "KAPISH: allocation error\n"); 
                exit(EXIT_FAILURE); 
            }
        }
    }
}

// Parsing line into array of tokens 
char** kapish_make_tokens(char* line){
    int bufsize = LINE_BUFSIZE; 
    int i = 0; 
    char** tokens = malloc(sizeof(char*) * bufsize); 
    char* token; 
    char** tokens_backup; 
    
    if (!tokens){
        fprintf(stderr, "KAPISH: allocation error\n"); 
        exit(EXIT_FAILURE); 
    }
    
    token = strtok(line, TOK_DELIM); //first call with string to be parsed, returns ptr to next null terminated token 
        
    while (token != NULL){
        tokens[i] = token; 
        i++; 
        if (i >= bufsize){
            bufsize += LINE_BUFSIZE; 
            tokens_backup = tokens; 
            tokens = realloc(tokens, sizeof(char*) * bufsize); 
            if (!tokens){
                free(tokens_backup); 
                fprintf(stderr, "KAPISH: allocation error\n"); 
                exit(EXIT_FAILURE); 
            }
        }
        token = strtok(NULL, TOK_DELIM); //subsequent calls should parse same string using NULL
    }
    tokens[i] = NULL;
    return tokens; 
}

// Execute KAPISH builtin command or launch program
int kapish_execute(char* line){  
    char** args = kapish_make_tokens(line); 
    int i;
    if (args[0] == NULL){
        return 1; 
    }
    //KAPISH builtin commands 
    for (i =0; i < num_builtin(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args); 
        }
    }
    //launch external programs 
    return kapish_launch(args); 
}

// Launch program and wait for it to terminate 
int kapish_launch(char** args){
    pid_t pid;
    int status; 

    pid = fork(); 
    // child process
    if (pid == 0){ 
        if (execvp(args[0], args) == -1){ // v->vector/array & p->search for program in path instead of absolute path
            perror("KAPISH"); // produce last encountered error message ending with ":" 
        }
        exit(EXIT_FAILURE); 
    }
    // parent process 
    else if (pid > 0){
        do {     
            waitpid(pid, &status, WUNTRACED); // wait(&wstatus) == waitpid(-1, &wstatus, 0) & WUNTRACED->return if child stopped 
        } while(!WIFEXITED(status) && !WIFSIGNALED(status)); // WIFEXITED->true if child terminated normally & WIFSIGNAL->true if child termined by signal        
    }
    // fork error 
    else{
        perror("KAPISH"); // produce last encountered error message ending with ":"
    }
    return 1; 
}

// KAPISH bootup 
void kapish_boot(){
    printf("***************************************************************\n");
    printf("*                     WELCOME TO KAPISH                       *\n");
    printf("* TO RUN A COMMAND SIMPLY TYPE YOUR COMMAND AND PRESS 'ENTER' *\n");
    printf("* TO KILL CHILD TYPE 'Control-C'                              *\n");
    printf("* TO EXIT TYPE 'Control-D' or 'exit'                          *\n");
    printf("***************************************************************\n");
}

// KAPISH configuration file reader 
void read_rcfile(void){
    FILE* fp;  
    char* line; 
    int status; 
    
    char* dir1 = getenv("HOME");
    char* dir2 = malloc(strlen(dir1)+1 + strlen(DIRECTORY)+1);
    strcpy(dir2, dir1);

    if ((fp = fopen(strcat(dir2, DIRECTORY), "r")) == NULL){
        fprintf(stderr, "Failed to open .kapishrc file"); 
        exit(EXIT_FAILURE); 
    }
    do {
        fflush(stdout);
        fprintf(stderr, "? ");
        line = kapish_read_line_rc(fp);
		fflush(stdout);
        fprintf(stderr, "%s\n", line);
        status = kapish_execute(line); 
        free(line); 
    }while (status && !feof(fp)); 
    fclose(fp);
}

// KAPISH interactive terminal 
void kapish_loop(void){
    char *line; 
    int status; 

    do {
        fflush(stdout);
        fprintf(stderr, "? ");
        line = kapish_read_line();
		fflush(stdout);
        fprintf(stderr, "%s\n", line);
        status = kapish_execute(line); 
        free(line); 
    }while (status); 
}

int main() {
    kapish_boot(); 
    if (signal(SIGINT,SIGINT_handler) == SIG_ERR){
            printf ("SIGINT install error\n"); 
            exit(EXIT_FAILURE); 
        }
    read_rcfile(); 
    kapish_loop(); 
    return (EXIT_SUCCESS);
}


