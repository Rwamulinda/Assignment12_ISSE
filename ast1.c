#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Function to create a new command
Command *create_command() {
    Command *cmd = malloc(sizeof(Command));
    cmd->args = NULL;
    cmd->arg_count = 1;
    cmd->next = NULL;
    return cmd;
}

// Function to add an argument to a command
void add_argument_to_command(Command *cmd, const char *arg) {
    cmd->args = realloc(cmd->args, sizeof(char *) * (cmd->arg_count + 1));
    cmd->args[cmd->arg_count - 1] = strdup(arg);  // Allocate and copy the argument string
    cmd->args[cmd->arg_count] = NULL;
    cmd->arg_count++;
}

// Function to add a command to the pipeline
void add_command_to_pipeline(Pipeline *pipeline, Command *cmd) {
    if (pipeline->command == NULL) {
        pipeline->command = cmd;
    } else {
        Command *last = pipeline->command;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = cmd;
    }
}

// Function to set the input file for the pipeline
void set_input_file(Pipeline *pipeline, const char *input_file) {
    if (pipeline->input_file == NULL) {
        pipeline->input_file = strdup(input_file);  // Allocate and copy the input file path
    }
}

// Function to set the output file for the pipeline
void set_output_file(Pipeline *pipeline, const char *output_file) {
    if (pipeline->output_file == NULL) {
        pipeline->output_file = strdup(output_file);  // Allocate and copy the output file path
    }
}
