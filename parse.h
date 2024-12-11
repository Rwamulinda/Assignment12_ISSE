#ifndef PARSE_H
#define PARSE_H

#include "Tokenize.h"
#include "ast.h"  // Include ast.h to use the Command and Pipeline structs

// Function to parse a list of tokens into a pipeline
Pipeline *parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz);

// Helper functions
void free_pipeline(Pipeline *pipeline);

#endif // PARSE_H
