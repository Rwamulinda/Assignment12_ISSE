#ifndef PIPELINE_H
#define PIPELINE_H

#include "ast.h"  // Assuming the ASTNode structure is declared in ast.h
#include <stdio.h>

// Function declarations
void execute_pipeline(Pipeline *pipeline, char *errmsg, size_t errmsg_size);
int handle_builtin_commands(char *command, char **args);
int handle_redirection(char **args)

#endif // PIPELINE_H
