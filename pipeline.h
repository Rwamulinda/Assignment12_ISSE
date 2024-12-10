#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdio.h>
#include <stdlib.h>
#include "token.h"
#include "clist.h"  // Include circular linked list (CList) for token management
#include "parse.h"

// Represents a single command with its arguments
typedef struct {
    char **args;       // Dynamically allocated array of arguments
    int arg_count; 
    int arg_capacity;     // Number of arguments
    char *executable;  // Path or name of the executable
} Command;

// Represents the entire pipeline of commands
typedef struct {
    Command *commands;     // Dynamically allocated array of commands
    int command_count;     // Number of commands in the pipeline
    
    // Redirection support
    char *input_file;      // Input redirection file (optional, can be NULL)
    char *output_file;     // Output redirection file (optional, can be NULL)
    
    // Pipeline-specific flags
    int is_piped;          // Indicates whether commands are piped together
} Pipeline;

// Pipeline management functions
Pipeline* pipeline_create();                           // Create a new empty pipeline
void pipeline_add_command(Pipeline *pipeline, Command *command); // Add a command to the pipeline
void pipeline_set_input_file(Pipeline *pipeline, const char *input_file); // Set input redirection
void pipeline_set_output_file(Pipeline *pipeline, const char *output_file); // Set output redirection
void pipeline_free(Pipeline *pipeline);               // Free all resources used by the pipeline

// New functions for pipeline handling (updated for CList token handling)
Pipeline* parse_tokens(CList *tokens, char *errmsg, size_t errmsg_sz); // Parse tokens into a Pipeline structure
void execute_pipeline(Pipeline *pipeline);            // Execute the commands in the pipeline

#endif // PIPELINE_H
