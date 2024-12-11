#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "clist.h"
#include "Token.h"
#include "Tokenize.h"

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

        // Tokenize the input
        tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));

        // If tokens is NULL, skip printing and move to the next loop iteration
        if (tokens != NULL) {
            TOK_print(tokens);
        } else {
            printf("Error: %s\n", errmsg);  // Optionally print the error message if tokenization failed
        }

        // Free the allocated memory for input
        free(input);

    }

    return 0;
}
