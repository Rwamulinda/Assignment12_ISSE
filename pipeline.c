#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "pipeline.h"
#include "ast.h"
#include "parse.h"

// Function to handle built-in commands
// Enhanced built-in commands
int handle_builtin_commands(Command *cmd)
{
    if (cmd == NULL || cmd->args == NULL || cmd->arg_count == 0) {
        return -1;
    }

    // Safely access the first argument (command name)
    const char *command = cmd->args[0];

    if (strcmp(command, "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
            return 0;
        }
        perror("pwd");
        return 1;
    }

    if (strcmp(command, "author") == 0) {
        printf("Uwase Pauline\n");
        return 0;
    }

    if (strcmp(command, "cd") == 0) {
        // Handle cd with optional directory argument
        const char *target_dir = (cmd->arg_count > 1) ? cmd->args[1] : getenv("HOME");
        
        if (target_dir == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return 1;
        }

        if (chdir(target_dir) != 0) {
            perror("cd");
            return 1;
        }
        return 0;
    }

    if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
        exit(0);
    }

    return -1; // Not a built-in command
}
// Redirection handling
int handle_redirection(char **args)
{
    int input_fd = -1, output_fd = -1;

    // Scan for < and > redirections
    for (int i = 0; args[i] != NULL; i++)
    {
        if (strcmp(args[i], "<") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "Missing input file\n");
                return -1;
            }
            input_fd = open(args[i + 1], O_RDONLY);
            if (input_fd == -1)
            {
                perror("input redirection");
                return -1;
            }
            // Remove redirection tokens
            args[i] = NULL;
        }

        if (strcmp(args[i], ">") == 0)
        {
            if (args[i + 1] == NULL)
            {
                fprintf(stderr, "Missing output file\n");
                return -1;
            }
            output_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1)
            {
                perror("output redirection");
                return -1;
            }
            // Remove redirection tokens
            args[i] = NULL;
        }
    }

    // Perform redirections if needed
    if (input_fd != -1)
    {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    if (output_fd != -1)
    {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }

    return 0;
}

// Function to execute the pipeline
void execute_pipeline(Pipeline *pipeline, char *errmsg, size_t errmsg_size)
{
    if (pipeline == NULL) {
        snprintf(errmsg, errmsg_size, "Empty pipeline");
        return;
    }

    Pipeline *current = pipeline;
    int prev_pipe_fd = -1;

    while (current != NULL)
    {
        Command *cmd = current->command;
        
        // Validate command
        if (cmd == NULL || cmd->args == NULL || cmd->arg_count == 0) {
            snprintf(errmsg, errmsg_size, "Invalid command in pipeline");
            return;
        }

        // Check for built-in commands
        int builtin_result = handle_builtin_commands(cmd);
        if (builtin_result == 0) {
            current = current->next;
            continue;
        }

        // Prepare for external command execution
        int pipe_fds[2];
        if (current->next != NULL && pipe(pipe_fds) == -1) {
            snprintf(errmsg, errmsg_size, "Pipe creation failed");
            return;
        }

        pid_t pid = fork();
        if (pid == 0) {  // Child process
            // Handle input redirection
            if (current->input_file) {
                int input_fd = open(current->input_file, O_RDONLY);
                if (input_fd == -1) {
                    perror("Input redirection failed");
                    exit(EXIT_FAILURE);
                }
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }

            // Handle output redirection
            if (current->output_file) {
                int output_fd = open(current->output_file, 
                                     O_WRONLY | O_CREAT | O_TRUNC, 
                                     0644);
                if (output_fd == -1) {
                    perror("Output redirection failed");
                    exit(EXIT_FAILURE);
                }
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }

            // Handle piping
            if (prev_pipe_fd != -1) {
                dup2(prev_pipe_fd, STDIN_FILENO);
                close(prev_pipe_fd);
            }

            if (current->next != NULL) {
                dup2(pipe_fds[1], STDOUT_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
            }

            // Execute command
            execvp(cmd->args[0], cmd->args);
            
            // If execvp fails
            perror("Command execution failed");
            exit(EXIT_FAILURE);
        }
        else if (pid > 0) {  // Parent process
            // Wait for child
            int status;
            waitpid(pid, &status, 0);

            // Cleanup pipe resources
            if (current->next != NULL) {
                close(pipe_fds[1]);
                prev_pipe_fd = pipe_fds[0];
            }
        }
        else {
            snprintf(errmsg, errmsg_size, "Fork failed");
            return;
        }

        current = current->next;
    }
}