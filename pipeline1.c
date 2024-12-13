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
    Pipeline *current = pipeline; // Start with the first command in the pipeline
    int pipe_fds[2];
    int prev_pipe_fd = -1;

    while (current != NULL)
    {
        // Check if the current command is a built-in command
        if (current->command != NULL)
        {
            if (handle_builtin_commands(current->command->command, current->command->args) == 0)
            {
                // If it's a built-in, just execute it directly
                current = current->next; // Move to the next command in the pipeline
                continue;
            }
        }

        // If it's not a built-in, execute it via pipeline
        if (pipe(pipe_fds) == -1)
        {
            snprintf(errmsg, errmsg_size, "Error creating pipe");
            return; // Return with error message
        }

        pid_t pid = fork();
        if (pid == 0)
        { // Child process
            // If there was a previous pipe, redirect input from it
            if (prev_pipe_fd != -1)
            {
                dup2(prev_pipe_fd, STDIN_FILENO);
                close(prev_pipe_fd);
            }

            // If this is not the last command, redirect output to the pipe
            if (current->next != NULL)
            {
                dup2(pipe_fds[1], STDOUT_FILENO);
            }

            close(pipe_fds[0]); // Close read end in child process

            // Execute the command
            if (execvp(current->command->command, current->command->args) == -1)
            {
                snprintf(errmsg, errmsg_size, "Error executing command: %s", current->command->command);
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        }
        else if (pid > 0)
        {                       // Parent process
            wait(NULL);         // Wait for the child to finish
            close(pipe_fds[1]); // Close write end in parent process

            // Update the previous pipe read end for the next command
            prev_pipe_fd = pipe_fds[0];
        }
        else
        {
            snprintf(errmsg, errmsg_size, "Fork failed");
            perror("fork");
            exit(EXIT_FAILURE);
        }

        current = current->next; // Move to the next command in the pipeline
    }
}
