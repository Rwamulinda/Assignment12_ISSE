// pipeline.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "pipeline.h"
#include "ast.h"

// Function to handle built-in commands
int handle_builtin_commands(Command *cmd) {
    if (cmd == NULL || cmd->args == NULL || cmd->arg_count == 0) return -1;

    const char *command = cmd->args[0];

    if (strcmp(command, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
            return 0;
        }
        perror("pwd");
        return 1;
    } else if (strcmp(command, "author") == 0) {
        printf("Uwase Pauline\n");
        return 0;
    } else if (strcmp(command, "cd") == 0) {
        const char *target_dir = (cmd->arg_count > 1) ? cmd->args[1] : getenv("HOME");
        if (target_dir == NULL || chdir(target_dir) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    } else if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        exit(0);
    }

    return -1; // Not a built-in command
}

// Redirection handling function
int handle_redirection(char **args) {
    int input_fd = -1, output_fd = -1;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0 && args[i + 1]) {
            input_fd = open(args[i + 1], O_RDONLY);
            if (input_fd == -1)
                perror("input redirection");
            args[i] = NULL; // Remove redirection token
        } else if (strcmp(args[i], ">") == 0 && args[i + 1]) {
            output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1)
                perror("output redirection");
            args[i] = NULL; // Remove redirection token
        }
    }

    if (input_fd != -1)
        dup2(input_fd, STDIN_FILENO), close(input_fd);
    
    if (output_fd != -1)
        dup2(output_fd, STDOUT_FILENO), close(output_fd);

    return input_fd != -1 || output_fd != -1 ? 0 : -1;
}

// Function to execute the pipeline
void execute_pipeline(Pipeline *pipeline, char *errmsg, size_t errmsg_size) {
    Pipeline *current = pipeline; 
    int pipe_fds[2];
    int prev_pipe_fd = -1;

    // Handle empty pipeline
    if (current == NULL) {
        snprintf(errmsg, errmsg_size, "No command specified");
        return;
    }

    while (current != NULL) {
        // Validate command
        if (current->command == NULL || current->command->args == NULL || 
            current->command->arg_count == 0) {
            snprintf(errmsg, errmsg_size, "Invalid or empty command");
            return;
        }

        // Check for built-in commands first
        int builtin_result = handle_builtin_commands(current->command);
        if (builtin_result == 0) {
            current = current->next; 
            continue; 
        } else if (builtin_result > 0) {
            // Built-in command encountered an error
            snprintf(errmsg, errmsg_size, "Built-in command failed");
            return;
        }

        // Create a pipe for inter-process communication
        if (current->next != NULL && pipe(pipe_fds) == -1) {
            snprintf(errmsg, errmsg_size, "Error creating pipe");
            return; 
        }

        pid_t pid = fork();
       
        if (pid == 0) { 
            // Child process
            
            // Handle input/output redirections
            if (current->input_file) {
                int input_fd = open(current->input_file, O_RDONLY);
                if (input_fd == -1) {
                    perror("Input file error");
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            if (current->output_file) {
                int output_fd = open(current->output_file, 
                                     O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd == -1) {
                    perror("Output file error");
                    exit(EXIT_FAILURE);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            // If there was a previous pipe, redirect input from it
            if (prev_pipe_fd != -1)
                dup2(prev_pipe_fd, STDIN_FILENO);
           
            // If this is not the last command, redirect output to the pipe
            if (current->next != NULL)
                dup2(pipe_fds[1], STDOUT_FILENO);

            close(pipe_fds[0]); 

            // Execute command
            execvp(current->command->args[0], current->command->args); 
            
            // If execvp fails
            perror("Command not found");
            exit(EXIT_FAILURE); 
            
        } else if (pid > 0) { 
            // Parent process
            wait(NULL); 

            close(pipe_fds[1]); 

            prev_pipe_fd = pipe_fds[0]; 

            current = current->next; 
            
        } else { 
            snprintf(errmsg, errmsg_size, "Fork failed");
            perror("fork");
            exit(EXIT_FAILURE); 
        }
    }
}