#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include "defs.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>



pid_t launch_command(char** argv){
    /* To be completed */
    pid_t PID= fork();
    int j;
    if(PID==0){
        j = execvp(argv[0],argv);
        exit(1); 
    }
   return PID;
   
}



 char **parse_command(const char *cmd, int* argc) {
    // Allocate space for the argv array (initially with space for 10 args)
    size_t argv_size = 10;
    const char *end;
    size_t arg_len; 
    int arg_count = 0;
    const char *start = cmd;
    char **argv = malloc(argv_size * sizeof(char *));
    
    if (argv == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    while (*start && isspace(*start)) start++; // Skip leading spaces

    while (*start) {
        // Reallocate more space if needed
        if (arg_count >= argv_size - 1) {  // Reserve space for the NULL at the end
            argv_size *= 2;
            argv = realloc(argv, argv_size * sizeof(char *));
            if (argv == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }

        // Find the start of the next argument
        end = start;
        while (*end && !isspace(*end)) end++;

        // Allocate space and copy the argument
        arg_len = end - start;
        argv[arg_count] = malloc(arg_len + 1);

        if (argv[arg_count] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strncpy(argv[arg_count], start, arg_len);
        argv[arg_count][arg_len] = '\0';  // Null-terminate the argument
        arg_count++;

        // Move to the next argument, skipping spaces
        start = end;
        while (*start && isspace(*start)) start++;
    }

    argv[arg_count] = NULL; // Null-terminate the array

    (*argc)=arg_count; // Return argc

    return argv;
}
void ejecuta_fichero(char* file){
    FILE* f;
    f = fopen(file,"r");
    char comando[256];
    char **cmd_argv;
    pid_t pidcmd;
    int cmd_argc;
    while(fgets(comando,256,f)!=NULL){
        cmd_argv=parse_command(comando,&cmd_argc);
        pidcmd = launch_command(cmd_argv);
        waitpid(pidcmd,NULL,0);
        printf("PID Hijo: %d\n",pidcmd);
        
    }
    fclose(f);
    for (int i = 0; cmd_argv[i] != NULL; i++) {  
        free(cmd_argv[i]);  // Free individual argument
    }

    free(cmd_argv); 
    return;
}
extern char *optarg;
extern int optind, opterr, optopt;
int main(int argc, char *argv[]) {
    char **cmd_argv;
    int cmd_argc;
    int i;
    pid_t pidcmd;
    int  opt;
	struct options options;
    options.action=NONE_ACT;
  
    while ((opt = getopt(argc, argv, "x:s:")) != -1)
	{
		switch (opt)
		{
		
		case 'x':
			options.cmds= optarg;
            options.action=X_CMD_ACT;
			break;
		case 's':
			options.input_file= optarg;
            options.action=FILE_X_ACT;
			break;
		default:
			exit(EXIT_FAILURE);
		}
	}
    switch (options.action)
    {
        case X_CMD_ACT:
        cmd_argv=parse_command(options.cmds,&cmd_argc);
        pidcmd = launch_command(cmd_argv);
        printf("PID hijo: %d\n",pidcmd);
        for (i = 0; cmd_argv[i] != NULL; i++) {  
        free(cmd_argv[i]);  // Free individual argument
        }

        free(cmd_argv); 
        break;
        case FILE_X_ACT:
            ejecuta_fichero(options.input_file);
        break;
        case NONE_ACT:
        fprintf(stderr, "Must indicate one of the following options: -x, -s, -b \n");
		
        break;
        default:
        break;
    }

    

     // Free the cmd_argv array

    return EXIT_SUCCESS;
}