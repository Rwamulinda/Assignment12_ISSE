// ast.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

// Function to create a new command
Command *create_command()
{
    Command *cmd = malloc(sizeof(Command));
    if (!cmd)
    {
        perror("Failed to allocate memory for command");
        exit(EXIT_FAILURE);
    }
    cmd->args = NULL;
    cmd->arg_count = 0; // Initialize argument count to 0
    cmd->next = NULL;
    return cmd;
}

// Function to add an argument to a command
void add_argument_to_command(Command *cmd, const char *arg)
{
    if (!cmd)
        return; // Check for null command

    cmd->args = realloc(cmd->args, sizeof(char *) * (cmd->arg_count + 1));
    if (!cmd->args)
    {
        perror("Failed to allocate memory for command arguments");
        exit(EXIT_FAILURE);
    }

    cmd->args[cmd->arg_count] = strdup(arg); // Allocate and copy the argument string
    if (!cmd->args[cmd->arg_count])
    {
        perror("Failed to duplicate argument string");
        exit(EXIT_FAILURE);
    }

    cmd->arg_count++;
}

// Function to add a command to the pipeline
void add_command_to_pipeline(Pipeline *pipeline, Command *cmd)
{
    if (!pipeline || !cmd)
        return; // Check for null pointers

    if (pipeline->command == NULL)
    {
        pipeline->command = cmd; // Set first command in pipeline
    }
    else
    {
        Command *last = pipeline->command;
        while (last->next != NULL)
        {
            last = last->next; // Traverse to the end of the command list
        }
        last->next = cmd; // Link new command at the end
    }
}

// Function to set the input file for the pipeline
void set_input_file(Pipeline *pipeline, const char *input_file)
{
    if (!pipeline || !input_file)
        return; // Check for null pointers

    if (pipeline->input_file == NULL)
    {
        pipeline->input_file = strdup(input_file); // Allocate and copy the input file path
        if (!pipeline->input_file)
        {
            perror("Failed to allocate memory for input file");
            exit(EXIT_FAILURE);
        }
    }
}

// Function to set the output file for the pipeline
void set_output_file(Pipeline *pipeline, const char *output_file)
{
    if (!pipeline || !output_file)
        return; // Check for null pointers

    if (pipeline->output_file == NULL)
    {
        pipeline->output_file = strdup(output_file); // Allocate and copy the output file path
        if (!pipeline->output_file)
        {
            perror("Failed to allocate memory for output file");
            exit(EXIT_FAILURE);
        }
    }
}

// Function to free allocated memory for a command
void free_command(Command *cmd)
{
    if (cmd)
    {
        for (int i = 0; i < cmd->arg_count; i++)
        {
            free(cmd->args[i]); // Free each argument string
        }
        free(cmd->args); // Free the arguments array
        free(cmd);       // Free the command structure itself
    }
}

// Function to free allocated memory for a pipeline
void free_pipeline(Pipeline *pipeline)
{
    while (pipeline != NULL)
    {
        Pipeline *next_pipeline = pipeline->next;

        free_command(pipeline->command); // Free the command in this pipeline

        // Free input and output file paths if they exist
        if (pipeline->input_file)
            free(pipeline->input_file);

        if (pipeline->output_file)
            free(pipeline->output_file);

        free(pipeline); // Free the pipeline structure itself

        pipeline = next_pipeline; // Move to next pipeline segment
    }
}
