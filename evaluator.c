#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "pipeline.h"

int execute_builtin(Command *cmd) {
    if (strcmp(cmd->args[0], "exit") == 0 || strcmp(cmd->args[0], "quit") == 0) {
        exit(0);
    }
    if (strcmp(cmd->args[0], "author") == 0) {
        printf("Uwase Pauline\n");
        return 1;
    }
    if (strcmp(cmd->args[0], "pwd") == 0) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("pwd");
        }
        return 1;
    }
    if (strcmp(cmd->args[0], "cd") == 0) {
        const char *dir = cmd->arg_count > 1 ? cmd->args[1] : getenv("HOME");
        if (chdir(dir) != 0) {
            perror("cd");
        }
        return 1;
    }
    return 0;
}

int execute_pipeline(Pipeline *pipeline) {
    int pipes[2][2];
    int in_fd = STDIN_FILENO;
    
    // Handle input redirection
    if (pipeline->input_file) {
        in_fd = open(pipeline->input_file, O_RDONLY);
        if (in_fd == -1) {
            perror("Input file open");
            return -1;
        }
    }

    for (int i = 0; i < pipeline->command_count; i++) {
        // Check for builtin commands
        if (execute_builtin(&pipeline->commands[i])) {
            continue;
        }

        // Set up pipe for next command (except last command)
        if (i < pipeline->command_count - 1) {
            if (pipe(pipes[i % 2]) == -1) {
                perror("pipe");
                return -1;
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {  // Child process
            // Redirect input
            if (in_fd != STDIN_FILENO) {
                if (dup2(in_fd, STDIN_FILENO) == -1) {
                    perror("dup2 input");
                    exit(1);
                }
                close(in_fd);
            }

            // Redirect output (for all but last command)
            if (i < pipeline->command_count - 1) {
                if (dup2(pipes[i % 2][1], STDOUT_FILENO) == -1) {
                    perror("dup2 output");
                    exit(1);
                }
                close(pipes[i % 2][0]);
                close(pipes[i % 2][1]);
            }

            // Handle output redirection for last command
            if (i == pipeline->command_count - 1 && pipeline->output_file) {
                int out_fd = open(pipeline->output_file, 
                                  O_WRONLY | O_CREAT | O_TRUNC, 
                                  0644);
                if (out_fd == -1) {
                    perror("Output file open");
                    exit(1);
                }
                if (dup2(out_fd, STDOUT_FILENO) == -1) {
                    perror("dup2 output file");
                    exit(1);
                }
                close(out_fd);
            }

            // Execute command
            execvp(pipeline->commands[i].args[0], pipeline->commands[i].args);
            perror("execvp");
            exit(1);
        }

        // Parent process
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }

        // Prepare input for next iteration
        if (i < pipeline->command_count - 1) {
            close(pipes[i % 2][1]);
            in_fd = pipes[i % 2][0];
        }

        // Wait for child
        int status;
        waitpid(pid, &status, 0);
        if (WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Child %d exited with status %d\n", pid, WEXITSTATUS(status));
        }
    }

    return 0;
}