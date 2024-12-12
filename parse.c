#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>
#include "parse.h"
#include "Tokenize.h"
#include "pipeline.h"
#include "ast.h"
#include "clist.h"

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
            // Check if the token is a wildcard (e.g., *.c)
            if (strchr(token.value, '*') && strchr(token.value, '.'))
            {
                glob_t globbuf;
                int glob_result = glob(token.value, GLOB_TILDE_CHECK, NULL, &globbuf);
                
                if (glob_result == 0) // Glob was successful
                {
                    for (size_t i = 0; i < globbuf.gl_pathc; i++)
                    {
                        Token glob_token = {
                            .type = TOK_WORD,
                            .value = strdup(globbuf.gl_pathv[i])
                        };
                        
                        // Add glob result as an argument
                        CL_append(current_command->arguments, glob_token);
                    }
                    globfree(&globbuf);
                }
                else
                {
                    // No match found, add original token
                    if (current_command == NULL)
                    {
                        current_command = (Command *)malloc(sizeof(Command));
                        current_command->command = strdup(token.value);
                        current_command->arguments = CL_new();
                    }
                    else
                    {
                        CL_append(current_command->arguments, token);
                    }
                }
            }
            else
            {
                // Normal word token
                if (current_command == NULL)
                {
                    current_command = (Command *)malloc(sizeof(Command));
                    current_command->command = strdup(token.value);
                    current_command->arguments = CL_new();
                }
                else
                {
                    CL_append(current_command->arguments, token);
                }
            }
        }
        // If token is a pipe, connect the command and start a new command
        else if (token.type == TOK_PIPE)
        {
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
                current_pipeline = new_pipeline;
            }
            current_command = NULL;
        }
        // Handle redirection (if needed)
        else if (token.type == TOK_LESSTHAN || token.type == TOK_GREATERTHAN)
        {
            // Handle input/output redirection logic here
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
