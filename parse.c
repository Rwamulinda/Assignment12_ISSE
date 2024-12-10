#include <stdlib.h>
#include <string.h>
#include "clist.h"
#include "Token.h"
#include "pipeline.h"
#include "parse.h"

// The implementation of parse_tokens that we discussed earlier would go here
// Essentially the same code from the previous parser implementation artifact
// But now with the function signature matching parse.h

Pipeline* parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz) {
    Pipeline *pipeline = pipeline_create();
    if (!pipeline) {
        snprintf(errmsg, errmsg_sz, "Could not create pipeline");
        return NULL;
    }

    // Track input/output redirection
    int input_redirected = 0;
    int output_redirected = 0;

    Command current_command = {0};
    current_command.args = malloc(sizeof(char*) * 10);  // Initial allocation
    current_command.arg_capacity = 10;
    current_command.arg_count = 0;

    while (TOK_next_type(tokens) != TOK_END) {
        TokenType current_type = TOK_next_type(tokens);
        Token current_token = TOK_next(tokens);

        switch (current_type) {
            case TOK_WORD:
            case TOK_QUOTED_WORD:
                // Add argument to current command
                current_command.args[current_command.arg_count++] = strdup(current_token.value);
                
                // Reallocate if needed
                if (current_command.arg_count >= current_command.arg_capacity) {
                    current_command.arg_capacity *= 2;
                    current_command.args = realloc(current_command.args, 
                        sizeof(char*) * current_command.arg_capacity);
                }
                break;

            case TOK_LESSTHAN:
                // Input redirection
                if (input_redirected) {
                    snprintf(errmsg, errmsg_sz, "Multiple input redirections not allowed");
                    return NULL;
                }
                TOK_consume(tokens);
                if (TOK_next_type(tokens) != TOK_WORD) {
                    snprintf(errmsg, errmsg_sz, "Expected filename after input redirection");
                    return NULL;
                }
                pipeline_set_input_file(pipeline, TOK_next(tokens).value);
                input_redirected = 1;
                break;

            case TOK_GREATERTHAN:
                // Output redirection
                if (output_redirected) {
                    snprintf(errmsg, errmsg_sz, "Multiple output redirections not allowed");
                    return NULL;
                }
                TOK_consume(tokens);
                if (TOK_next_type(tokens) != TOK_WORD) {
                    snprintf(errmsg, errmsg_sz, "Expected filename after output redirection");
                    return NULL;
                }
                pipeline_set_output_file(pipeline, TOK_next(tokens).value);
                output_redirected = 1;
                break;

            case TOK_PIPE:
                // Complete current command and start a new one
                if (current_command.arg_count > 0) {
                    // Null-terminate args
                    current_command.args[current_command.arg_count] = NULL;
                    pipeline_add_command(pipeline, &current_command);
                    
                    // Reset current command
                    memset(&current_command, 0, sizeof(Command));
                    current_command.args = malloc(sizeof(char*) * 10);
                    current_command.arg_capacity = 10;
                    current_command.arg_count = 0;
                }
                break;

            default:
                break;
        }

        TOK_consume(tokens);
    }

    // Add the last command if not empty
    if (current_command.arg_count > 0) {
        current_command.args[current_command.arg_count] = NULL;
        pipeline_add_command(pipeline, &current_command);
    }

    return pipeline;
}