#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "clist.h"
#include "Token.h"
#include "Tokenize.h"
#include "pipeline.h"
#include "parse.h"
#include "ast.h"

int main() {
    printf("Welcome to Plaid Shell!\n");
    char errmsg[256]; // Buffer for error messages

    while (1) {
        // Display the prompt
        char *input = readline("#? ");

        // Check for EOF (Ctrl+D)
        if (!input) {
            printf("\nExiting Plaid Shell. Goodbye!\n");
            break;
        }

        // Add the command to history
        if (*input) {
            add_history(input);
        }

        // Tokenize the input
        CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
        if (tokens == NULL) {
            fprintf(stderr, "Tokenization error: %s\n", errmsg);
            free(input);
            continue;
        }

        // Parse tokens into a pipeline
        Pipeline *pipeline = parse_tokens(tokens, errmsg, sizeof(errmsg));
        if (pipeline == NULL) {
            fprintf(stderr, "Parsing error: %s\n", errmsg);
            CL_free(tokens);
            free(input);
            continue;
        }

        // Execute the pipeline (void return type)
        execute_pipeline(pipeline, errmsg, sizeof(errmsg));

        // Check if any error message was set
        if (errmsg[0] != '\0') {
            fprintf(stderr, "Execution error: %s\n", errmsg);
        }

        // Free allocated memory
        free(input);
        CL_free(tokens);
        free_pipeline(pipeline);
    }

    return 0;
}
