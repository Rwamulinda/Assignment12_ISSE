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

        // Handle words (general tokens, like commands or arguments)
        if (isalnum(input[i]) || input[i] == '-' || input[i] == '_')
        {
            size_t start = i;
            while (isalnum(input[i]) || input[i] == '-' || input[i] == '_')
            {
                i++;
            }
            token.type = TOK_WORD;
            token.value = 0; // Words don't have a value like numbers
            strncpy(token.value, &input[start], i - start);
        }
        // Handle quoted words
        else if (input[i] == '"')
        {
            size_t start = ++i;
            while (input[i] != '"' && input[i] != '\0')
            {
                i++;
            }

            if (input[i] == '\0')
            {
                snprintf(errmsg, errmsg_sz, "Position %zu: Missing closing quote", start);
                CL_free(tokens);
                return NULL;
            }

            token.type = TOK_QUOTED_WORD;
            strncpy(token.value, &input[start], i - start);
            i++; // Skip closing quote
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
            token.value = 0; // Redirection and pipe tokens don't have a value
            i++;
        }

        // Add token to the list
        CL_append(tokens, token);
    }

    // Add end-of-input token
    Token end_token = {.type = TOK_END, .value = 0};
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

void printToken(int pos, CListElementType element, void* cb_data) {
    if (element.type == TOK_WORD || element.type == TOK_QUOTED_WORD) {
        printf("Position %d: Token type: %s, Text: %s\n", pos, TT_to_str(element.type), element.value);
    } else {
        printf("Position %d: Token type: %s\n", pos, TT_to_str(element.type));
    }
}


// Documented in .h file
void TOK_print(CList tokens)
{
    CL_foreach(tokens, printToken, NULL);
}
