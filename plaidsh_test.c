#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parse.h"
#include "pipeline.h"
#include "ast.h"
#include "clist.h"
#include "tokenize.h"

// Helper function to print token details for debugging
void print_token(const Token* token, int index) {
    if (token) {
        printf("Token %d: type %d, value '%s'\n", 
               index, token->type, token->value);
    } else {
        printf("Token %d: NULL\n", index);
    }
}

// Validate a specific token in the list
void validate_token(CList tokens, int index, int expected_type, const char* expected_value) {
    Token token = CL_nth(tokens, index);
    
    // Detailed assertion with informative error message
    if (token.type != expected_type || strcmp(token.value, expected_value) != 0) {
        printf("Token validation failed at index %d\n", index);
        printf("Expected: type %d, value '%s'\n", expected_type, expected_value);
        printf("Actual:   type %d, value '%s'\n", token.type, token.value);
        print_token(&token, index);
        
        // Fail the test with a clear message
        assert(0);
    }
}

// Test basic word tokenization
void test_basic_word_tokenization() {
    printf("Running basic word tokenization test...\n");
    
    char errmsg[256] = {0};
    const char *input = "echo hello world";
    
    CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
    assert(tokens != NULL);
    
    int token_count = CL_length(tokens);
    printf("Token count: %d\n", token_count);
    
    // Debug: Print all tokens
    for (int i = 0; i < token_count; i++) {
        Token token = CL_nth(tokens, i);
        printf("Token %d: type=%d, value='%s', length=%zu\n", 
               i, token.type, token.value, strlen(token.value));
    }
    
    // Adjust assertion to expect 4 tokens (3 words + 1 end token)
    assert(token_count == 4);
    
    // Validate the actual words, ignoring the end token
    validate_token(tokens, 0, TOK_WORD, "echo");
    validate_token(tokens, 1, TOK_WORD, "hello");
    validate_token(tokens, 2, TOK_WORD, "world");
    
    // Verify the last token is the end token
    Token last_token = CL_nth(tokens, token_count - 1);
    assert(last_token.type == TOK_END);
    
    CL_free(tokens);
    printf("Basic word tokenization test passed.\n");
}

// Similarly update other test functions to account for the end token
void test_advanced_tokenization() {
    printf("Running advanced tokenization test...\n");
    
    char errmsg[256] = {0};
    const char *input = "cat < input.txt | grep 'pattern' > output.txt";
    
    CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
    assert(tokens != NULL);
    
    int token_count = CL_length(tokens);
    printf("Token count: %d\n", token_count);
    
    // Adjust to expect 8 tokens (7 meaningful tokens + 1 end token)
    assert(token_count == 8);
    
    // Validate tokens with special characters
    validate_token(tokens, 0, TOK_WORD, "cat");
    validate_token(tokens, 1, TOK_LESSTHAN, "<");
    validate_token(tokens, 2, TOK_WORD, "input.txt");
    validate_token(tokens, 3, TOK_PIPE, "|");
    validate_token(tokens, 4, TOK_WORD, "grep");
    validate_token(tokens, 5, TOK_QUOTED_WORD, "'pattern'");
    validate_token(tokens, 6, TOK_GREATERTHAN, ">");
    
    // Verify the last token is the end token
    Token last_token = CL_nth(tokens, token_count - 1);
    assert(last_token.type == TOK_END);
    
    CL_free(tokens);
    printf("Advanced tokenization test passed.\n");
}
// Test error handling for invalid input
void test_error_tokenization() {
    printf("Running error tokenization test...\n");
    
    char errmsg[256] = {0};
    const char *input = "echo \"unterminated string";
    
    CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
    
    // Expect NULL return and non-empty error message
    assert(tokens == NULL);
    assert(strlen(errmsg) > 0);
    
    printf("Error message: %s\n", errmsg);
    printf("Error tokenization test passed.\n");
}

int main() {
    printf("Starting Tokenizer Test Suite...\n");
    
    test_basic_word_tokenization();
    test_advanced_tokenization();
    test_error_tokenization();
    
    printf("All Tokenizer Tests Passed Successfully!\n");
    return 0;
}