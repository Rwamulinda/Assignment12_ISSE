#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parse.h"
#include "pipeline.h"
#include "ast.h"
#include "clist.h"
#include "tokenize.h"

void test_TOK_tokenize_input() {
    char errmsg[256];

    // Test Case 1: Valid input with simple tokens
    const char *input1 = "echo hello world";
    CList tokens1 = TOK_tokenize_input(input1, errmsg, sizeof(errmsg));
    assert(tokens1 != NULL);
    assert(CL_length(tokens1) == 3); // Expect 3 tokens: "echo", "hello", "world"

    Token token = CL_nth(tokens1, 0);
    assert(token.type == TOK_WORD);
    assert(strcmp(token.value, "echo") == 0);

    token = CL_nth(tokens1, 1);
    assert(token.type == TOK_WORD);
    assert(strcmp(token.value, "hello") == 0);

    token = CL_nth(tokens1, 2);
    assert(token.type == TOK_WORD);
    assert(strcmp(token.value, "world") == 0);

    CL_free(tokens1);

    // Test Case 2: Input with special characters
    const char *input2 = "cat < input.txt | grep 'pattern' > output.txt";
    CList tokens2 = TOK_tokenize_input(input2, errmsg, sizeof(errmsg));
    assert(tokens2 != NULL);
    assert(CL_length(tokens2) == 7); // Expect tokens for "cat", "<", "input.txt", "|", "grep", "'pattern'", ">"

    token = CL_nth(tokens2, 1);
    assert(token.type == TOK_LESSTHAN);
    assert(strcmp(token.value, "<") == 0);

    CL_free(tokens2);

    // Test Case 3: Invalid input
    const char *input3 = "echo \"unterminated string";
    CList tokens3 = TOK_tokenize_input(input3, errmsg, sizeof(errmsg));
    assert(tokens3 == NULL);
    assert(strlen(errmsg) > 0); // Expect an error message

    printf("All tokenizer tests passed.\n");
}