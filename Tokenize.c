/*
 * tokenize.c
 *
 * Functions to tokenize and manipulate lists of tokens
 *
 * Author: <Uwase Pauline>
 */

#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "clist.h"
#include "Tokenize.h"
#include "Token.h"
#include <stddef.h>

// Documented in .h file
const char *TT_to_str(TokenType tt)
{
    switch (tt)
    {

    case TOK_WORD:
        return "WORD";
    case TOK_QUOTED_WORD:
        return "QUOTED_WORD";
    case TOK_LESSTHAN:
        return "LESSTHAN";
    case TOK_GREATERTHAN:
        return "GREATERTHAN";
    case TOK_PIPE:
        return "PIPE";
    case TOK_END:
        return "(end)";
    default:
        return "UNKNOWN";
    }

    __builtin_unreachable();
}

char handle_escape_sequence(char next_char, char *errmsg, size_t errmsg_sz)
{
    switch (next_char)
    {
    case 'n':
        return '\n'; // New line
    case 'r':
        return '\r'; // Carriage return
    case 't':
        return '\t'; // Tab
    case '"':
        return '\"'; // Double quote
    case '\\':
        return '\\'; // Backslash
    case ' ':
        return ' '; // Space
    case '|':
        return '|'; // Pipe
    case '<':
        return '<'; // Less than
    case '>':
        return '>'; // Greater than
    default:
        snprintf(errmsg, errmsg_sz, "Unrecognized escape sequence: \\%c", next_char);
        return '\0'; // Indicate an error
    }
}

// Documented in .h file
CList TOK_tokenize_input(const char *input, char *errmsg, size_t errmsg_sz)
{
    CList tokens = CL_new();
    size_t i = 0;

    while (input[i] != '\0')
    {
        if (isspace(input[i]))
        {
            i++; // Skip whitespace
            continue;
        }

        Token token;
        token.value = NULL; // Initialize to NULL
         // If the input is empty or contains only whitespace, return with no tokens
        if (input[i] == '\0') {
        snprintf(errmsg, errmsg_sz, "No tokens found");
        return tokens; // Return the empty list to signify no tokens
        }
        // Handle words (including escaped characters)
        if (isalnum(input[i]) || input[i] == '-' || input[i] == '_')
        {
            //size_t start = i;
            char temp[256]; // Temporary buffer for token value
            size_t temp_idx = 0;

            while (isalnum(input[i]) || input[i] == '-' || input[i] == '_' || input[i] == '\\')
            {
                if (input[i] == '\\' && input[i + 1] != '\0')
                {
                    // Process escape sequence
                    char escaped = handle_escape_sequence(input[++i], errmsg, errmsg_sz);
                    if (escaped == '\0')
                    {
                        // Error in escape sequence
                        CL_free(tokens);
                        return NULL;
                    }
                    temp[temp_idx++] = escaped;
                }
                else
                {
                    temp[temp_idx++] = input[i];
                }
                i++;
            }

            temp[temp_idx] = '\0'; // Null-terminate the temporary string
            token.type = TOK_WORD;
            token.value = malloc(strlen(temp) + 1); // Allocate memory for the token value
            if (!token.value)
            {
                snprintf(errmsg, errmsg_sz, "Memory allocation failed");
                CL_free(tokens);
                return NULL;
            }
            strcpy(token.value, temp); // Copy temp into dynamically allocated token.value
        }
        // Handle quoted words
        else if (input[i] == '"')
        {
            size_t start = ++i;
            char temp[256]; // Temporary buffer for quoted token value
            size_t temp_idx = 0;

            while (input[i] != '"' && input[i] != '\0')
            {
                if (input[i] == '\\' && input[i + 1] != '\0')
                {
                    // Process escape sequence inside quoted string
                    char escaped = handle_escape_sequence(input[++i], errmsg, errmsg_sz);
                    if (escaped == '\0')
                    {
                        // Error in escape sequence
                        CL_free(tokens);
                        return NULL;
                    }
                    temp[temp_idx++] = escaped;
                }
                else
                {
                    temp[temp_idx++] = input[i];
                }
                i++;
            }

            if (input[i] == '\0')
            {
                snprintf(errmsg, errmsg_sz, "Position %zu: Missing closing quote", start);
                CL_free(tokens);
                return NULL;
            }

            temp[temp_idx] = '\0'; // Null-terminate the temporary string
            token.type = TOK_QUOTED_WORD;
            token.value = malloc(strlen(temp) + 1); // Allocate memory for the token value
            if (!token.value)
            {
                snprintf(errmsg, errmsg_sz, "Memory allocation failed");
                CL_free(tokens);
                return NULL;
            }
            strcpy(token.value, temp); // Copy temp into dynamically allocated token.value
            i++;                       // Skip closing quote
        }
        // Handle redirection and pipe characters
        else
        {
            switch (input[i])
            {
            case '<':
                token.type = TOK_LESSTHAN;
                break;
            case '>':
                token.type = TOK_GREATERTHAN;
                break;
            case '|':
                token.type = TOK_PIPE;
                break;
            default:
                snprintf(errmsg, errmsg_sz, "Position %zu: unexpected character %c", i + 1, input[i]);
                CL_free(tokens);
                return NULL;
            }
            token.value = NULL; // Redirection and pipe tokens don't have a value
            i++;
        }

        // Add token to the list
        CL_append(tokens, token);
    }

    // Add end-of-input token
    Token end_token = {.type = TOK_END, .value = NULL};
    CL_append(tokens, end_token);

    return tokens;
}

// Documented in .h file
TokenType TOK_next_type(CList tokens)
{
    if (CL_length(tokens) == 0)
    {
        return TOK_END;
    }
    Token token = CL_nth(tokens, 0);
    return token.type;
}

// Documented in .h file
Token TOK_next(CList tokens)
{
    return CL_nth(tokens, 0);
}

// Documented in .h file
void TOK_consume(CList tokens)
{
    if (CL_length(tokens) > 0)
    {
        CL_pop(tokens);
    }
}

void printToken(int pos, CListElementType element, void *cb_data)
{
    if (element.type == TOK_WORD || element.type == TOK_QUOTED_WORD)
    {
        printf("Position %d: Token type: %s, Text: %s\n", pos, TT_to_str(element.type), element.value);
    }
    else
    {
        printf("Position %d: Token type: %s\n", pos, TT_to_str(element.type));
    }
}

// Documented in .h file
void TOK_print(CList tokens)
{
    CL_foreach(tokens, printToken, NULL);
}
