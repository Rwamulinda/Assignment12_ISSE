#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "parse.h"
#include "pipeline.h"
#include "ast.h"
#include "clist.h"
#include "tokenize.h"

// Test case tracking and reporting
int total_tests = 0;
int passed_tests = 0;
int failed_tests = 0;

// Macro to simplify test reporting
#define RUN_TEST(test_func) do { \
    total_tests++; \
    printf("\n[TEST %d] Running %s...\n", total_tests, #test_func); \
    int result = test_func(); \
    if (result == 0) { \
        passed_tests++; \
        printf("[PASS] %s\n", #test_func); \
    } else { \
        failed_tests++; \
        printf("[FAIL] %s\n", #test_func); \
    } \
} while(0)

// Helper function to print token details for debugging
void print_token(const Token* token, int index) {
    if (token) {
        printf("Token %d: type %d, value '%s'\n", 
               index, token->type, token->value);
    } else {
        printf("Token %d: NULL\n", index);
    }
}

// Safe token validation with more robust error handling
int validate_token(CList tokens, int index, int expected_type, const char* expected_value) {
    if (index < 0 || index >= CL_length(tokens)) {
        printf("ERROR: Token index %d out of bounds\n", index);
        return -1;
    }
    
    Token token = CL_nth(tokens, index);
    
    if (token.type != expected_type || strcmp(token.value, expected_value) != 0) {
        printf("Token validation failed at index %d\n", index);
        printf("Expected: type %d, value '%s'\n", expected_type, expected_value);
        printf("Actual:   type %d, value '%s'\n", token.type, token.value);
        print_token(&token, index);
        
        return -1;
    }
    
    return 0;
}

// Test basic word tokenization
int test_basic_word_tokenization() {
    char errmsg[256] = {0};
    const char *input = "echo hello world";
    
    CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
    if (tokens == NULL) {
        printf("Tokenization failed: %s\n", errmsg);
        return -1;
    }
    
    int token_count = CL_length(tokens);
    printf("Token count: %d\n", token_count);
    
    // Debug: Print all tokens
    for (int i = 0; i < token_count; i++) {
        Token token = CL_nth(tokens, i);
        printf("Token %d: type=%d, value='%s', length=%zu\n", 
               i, token.type, token.value, strlen(token.value));
    }
    
    // Validate tokens
    if (token_count != 4) {
        printf("Expected 4 tokens, got %d\n", token_count);
        CL_free(tokens);
        return -1;
    }
    
    if (validate_token(tokens, 0, TOK_WORD, "echo") != 0) return -1;
    if (validate_token(tokens, 1, TOK_WORD, "hello") != 0) return -1;
    if (validate_token(tokens, 2, TOK_WORD, "world") != 0) return -1;
    
    // Check end token
    Token last_token = CL_nth(tokens, token_count - 1);
    if (last_token.type != TOK_END) {
        printf("Last token is not END token\n");
        CL_free(tokens);
        return -1;
    }
    
    CL_free(tokens);
    return 0;
}

// Advanced tokenization test
int test_advanced_tokenization() {
    char errmsg[256] = {0};
    const char *input = "cat < input.txt | grep 'pattern' > output.txt";
    
    CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
    if (tokens == NULL) {
        printf("Tokenization failed: %s\n", errmsg);
        return -1;
    }
    
    int token_count = CL_length(tokens);
    printf("Token count: %d\n", token_count);
    
    if (token_count != 8) {
        printf("Expected 8 tokens, got %d\n", token_count);
        CL_free(tokens);
        return -1;
    }
    
    // Validate specific tokens
    if (validate_token(tokens, 0, TOK_WORD, "cat") != 0) return -1;
    if (validate_token(tokens, 1, TOK_LESSTHAN, "<") != 0) return -1;
    if (validate_token(tokens, 2, TOK_WORD, "input.txt") != 0) return -1;
    if (validate_token(tokens, 3, TOK_PIPE, "|") != 0) return -1;
    if (validate_token(tokens, 4, TOK_WORD, "grep") != 0) return -1;
    if (validate_token(tokens, 5, TOK_QUOTED_WORD, "'pattern'") != 0) return -1;
    if (validate_token(tokens, 6, TOK_GREATERTHAN, ">") != 0) return -1;
    
    // Check end token
    Token last_token = CL_nth(tokens, token_count - 1);
    if (last_token.type != TOK_END) {
        printf("Last token is not END token\n");
        CL_free(tokens);
        return -1;
    }
    
    CL_free(tokens);
    return 0;
}

// Test error handling for invalid input
int test_error_tokenization() {
    char errmsg[256] = {0};
    const char *input = "echo \"unterminated string";
    
    CList tokens = TOK_tokenize_input(input, errmsg, sizeof(errmsg));
    
    // Expect NULL return and non-empty error message
    if (tokens != NULL) {
        printf("Expected NULL tokens for invalid input\n");
        CL_free(tokens);
        return -1;
    }
    
    if (strlen(errmsg) == 0) {
        printf("Expected non-empty error message\n");
        return -1;
    }
    
    printf("Error message: %s\n", errmsg);
    return 0;
}

int main() {
    printf("Starting Tokenizer Test Suite...\n");
    
    RUN_TEST(test_basic_word_tokenization);
    RUN_TEST(test_advanced_tokenization);
    RUN_TEST(test_error_tokenization);
    
    printf("\nTest Summary:\n");
    printf("Total Tests:  %d\n", total_tests);
    printf("Passed Tests: %d\n", passed_tests);
    printf("Failed Tests: %d\n", failed_tests);
    
    return failed_tests > 0 ? -1 : 0;
}