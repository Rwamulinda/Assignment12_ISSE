#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "clist.h"
#include "Token.h"

int main() {
    printf("Welcome to Plaid Shell!\n");

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

        // Print the input back for debugging
        printf("You entered: %s\n", input);

        // Free the allocated memory
        free(input);
    }

    return 0;
}
