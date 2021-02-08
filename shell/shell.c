#include "parser/ast.h"
#include "shell.h"
#include <sys/wait.h>
#include <signal.h>

//const char* default_message = "vush$ ";

void command(const node_t* node) {
    const char* sequence_command = node->command.program;
    const char** current_args = node->command.argv;
    
    if (strcmp(sequence_command, "exit") == 0) {
        exit(42);
    }
    
    else if (strcmp(sequence_command, "cd") == 0) {
        chdir(current_args[1]);
    }
    
    else {
        const int forked_process_id = fork();
        int status = 0;
        
        if (forked_process_id == 0) {
            execvp(sequence_command, current_args);
            perror("No such file or directory");
            exit(1);
        }
        
        else {
            wait(&status);
        }
    }
}

void sequence(const node_t* node) {
    const node_t* first = node->sequence.first;
    const node_t* second = node->sequence.second;
    command(first);
    
    if (second->type == NODE_SEQUENCE) {
        sequence(second);
    }
    
    else {
        command(second);
    }
}

void handle_pipe(const node_t* n) {
    const node_t** node_parts = n->pipe.parts;
    pid_t first_process_id;
    pid_t second_process_id;
    int pipe_forked_info[2];
    pipe(pipe_forked_info);
    first_process_id = fork();
    second_process_id = fork();
    
    
    if (first_process_id == 0) {
        execvp(node_parts[0]->command.program, node_parts[0]->command.argv);
    }
    
    else if (second_process_id == 0) {
        execvp(node_parts[1]->command.program, node_parts[1]->command.argv);
    }
    
    close(pipe_forked_info[0]);
    close(pipe_forked_info[1]);
    
}

void run_command(node_t *node) {
    switch (node->type) {
        case NODE_COMMAND:
            command(node);
            break;
        case NODE_PIPE:
            handle_pipe(node);
            break;
        case NODE_SEQUENCE:
            sequence(node);
            break;
    }
    
    if (prompt){
        prompt = "vush$ ";
    }
}

void noctrlz(){
    signal(SIGINT, noctrlz);
}

void initialize(void) {
    noctrlz();

    if (prompt)
        prompt = "vush$ ";
}
