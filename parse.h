#ifndef PARSE_H
#define PARSE_H

#include "clist.h"
#include "pipeline.h"

/**
 * Parse tokens into a pipeline structure
 * 
 * @param tokens List of tokens to parse
 * @param errmsg Buffer to store error message if parsing fails
 * @param errmsg_sz Size of error message buffer
 * @return Pointer to created Pipeline, or NULL if parsing fails
 */
Pipeline* parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz);

#endif // PARSE_H
