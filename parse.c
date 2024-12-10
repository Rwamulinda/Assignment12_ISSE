#include <stdlib.h>
#include <string.h>
#include "clist.h"
#include "Token.h"
#include "pipeline.h"
#include "parse.h"
#include "Tokenize.h"

Pipeline *parse_tokens(CList *tokens, char *errmsg, size_t errmsg_sz)
{
  // Create a new pipeline
  Pipeline *pipeline = pipeline_create();
  if (!pipeline)
  {
    snprintf(errmsg, errmsg_sz, "Could not create pipeline");
    return NULL;
  }

  // Initialize a command to track current command being built
  Command current_command = {0};
  current_command.args = malloc(sizeof(char *) * 10); // Initial allocation
  current_command.arg_capacity = 10;
  current_command.arg_count = 0;

  // Flags to track input/output redirection
  int input_redirected = 0;
  int output_redirected = 0;

  // Loop through tokens until we reach the end
  while (TOK_next_type(tokens) != TOK_END)
  {
    Token current_token = TOK_next(tokens);
    TokenType current_type = current_token.type;

    switch (current_type)
    {
    case TOK_WORD:
    case TOK_QUOTED_WORD:
      // Add argument to the current command
      if (current_command.arg_count >= current_command.arg_capacity)
      {
        current_command.arg_capacity *= 2;
        current_command.args = realloc(current_command.args,
                                       sizeof(char *) * current_command.arg_capacity);
      }
      current_command.args[current_command.arg_count++] = strdup(current_token.value);
      break;

    case TOK_LESSTHAN:
      // Input redirection
      if (input_redirected)
      {
        snprintf(errmsg, errmsg_sz, "Multiple input redirections not allowed");
        goto error_cleanup;
      }
      TOK_consume(tokens); // Consume '<'

      if (TOK_next_type(tokens) != TOK_WORD)
      {
        snprintf(errmsg, errmsg_sz, "Expected filename after input redirection");
        goto error_cleanup;
      }

      pipeline_set_input_file(pipeline, TOK_next(tokens).value);
      input_redirected = 1;
      break;

    case TOK_GREATERTHAN:
      // Output redirection
      if (output_redirected)
      {
        snprintf(errmsg, errmsg_sz, "Multiple output redirections not allowed");
        goto error_cleanup;
      }
      TOK_consume(tokens); // Consume '>'

      if (TOK_next_type(tokens) != TOK_WORD)
      {
        snprintf(errmsg, errmsg_sz, "Expected filename after output redirection");
        goto error_cleanup;
      }

      pipeline_set_output_file(pipeline, TOK_next(tokens).value);
      output_redirected = 1;
      break;

    case TOK_PIPE:
      // Complete the current command and start a new one
      if (current_command.arg_count > 0)
      {
        current_command.args[current_command.arg_count] = NULL;

        // Add command to pipeline
        if (!pipeline_add_command(pipeline, &current_command))
        {
          snprintf(errmsg, errmsg_sz, "Failed to add command to pipeline");
          goto error_cleanup;
        }

        // Reset the current command
        memset(&current_command, 0, sizeof(Command));
        current_command.args = malloc(sizeof(char *) * 10);
        current_command.arg_capacity = 10;
        current_command.arg_count = 0;
      }
      break;

    default:
      break;
    }

    // Consume the current token
    TOK_consume(tokens);
  }

  // Add the last command if it contains arguments
  if (current_command.arg_count > 0)
  {
    current_command.args[current_command.arg_count] = NULL;
    if (!pipeline_add_command(pipeline, &current_command))
    {
      snprintf(errmsg, errmsg_sz, "Failed to add final command to pipeline");
      goto error_cleanup;
    }
  }
  else
  {
    // Free the last command's args if no command was added
    free(current_command.args);
  }

  return pipeline;

error_cleanup:
  // Clean up allocated memory in case of error
  for (int i = 0; i < current_command.arg_count; i++)
  {
    free(current_command.args[i]);
  }
  free(current_command.args);
  pipeline_destroy(pipeline);
  return NULL;
}