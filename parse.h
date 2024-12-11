#ifndef PARSE_H
#define PARSE_H

#include "Tokenize.h"
#include "pipeline.h"


// Representation of a single command
typedef struct Command
{
    char *command;   // The command (e.g., "cat")
    CList arguments; // List of arguments (e.g., ["file1.txt"])
} Command;

// Representation of the entire pipeline
typedef struct Pipeline
{
    Command *command;      // A command
    struct Pipeline *next; // Next command in the pipeline
} Pipeline;

// Function to parse a list of tokens into a pipeline
Pipeline *parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz);

// Helper functions
void free_pipeline(Pipeline *pipeline);

#endif
