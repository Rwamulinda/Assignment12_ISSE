#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include <stdlib.h>

// Represents a single command with its arguments
typedef struct {
    char **args;       // Dynamically allocated array of arguments
    int arg_count;     // Number of arguments
    char *executable;  // Path or name of the executable
} Command;

// Represents the entire pipeline of commands
typedef struct {
    Command *commands;     // Array of commands to execute
    int command_count;     // Number of commands in the pipeline
    
    // Redirection support
    char *input_file;      // Input redirection file (optional)
    char *output_file;     // Output redirection file (optional)
    
    // Pipeline-specific flags
    int is_piped;          // Whether commands are piped together
} Pipeline;

// Function prototypes for pipeline management
Pipeline* pipeline_create();
void pipeline_add_command(Pipeline *pipeline, Command *command);
void pipeline_set_input_file(Pipeline *pipeline, const char *input_file);
void pipeline_set_output_file(Pipeline *pipeline, const char *output_file);
void pipeline_free(Pipeline *pipeline);

#endif