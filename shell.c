#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

/* maximum number of args in a command */
#define LENGTH 20 

struct job{ //jobs defined as linked lists
    pid_t pid;
    int line_place;
    struct job *next_job;
    char *command_name;
    char *command_param;
};

int status; // status of jobs can change so we attach a global variable for every one of them
struct job *head_job; //create head of linkedlist
struct job *tail_job; //create tail of linkedlist
pid_t background_pid;
pid_t foreground_pid;
int addition_job_place = 1;  //place of current job in linked list
int global_file_holder = -1;
int stdout_place;
int fd[2]; //PIPE


int resetJobs(){ //reset every job in background
    struct job *cur = head_job;
    while(cur != NULL){ //free every job from memory
        struct job *next_cur = cur;
        cur = cur->next_job;
        free(next_cur);
    } 
    return 0;
}

struct job *create_job(pid_t *pid){ //create job function with allocating memory
    struct job* resJob = malloc(sizeof(struct job));
    if(head_job == NULL){
        resJob->pid = *pid;
    }
    return resJob;
}

void printJobs(){ //for testing
    struct job *cur_job = head_job;

    while(cur_job != NULL){

        printf("%s,%s|", cur_job->command_name, cur_job->command_param);
        cur_job = cur_job->next_job;
    }
    printf("\n");
    return;
}


int addJob(pid_t *pid, char* args[]){ //adding job to the linked list

    //printf("Adding Job");
    struct job* job = create_job(pid);
    if(head_job == NULL){ //if empty
        (*job).line_place = addition_job_place;
        head_job = job;
        tail_job = job;
        job->command_name = args[0];
        job->command_param = args[1];
        addition_job_place++;
    }
    else{ //if not empty add to tail
        struct job *temp_job = head_job;
        while(temp_job->next_job != NULL){
            temp_job = temp_job->next_job;
        }
        temp_job->next_job = job;
        (*job).line_place = addition_job_place;
        job->pid = *pid; //dont forget pid 
        (*job).next_job = NULL; //next is null
        job->command_name = args[0];
        job->command_param = args[1];
        addition_job_place++; //line_place increased for extra addition after
        
    }
    //printf("Current job count: %d", addition_job_place - 1);
    return 0;

}

int locateJob(int place_in_line){

    struct job *finder = head_job;
    while(finder != NULL){  //go through every job in list and return if the intended place for job is same - so same job.
        if(finder->line_place == place_in_line){
            return finder->line_place;
        }
        finder = finder->next_job;
    }
    return -1;
}


//all functions
int runCd(char* file_path){
    if(file_path == NULL){
        printf(file_path);
        printf("\n");
        return 1;
    }
    else{
        if(chdir(file_path) < 0){
            printf("Error: Failed to locate file.");
            return -1;
        }
        else{
            return 0;
        }

    }
    return -1;
}

void freeJobMem(struct job *cur_job, struct job *prev_job){
    
}

int runJobs(){ //run jobs command
    struct job *prev_job = head_job;//need the previously running command
    struct job *cur_job;  //need the currently running command  
    struct job *printer_job;
     
    int clear_pids[LENGTH];
    int i = 0;
    //printJobs();
    int first_done = 0; //flag to fix bug
    for(cur_job = head_job; cur_job != NULL; 1){ //go through every job
        int waiter_id = waitpid(cur_job->pid, &status, WNOHANG);

        printf("%d",cur_job->line_place);
        printer_job = cur_job;
        if(waiter_id == 0){  //if running
            
            printf("[Running]+\t"); //print running statement   
            prev_job = cur_job;
            cur_job = cur_job->next_job;
            //printf("1prev is %s, %s\n",(*prev_job).command_name, (*prev_job).command_param );
            //printf("cur is %s, %s\n",(*cur_job).command_name, (*cur_job).command_param );
        }
        else if(waiter_id > 0){
            printf("[Done]-\t");
            if(cur_job->line_place == head_job->line_place){
                //delete head job and let head be the next job
                head_job = head_job->next_job;
                free(cur_job); //we love memory saving
                prev_job = head_job;
                cur_job = head_job;
            }
            else{
                //skip cur_job and delete it
                prev_job->next_job = cur_job->next_job;
                free(cur_job);
                cur_job = prev_job->next_job;
            }
            //printf("2prev is %s, %s\n",(*prev_job).command_name, (*prev_job).command_param );
            //printf("cur is %s, %s\n",(*cur_job).command_name, (*cur_job).command_param );
        }
        printf("Process command name:\t%s %s  \t Pid:\t %d\n", (*printer_job).command_name, (*printer_job).command_param, printer_job->pid); //print
    }
    //printJobs();
    return 0;
}
int runFg(char *args[]){
    pid_t res_pid;
    struct job *prev_job = head_job;
    int locater_num = atoi(args[1]);
    if(args[1] == NULL){
        return -1;
    }
    else{
        struct job *repeater = head_job;
        while(repeater != NULL){
            if(repeater->line_place == locater_num){
                res_pid = repeater->pid;
                printf("%s %s\n",repeater->command_name, repeater->command_param);
                if(waitpid(res_pid,NULL,WUNTRACED) < 0){
                    return -1;
                }
                prev_job->next_job = repeater->next_job;
                return 0;
            }
            prev_job = repeater;
            repeater = repeater->next_job; //crucial
        }
        return -1;
    }

}

int runPwd(){
    char cwd[1024];
    if(getcwd(cwd, sizeof(cwd)) != NULL){ //get path
        printf("%s\n", cwd); //print path
        return 0;
    }
    else{
        return -1;
    }
    
}

int runEcho(char *args[]){

    int i = 1; //start after echo
    while(args[i] != NULL){ //print every arg after echo
        printf("%s", args[i]);
        i++;
        printf(" "); //space after every arg
    }
    printf("\n");
    return 0;
}


//redirection handler
int runRedirection(int arg_count, char *args[], int redirect_place){

    if(args[redirect_place + 1] != NULL){ //if there is a target
        if(redirect_place + 2 <= arg_count){ //if there are just the necessary arguments!
            //printf("Redirect open.\n");
            stdout_place = dup(1);
            global_file_holder = open(args[arg_count - 1], O_CREAT | O_WRONLY | O_APPEND, 0666);
            args[arg_count - 1] = NULL;
            args[arg_count - 2] = NULL;
            dup2(global_file_holder, 1);
            return 0;
        }
    }
    return -1;
}
//pipe checker
int checkPipe(int arg_count, char *args[], int redirect_place){

    if(args[redirect_place + 1] != NULL){ //if there is a target
        if(redirect_place + 2 <= arg_count){ //if there are just the necessary arguments!
            //printf("Pipe function open.\n");
            pipe(fd); //created pipe
            return 0;
        }
    }
    return -1;
}
//pipe handler
int runPipe(char *args[], int count){

    char *parent_args[2];
    char *child_args[count - 1];

    parent_args[0] = args[0];
    parent_args[1] = NULL;
    for(int i = 0; i < count; i++){
        child_args[i] = args[i + 2];
    }
    //printf("%s, %s, %s\n",child_args[0], child_args[1], child_args[2]);
    //printf("%s, %s\n",parent_args[0], parent_args[1]);
    //printf("%s, %s, %s\n", args[0], args[1], args[2]);
    
    int pp= getppid(); //current pid
    
    int first_child_pid = fork();

    if(first_child_pid == 0){
        close(1); //close stdout
        dup(fd[1]); //replace with pipe write end
        close(fd[0]); //close unused end for pipe
        if(execvp(args[0], parent_args) < 0) { //execute first half of pipe
            perror("execvp");
            exit(1); 
        }
        close(fd[1]); //closed used end because it is copied!
        exit(0); //close child
    }
    int second_child_pid = fork();

    if(second_child_pid == 0){
        close(0); //close stdin
        dup(fd[0]); //replace with pipe read end
        close(fd[1]); //close unused end for pipe!
        if(execvp(args[2], child_args) < 0) {  //execute second part of pipe
            perror("execvp");
            exit(1); 
        }
        close(fd[0]); //close the first end double after execution(because of duplication)
        exit(0); //close child
    }
    //after parent we make sure to close every end of pipe
    close(fd[0]); //close the original double end of pipe! To not be stuck(!)
    close(fd[1]);
    waitpid(first_child_pid, NULL, 0); //wait for both childs with no flags !!
    waitpid(second_child_pid, NULL, 0);//wait for both childs with no flags !!

    return 0;
    /*if(pipe_pid == 0){//child process of pipe

        printf("Running Child...");
        dup2(fd[0], 0);   //dup from fd 0
        close(fd[1]); //close the unneeded side
        if(execvp(args[2], child_args) < 0) { 
            perror("execvp");
            exit(1); 
        }
        
    }
    else{//parent  process of pipe

        printf("Running parent.\n");
        dup2(fd[1], 1);
        close(fd[0]);
        if(execvp(args[0], parent_args) < 0) { 
            perror("execvp");
            exit(1); 
        }
        waitpid(pipe_pid, &status, WUNTRACED);
        return 0;
    }*/
    
}


void sigint_handler(int sig) {
    // add your code here
    if(foreground_pid != background_pid){ //if in background
        kill(background_pid, SIGKILL); //kill process
        //printf("\nYou have pressed CTRL-C this caused the current process to stop\n");
    }
    return;
}

void handle_sigtstp(int sig) {
    // add your code here
    signal(sig, SIG_IGN);
    //printf("\nYou have pressed CTRL-Z this caused the current process to ignore your signal\n");
}

/* Parses the user command in char *args[]. 
   Returns the number of entries in the array */

int getcmd(char *prompt, char *args[], int *background){

    int length, flag, i = 0;
    char *token, *loc;
    char *line = NULL;
    size_t linecap = 0;

    printf("%s", prompt);
    length = getline(&line, &linecap, stdin);

    // check fof ionvalid command
    if (length == 0) {
        return 0; 
    }
    // check for CTRL + D
    else if (length == -1){
        exit(0);
    }

    /* check if background is specified */
    if ((loc = index(line, '&')) != NULL) {
        *background = 1;
        *loc = ' ';
    } else {
        *background = 0;
    }
    
    // Clear args 
    memset(args, '\0', LENGTH);

    // Splitting the command and putting the tokens inside args[]
    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++) {
            if (token[j] <= 32) { 
                token[j] = '\0'; 
            }
            // Here you can add the logic to detect piping or redirection in the command and set the flags
        }
        if (strlen(token) > 0) {
            args[i++] = token;
        }
    }
    
    return i;
}


int main(void) { 
    char* args[LENGTH];
    char* abs_path_name[50];
    int redirection; /* flag for output redirection */
    int piping; /* flag for piping */
    int bg;     /* flag for running processes in the background */
    int cnt; /* count of the arguments in the command */
    foreground_pid = getppid();

    printf("\n\n******************************SHELL OF EGE******************************\n\n");

    if (signal(SIGINT, sigint_handler) == SIG_ERR){ 
            printf("ERROR: could not bind signal handler for SIGINT\n");
            exit(1);
    }


    while(1){
        // reset flags 
        bg = 0;
        background_pid = getppid(); //background or running pid
        int pipeFlag = 0;

        if ((cnt = getcmd("\n$-->Ege's Shell>> ", args, &bg)) == 0) {
            printf("Invalid command\n");
            continue;
        }
        if (signal(SIGTSTP, handle_sigtstp) == SIG_ERR){ 
            printf("ERROR: could not bind signal handler for SIGTSTP\n");
            exit(1);
        }
        
        if (signal(SIGINT, sigint_handler) == SIG_ERR){ 
            printf("ERROR: could not bind signal handler for SIGINT\n");
            exit(1);
        }
        
        for (int l = 0; l < cnt + 1; l++) {
            if(l == cnt){
                args[l] = NULL;
            }
        }
        int i = 0;
        for(i = 0; i < cnt; i++){
            if(strcmp(args[i], "|") == 0){ //if there is output pipes
                //printf("Command piping commencing...\n");
               
                if(checkPipe(cnt, args, i) >= 0){
                    pipeFlag = 1; //we need to use later on to connect pipe ends!
                };
            }   
        }
        
        

        //all parent commands
        if(strcmp(args[0],"cd") == 0){
                if(runCd(args[1]) < 0){
                    printf("Failed to execute CD command\n");

                }
        }
        else if(strcmp(args[0],"fg") == 0){ 
            if(runFg(args) < 0){
                printf("Failed to execute FG command.\n");
            }
            continue;
            
        }
        else if(strcmp(args[0],"jobs") == 0){
            if(runJobs() < 0){
                printf("Failed to execute JOBS command.\n");
            }
        }
        else if(strcmp(args[0],"exit") == 0){

            resetJobs(); //memory reset
            exit(0); //exit
        }
        else if(strcmp(args[0],"jobs") == 0){
            if(runJobs() < 0){
                printf("Failed to execute JOBS command.\n");
            }
        }
        else if(strcmp(args[0],"pwd") == 0){
            if(runPwd() < 0){
                printf("Failed to execute PWD command.\n");
            }
        }
        else if(strcmp(args[0],"echo") == 0){
            if(runEcho(args) < 0){
                printf("Failed to execute ECHO command.\n");
            }
        }
        //pipes for child
        else if(pipeFlag == 1){
            //printf("Processing command pipe...\n");
            runPipe(args, cnt);
            pipeFlag = 0;   //reset pipe
            continue;
        }
        else{
            int pid = fork();

        // For the child process
            if (pid == 0) {
            // execute the command
                sleep(0.2); //wait just in case, we do not want any unnecesarry errors
                
                int redirectFlag = 0; //if redirect is there => 1, otherwise => 0, initialize as 0
                
                int file = -1; //initialize as -1

                //redirect
                int i = 0;
                for(i = 0; i < cnt; i++){

                    if(strcmp(args[i], ">") == 0){ //if there is output redirection
                        printf("Output redirection commencing...\n");
                        redirectFlag = 1; //we need to use later on to close redirection!
                        file = runRedirection(cnt, args, i);
                        break;
                    }
                }

                
                
                    if(execvp(args[0], args) < 0) {  //if
                        //perror("execvp");
                        printf("Invalid Command.");
                        exit(1); 
                    }
                
            
                //exit(0); /* child termination */

            }   
            else if (pid == -1){      
                printf("ERROR: fork failed\n");
                exit(1);
            }
            // For the parent process
            else {
                if(bg == 1){
                    addJob(&pid, args);
                    //printf("Adding job.");
                }
            // If the child process is not in the bg => wait for it
                else if (bg == 0){ 
                    background_pid = pid;
                    waitpid(pid, &status, WUNTRACED);
                }  
        
            }
            if(global_file_holder != -1){ //reset if output redirection is used!
                dup2(stdout_place, 1);
                fflush(stdout);
                close(global_file_holder);
                global_file_holder = -1;
            }
        }
    }
}