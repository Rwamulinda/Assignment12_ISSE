#ifndef AST_H
#define AST_H

// Structure to represent a single command
typedef struct Command {
    char **args;           // Array of command arguments (strings)
    int arg_count;         // Number of arguments
    struct Command *next;  // Pointer to the next command in the pipeline (for piped commands)
} Command;

// Structure to represent the entire pipeline
typedef struct Pipeline {
    Command *commands;     // Linked list of commands in the pipeline
    char *input_file;      // Input file (default is stdin)
    char *output_file;     // Output file (default is stdout)
} Pipeline;

// Function prototypes for managing the pipeline and commands
Command *create_command();                  // Create a new command
void add_argument_to_command(Command *cmd, const char *arg);  // Add an argument to a command
void add_command_to_pipeline(Pipeline *pipeline, Command *cmd);  // Add a command to the pipeline
void set_input_file(Pipeline *pipeline, const char *input_file);  // Set input file for the pipeline
void set_output_file(Pipeline *pipeline, const char *output_file);  // Set output file for the pipeline

#endif // AST_H
