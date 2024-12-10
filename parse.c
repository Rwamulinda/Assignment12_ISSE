#include <stdlib.h>
#include <string.h>
#include "clist.h"
#include "Token.h"
#include "pipeline.h"
#include "parse.h"

// Documented in .h file
Pipeline* parse_tokens(CList *tokens, char *errmsg, size_t errmsg_sz);

    // Initialize a new pipeline to hold commands
    Pipeline *pipeline = pipeline_create();
    if (!pipeline) {
        snprintf(errmsg, errmsg_sz, "Could not create pipeline");
        return NULL;
    }

    // Flags to check if input/output redirection is handled
    int input_redirected = 0;
    int output_redirected = 0;

    // Initialize a current command
    Command current_command = {0};
    current_command.args = malloc(sizeof(char*) * 10);  // Initial allocation
    current_command.arg_capacity = 10;
    current_command.arg_count = 0;

    // Loop through tokens and parse
    while (TOK_next_type(tokens) != TOK_END) {
        Token current_token = TOK_next(tokens);
        TokenType current_type = current_token.type;

        switch (current_type) {
            case TOK_WORD:
            case TOK_QUOTED_WORD:
                // Add argument to the current command
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
                TOK_consume(tokens);  // Consume '<'
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
                TOK_consume(tokens);  // Consume '>'
                if (TOK_next_type(tokens) != TOK_WORD) {
                    snprintf(errmsg, errmsg_sz, "Expected filename after output redirection");
                    return NULL;
                }
                pipeline_set_output_file(pipeline, TOK_next(tokens).value);
                output_redirected = 1;
                break;

            case TOK_PIPE:
                // Complete the current command and start a new one
                if (current_command.arg_count > 0) {
                    current_command.args[current_command.arg_count] = NULL;
                    pipeline_add_command(pipeline, &current_command);

                    // Reset the current command
                    memset(&current_command, 0, sizeof(Command));
                    current_command.args = malloc(sizeof(char*) * 10);
                    current_command.arg_capacity = 10;
                    current_command.arg_count = 0;
                }
                break;

            default:
                break;
        }

        // Consume the current token and move to the next one
        TOK_consume(tokens);
    }

    // Add the last command if it contains arguments
    if (current_command.arg_count > 0) {
        current_command.args[current_command.arg_count] = NULL;
        pipeline_add_command(pipeline, &current_command);
    }

    return pipeline;
}
