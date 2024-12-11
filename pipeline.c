#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pipeline.h"
#include "ast.h"

// Function to handle built-in commands
int handle_builtin_commands(char *command, char **args) {
    if (strcmp(command, "cd") == 0) {
        // Handle `cd` (change directory)
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
            return 1;
        }
        if (chdir(args[1]) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    } else if (strcmp(command, "exit") == 0) {
        // Handle `exit` (exit the shell)
        exit(0);
    } else if (strcmp(command, "echo") == 0) {
        // Handle `echo` (print arguments)
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
        return 0;
    }
    return -1;  // Return -1 if not a built-in command
}

// Function to execute the pipeline
void execute_pipeline(ASTNode *ast) {
    ASTNode *current = ast;
    int pipe_fds[2];
    int prev_pipe_fd = -1;

    while (current != NULL) {
        // Check if the current command is a built-in command
        if (handle_builtin_commands(current->command, current->args) == 0) {
            // If it's a built-in, just execute it directly
            current = current->next;  // Move to the next command in the pipeline
            continue;
        }

        // If it's not a built-in, execute it via pipeline
        if (pipe(pipe_fds) == -1) {
            perror("Error creating pipe");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid == 0) {  // Child process
            // If there was a previous pipe, redirect input from it
            if (prev_pipe_fd != -1) {
                dup2(prev_pipe_fd, STDIN_FILENO);
                close(prev_pipe_fd);
            }

            // If this is not the last command, redirect output to the pipe
            if (current->next != NULL) {
                dup2(pipe_fds[1], STDOUT_FILENO);
            }

            close(pipe_fds[0]);  // Close read end in child process

            // Execute the command
            if (execvp(current->command, current->args) == -1) {
                perror("Error executing command");
                exit(EXIT_FAILURE);
            }
        } else if (pid > 0) {  // Parent process
            wait(NULL);  // Wait for the child to finish
            close(pipe_fds[1]);  // Close write end in parent process

            // Update the previous pipe read end for the next command
            prev_pipe_fd = pipe_fds[0];
        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }

        current = current->next;  // Move to the next command in the pipeline
    }
}
