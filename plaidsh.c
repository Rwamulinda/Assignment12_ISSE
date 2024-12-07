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

        tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));          
        if(tokens == NULL) {
            fprintf(stderr, "%s\n", errmsg);
            free(input);  // Free input on tokenization failure
            continue;  // Go to next iteration
        }

        TOK_print(tokens);          

        // Free the allocated memory
        free(input);
        CL_free(tokens);
        tokens = NULL;
    }      

    return 0;
}