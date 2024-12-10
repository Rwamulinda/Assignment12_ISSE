// pipeline.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/wait.h> // For waitpid()
#include "pipeline.h"

// Create a new pipeline
Pipeline *pipeline_create() {
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (!pipeline) {
        return NULL;
    }

    pipeline->head = NULL;
    pipeline->tail = NULL;
    pipeline->command_count = 0;
    pipeline->input_file = NULL;
    pipeline->output_file = NULL;

    return pipeline;
}
// Execute the pipeline (fork and exec each command, handle redirection)
int execute_pipeline(Pipeline *pipeline) {
    if (!pipeline || pipeline->command_count == 0) {
        return -1;  // No pipeline or empty pipeline
    }

    int pipe_fds[2];
    int prev_fd = -1;  // No previous file descriptor (stdin for the first command)

    // Iterate through the commands in the pipeline
    PipelineNode *current = pipeline->head;
    while (current) {
        // Create a pipe for each command except the last
        if (current->next) {
            if (pipe(pipe_fds) == -1) {
                perror("pipe");
                return -1;
            }
        }

        // Fork to create a new process for each command
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return -1;
        }

        if (pid == 0) {  // Child process
            // Redirect input if there is a previous command
            if (prev_fd != -1) {
                if (dup2(prev_fd, STDIN_FILENO) == -1) {
                    perror("dup2 input");
                    exit(EXIT_FAILURE);
                }
                close(prev_fd);
            }

            // Redirect output if there is a next command
            if (current->next) {
                if (dup2(pipe_fds[1], STDOUT_FILENO) == -1) {
                    perror("dup2 output");
                    exit(EXIT_FAILURE);
                }
            } else if (pipeline->output_file) {  // If it's the last command and has an output file
                int output_fd = open(pipeline->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (output_fd == -1) {
                    perror("open output file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(output_fd, STDOUT_FILENO) == -1) {
                    perror("dup2 output file");
                    exit(EXIT_FAILURE);
                }
                close(output_fd);
            }

            // Execute the command
            execvp(current->command->args[0], current->command->args);
            perror("execvp");  // If execvp fails
            exit(EXIT_FAILURE);
        } else {  // Parent process
            // Close the write end of the pipe in the parent
            if (current->next) {
                close(pipe_fds[1]);
            }

            // Wait for the child to finish if it's the last command
            if (!current->next) {
                waitpid(pid, NULL, 0);
            }

            // Close the previous file descriptor after use
            if (prev_fd != -1) {
                close(prev_fd);
            }

            // Move to the next command
            prev_fd = pipe_fds[0];
        }

        current = current->next;
    }

    return 0;
}
// Destroy a pipeline and free all memory
void pipeline_destroy(Pipeline *pipeline) {
    if (!pipeline) return;

    // Free input and output files
    free(pipeline->input_file);
    free(pipeline->output_file);

    // Free each command in the pipeline
    PipelineNode *current = pipeline->head;
    while (current) {
        PipelineNode *next = current->next;

        // Free arguments in the command
        for (int i = 0; i < current->command->arg_count; i++) {
            free(current->command->args[i]);
        }
        free(current->command->args);
        free(current->command);
        free(current);

        current = next;
    }

    // Free the pipeline itself
    free(pipeline);
}

// Add a command to the pipeline
int pipeline_add_command(Pipeline *pipeline, Command *command) {
    if (!pipeline || !command) {
        return 0;
    }

    // Create a new pipeline node
    PipelineNode *new_node = malloc(sizeof(PipelineNode));
    if (!new_node) {
        return 0;
    }

    // Deep copy the command
    new_node->command = malloc(sizeof(Command));
    if (!new_node->command) {
        free(new_node);
        return 0;
    }

    // Copy command details
    new_node->command->args = malloc(sizeof(char *) * command->arg_capacity);
    if (!new_node->command->args) {
        free(new_node->command);
        free(new_node);
        return 0;
    }

    new_node->command->arg_count = command->arg_count;
    new_node->command->arg_capacity = command->arg_capacity;

    // Copy argument strings
    for (int i = 0; i < command->arg_count; i++) {
        new_node->command->args[i] = strdup(command->args[i]);
    }

    // Link the new node
    new_node->next = NULL;
    if (!pipeline->head) {
        pipeline->head = new_node;
        pipeline->tail = new_node;
    } else {
        pipeline->tail->next = new_node;
        pipeline->tail = new_node;
    }

    pipeline->command_count++;
    return 1;
}

// Set input file for the pipeline
void pipeline_set_input_file(Pipeline *pipeline, const char *filename) {
    if (!pipeline) return;

    // Free existing input file if any
    free(pipeline->input_file);

    // Set new input file
    pipeline->input_file = filename ? strdup(filename) : NULL;
}

// Set output file for the pipeline
void pipeline_set_output_file(Pipeline *pipeline, const char *filename) {
    if (!pipeline) return;

    // Free existing output file if any
    free(pipeline->output_file);

    // Set new output file
    pipeline->output_file = filename ? strdup(filename) : NULL;
}

// Optional: Print pipeline details (useful for debugging)
void pipeline_print(Pipeline *pipeline) {
    if (!pipeline) {
        printf("Pipeline is NULL\n");
        return;
    }

    printf("Pipeline Details:\n");
    printf("Command Count: %d\n", pipeline->command_count);
    printf("Input File: %s\n", pipeline->input_file ? pipeline->input_file : "stdin");
    printf("Output File: %s\n", pipeline->output_file ? pipeline->output_file : "stdout");

    PipelineNode *current = pipeline->head;
    int cmd_index = 0;
    while (current) {
        printf("Command %d:\n", cmd_index++);
        for (int i = 0; i < current->command->arg_count; i++) {
            printf("  Arg %d: %s\n", i, current->command->args[i]);
        }
        current = current->next;
    }
}