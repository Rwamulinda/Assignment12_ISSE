#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "clist.h"
#include "Token.h"
#include "Tokenize.h"
#include "pipeline.h"

// Function prototypes
Pipeline* parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz);
int execute_pipeline(Pipeline *pipeline);

int main() {
    printf("Welcome to Plaid Shell!\n");
    CList tokens = NULL;
    char errmsg[256];

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

        // Tokenize input
        tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
        if (tokens == NULL) {
            fprintf(stderr, "%s\n", errmsg);
            free(input);
            continue;
        }

        // Parse tokens into pipeline
        Pipeline *pipeline = parse_tokens(tokens, errmsg, sizeof(errmsg));
        if (pipeline == NULL) {
            fprintf(stderr, "%s\n", errmsg);
            CL_free(tokens);
            free(input);
            continue;
        }

        // Execute pipeline
        execute_pipeline(pipeline);

        // Free the allocated memory
        free(input);
        CL_free(tokens);
        pipeline_free(pipeline);
        tokens = NULL;
    }

    return 0;
}