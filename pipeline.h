// pipeline.h
#ifndef PIPELINE_H
#define PIPELINE_H

#include <stdlib.h>

// Command structure to hold individual command details
typedef struct Command {
    char **args;          // Array of arguments
    int arg_count;        // Number of arguments
    int arg_capacity;     // Allocated argument capacity
} Command;

// Pipeline node for linked list representation
typedef struct PipelineNode {
    Command *command;               // Current command
    struct PipelineNode *next;      // Next command in pipeline
} PipelineNode;

// Main Pipeline structure
typedef struct {
    PipelineNode *head;             // First command in pipeline
    PipelineNode *tail;              // Last command in pipeline
    int command_count;               // Total number of commands
    
    char *input_file;                // Input file for pipeline
    char *output_file;               // Output file for pipeline
} Pipeline;

// Function prototypes
Pipeline *pipeline_create();
void pipeline_destroy(Pipeline *pipeline);
int pipeline_add_command(Pipeline *pipeline, Command *command);
void pipeline_set_input_file(Pipeline *pipeline, const char *filename);
void pipeline_set_output_file(Pipeline *pipeline, const char *filename);
void pipeline_print(Pipeline *pipeline);  // Optional: for debugging
int pipeline_execute(Pipeline *pipeline);
#endif // PIPELINE_H
