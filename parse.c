// parse.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h> // Include glob for wildcard expansion
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
    int pipe_count = 0;
    int redirection_count = 0;

    // Reset error message buffer
    if (errmsg)
        errmsg[0] = '\0';

    // Validate input tokens
    if (CL_length(tokens) == 0)
    {
        snprintf(errmsg, errmsg_sz, "No tokens to parse");
        return NULL;
    }

    while (CL_length(tokens) > 0)
    {
        Token token = TOK_next(tokens);
        TOK_consume(tokens);

        if (token.type == TOK_WORD || token.type == TOK_QUOTED_WORD)
        {
            if (current_command == NULL)
            {
                current_command = create_command();
            }
            add_argument_to_command(current_command, token.value);

            // Handle globbing for wildcard expansion
            if (strchr(token.value, '*') || strchr(token.value, '?') ||
                strchr(token.value, '~') || strchr(token.value, '[') ||
                strchr(token.value, ']'))
            {
                glob_t globbuf;
                int glob_result = glob(token.value, GLOB_TILDE_CHECK, NULL, &globbuf);
                if (glob_result == 0)
                {
                    for (size_t i = 0; i < globbuf.gl_pathc; i++)
                    {
                        add_argument_to_command(current_command, globbuf.gl_pathv[i]);
                    }
                    globfree(&globbuf);
                }
            }
        }
        else if (token.type == TOK_PIPE)
        {
            pipe_count++;

            // Ensure commands exist before and after pipe
            if (current_command == NULL)
            {
                snprintf(errmsg, errmsg_sz, "No command specified before pipe");
                free_pipeline(pipeline);
                return NULL;
            }

            Pipeline *new_pipeline = malloc(sizeof(Pipeline));
            if (new_pipeline == NULL)
            {
                snprintf(errmsg, errmsg_sz, "Memory allocation error for pipeline");
                free_pipeline(pipeline);
                return NULL;
            }

            new_pipeline->command = current_command;
            new_pipeline->next = NULL;
            new_pipeline->input_file = NULL;
            new_pipeline->output_file = NULL;

            if (pipeline == NULL)
            {
                pipeline = new_pipeline;
            }
            else
            {
                current_pipeline->next = new_pipeline;
            }

            current_pipeline = new_pipeline;
            current_command = NULL;
        }
        else if (token.type == TOK_LESSTHAN || token.type == TOK_GREATERTHAN)
        {
            redirection_count++;

            // Prevent multiple redirections
            if (redirection_count > 1)
            {
                snprintf(errmsg, errmsg_sz, "Multiple redirections not allowed");
                free_pipeline(pipeline);
                return NULL;
            }

            if (CL_length(tokens) == 0)
            {
                snprintf(errmsg, errmsg_sz, "Expected filename after redirection");
                free_pipeline(pipeline);
                return NULL;
            }

            Token file_token = TOK_next(tokens);
            TOK_consume(tokens);

            if (token.type == TOK_LESSTHAN && current_pipeline != NULL)
            {
                set_input_file(current_pipeline, file_token.value);
            }
            else if (token.type == TOK_GREATERTHAN && current_pipeline != NULL)
            {
                set_output_file(current_pipeline, file_token.value);
            }
        }
        else
        {
            snprintf(errmsg, errmsg_sz, "Unexpected token: %s", token.value);
            free_pipeline(pipeline);
            return NULL;
        }
    }

    if (current_command != NULL)
    {
        Pipeline *new_pipeline = malloc(sizeof(Pipeline));
        if (new_pipeline == NULL)
        {
            snprintf(errmsg, errmsg_sz, "Memory allocation error for final pipeline");
            free_pipeline(pipeline);
            return NULL;
        }

        new_pipeline->command = current_command;
        new_pipeline->next = NULL;
        new_pipeline->input_file = NULL;
        new_pipeline->output_file = NULL;

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
