#ifndef AST_H
#define AST_H

#include <stddef.h>
#include "clist.h"  // Assuming CList is defined in clist.h

// Structure to represent a single command
typedef struct Command {
    char *command;         // The command itself (e.g., "cat")
    CList arguments;       // A linked list of arguments
    char **args;           // List of arguments (e.g., ["file1.txt"])
    int arg_count;         // The count of arguments
    struct Command *next;  // Pointer to the next command in the pipeline
} Command;

// Structure to represent the entire pipeline
typedef struct Pipeline {
    Command *command;      // A command in the pipeline
    struct Pipeline *next; // Pointer to the next command in the pipeline
    char *input_file;      // Input file for the pipeline (if any)
    char *output_file;     // Output file for the pipeline
} Pipeline;

// Abstract Syntax Tree (AST) node
typedef struct ASTNode {
    Pipeline *pipeline;    // Pointer to the pipeline
    char *command;         // Valid if type == COMMAND_NODE
    char **args;           // Arguments for the command
    struct ASTNode *next;  // Pointer to the next AST node
} ASTNode;

// Function prototypes
Command *create_command();                  // Create a new command
void add_argument_to_command(Command *cmd, const char *arg);  // Add an argument to a command
void add_command_to_pipeline(Pipeline *pipeline, Command *cmd);  // Add a command to the pipeline
void set_input_file(Pipeline *pipeline, const char *input_file);  // Set input file for the pipeline
void set_output_file(Pipeline *pipeline, const char *output_file);  // Set output file for the pipeline

#endif // AST_H
