#include "pipeline.h"
#include <string.h>

Pipeline* pipeline_create() {
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    if (!pipeline) {
        return NULL;
    }
    
    pipeline->commands = NULL;
    pipeline->command_count = 0;
    pipeline->input_file = NULL;
    pipeline->output_file = NULL;
    pipeline->is_piped = 0;
    
    return pipeline;
}

void pipeline_add_command(Pipeline *pipeline, Command *command) {
    // Expand the commands array
    pipeline->commands = realloc(
        pipeline->commands, 
        (pipeline->command_count + 1) * sizeof(Command)
    );
    
    // Copy the command
    pipeline->commands[pipeline->command_count] = *command;
    pipeline->command_count++;
    
    // If more than one command, it's a pipeline
    if (pipeline->command_count > 1) {
        pipeline->is_piped = 1;
    }
}

void pipeline_set_input_file(Pipeline *pipeline, const char *input_file) {
    if (pipeline->input_file) {
        free(pipeline->input_file);
    }
    pipeline->input_file = strdup(input_file);
}

void pipeline_set_output_file(Pipeline *pipeline, const char *output_file) {
    if (pipeline->output_file) {
        free(pipeline->output_file);
    }
    pipeline->output_file = strdup(output_file);
}

void pipeline_free(Pipeline *pipeline) {
    if (!pipeline) return;
    
    // Free each command's arguments
    for (int i = 0; i < pipeline->command_count; i++) {
        Command *cmd = &pipeline->commands[i];
        for (int j = 0; j < cmd->arg_count; j++) {
            free(cmd->args[j]);
        }
        free(cmd->args);
    }
    
    // Free the commands array
    free(pipeline->commands);
    
    // Free input/output file strings
    free(pipeline->input_file);
    free(pipeline->output_file);
    
    // Free the pipeline itself
    free(pipeline);
}