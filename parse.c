#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse.h"
#include "Tokenize.h"
#include "pipeline.h"
#include "ast.h"

Pipeline *parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz)
{
    Pipeline *pipeline = NULL;
    Pipeline *current_pipeline = NULL;
    Command *current_command = NULL;

    while (CL_length(tokens) > 0)
    {
        Token token = TOK_next(tokens);
        TOK_consume(tokens);

        // If token is a word or quoted word, it's part of a command
        if (token.type == TOK_WORD || token.type == TOK_QUOTED_WORD)
        {
            // Start a new command if this is the first token
            if (current_command == NULL)
            {
                current_command = (Command *)malloc(sizeof(Command));
                current_command->command = strdup(token.value);
                current_command->arguments = CL_new();
            }
            else
            {
                // Add token to arguments of the current command
                CL_append(current_command->arguments, token);
            }
        }
        // If token is a pipe, connect the command and start a new command
        else if (token.type == TOK_PIPE)
        {
            if (current_command != NULL)
            {
                // Add the current command to the pipeline
                Pipeline *new_pipeline = (Pipeline *)malloc(sizeof(Pipeline));
                new_pipeline->command = current_command;
                new_pipeline->next = NULL;

                if (pipeline == NULL)
                {
                    pipeline = new_pipeline;
                }
                else
                {
                    current_pipeline->next = new_pipeline;
                }
                current_pipeline = new_pipeline;
            }
            current_command = NULL; // Reset current command for the next part of the pipeline
        }
        // Handle redirection (if needed)
        else if (token.type == TOK_LESSTHAN || token.type == TOK_GREATERTHAN)
        {
            // Handle input/output redirection logic here
            // For example, associate redirection token with the current command
        }
    }

    // Finalize the last command in the pipeline
    if (current_command != NULL)
    {
        Pipeline *new_pipeline = (Pipeline *)malloc(sizeof(Pipeline));
        new_pipeline->command = current_command;
        new_pipeline->next = NULL;

        if (pipeline == NULL)
        {
            pipeline = new_pipeline;
        }
        else
        {
            current_pipeline->next = new_pipeline;
        }
    }

    return pipeline;
}

void free_pipeline(Pipeline *pipeline)
{
    while (pipeline != NULL)
    {
        Pipeline *next = pipeline->next;
        free(pipeline->command->command);
        CL_free(pipeline->command->arguments);
        free(pipeline->command);
        free(pipeline);
        pipeline = next;
    }
}
