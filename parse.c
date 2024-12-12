#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#include "parse.h"
#include "Tokenize.h"
#include "pipeline.h"
#include "ast.h"
#include "clist.h"

Pipeline *parse_tokens(CList tokens, char *errmsg, size_t errmsg_sz)
{
  Pipeline *pipeline = NULL;
  Pipeline *current_pipeline = NULL;
  Command *current_command = NULL;

  // Reset error message buffer
  if (errmsg)
    errmsg[0] = '\0';

  // Validate input tokens
  if (CL_length(tokens) == 0)
  {
    snprintf(errmsg, errmsg_sz, "No tokens to parse");
    return NULL;
  }

  while (CL_length(tokens) > 0)
  {
    Token token = TOK_next(tokens);
    TOK_consume(tokens);

    // Word or Quoted Word Processing
    if (token.type == TOK_WORD || token.type == TOK_QUOTED_WORD)
    {
      // Initialize command if not exists
      if (current_command == NULL)
      {
        current_command = create_command();
      }

      // Handle Globbing for Wildcard Expansion
      if (strchr(token.value, '*') && strchr(token.value, '.'))
      {
        glob_t globbuf;
        int glob_result = glob(token.value, GLOB_TILDE_CHECK, NULL, &globbuf);

        if (glob_result == 0)
        {
          for (size_t i = 0; i < globbuf.gl_pathc; i++)
          {
            // Expand and add glob results
            add_argument_to_command(current_command, globbuf.gl_pathv[i]);
          }
          globfree(&globbuf);
        }
        else
        {
          // No match found, add original token
          if (current_command->args == NULL)
          {
            // First argument becomes command name
            add_argument_to_command(current_command, token.value);
          }
          else
          {
            add_argument_to_command(current_command, token.value);
          }
        }
      }
      else
      {
        // Normal word token processing
        if (current_command->args == NULL)
        {
          // First argument becomes command name
          add_argument_to_command(current_command, token.value);
        }
        else
        {
          add_argument_to_command(current_command, token.value);
        }
      }
    }
    // Pipe Handling
    else if (token.type == TOK_PIPE)
    {
      if (current_command != NULL)
      {
        // Create new pipeline segment
        Pipeline *new_pipeline = malloc(sizeof(Pipeline));
        if (new_pipeline == NULL)
        {
          snprintf(errmsg, errmsg_sz, "Memory allocation error for pipeline");
          // Cleanup previous allocations
          free_pipeline(pipeline);
          return NULL;
        }

        new_pipeline->command = current_command;
        new_pipeline->next = NULL;
        new_pipeline->input_file = NULL;
        new_pipeline->output_file = NULL;

        if (pipeline == NULL)
        {
          pipeline = new_pipeline;
        }
        else
        {
          current_pipeline->next = new_pipeline;
        }
        current_pipeline = new_pipeline;
        current_command = NULL;
      }
    }
    // Input Redirection
    else if (token.type == TOK_LESSTHAN)
    {
      if (CL_length(tokens) == 0)
      {
        snprintf(errmsg, errmsg_sz, "Unexpected end of input after '<'");
        free_pipeline(pipeline);
        return NULL;
      }

      Token input_file = TOK_next(tokens);
      TOK_consume(tokens);

      if (current_pipeline != NULL)
      {
        set_input_file(current_pipeline, input_file.value);
      }
    }
    // Output Redirection
    else if (token.type == TOK_GREATERTHAN)
    {
      if (CL_length(tokens) == 0)
      {
        snprintf(errmsg, errmsg_sz, "Unexpected end of input after '>'");
        free_pipeline(pipeline);
        return NULL;
      }

      Token output_file = TOK_next(tokens);
      TOK_consume(tokens);

      if (current_pipeline != NULL)
      {
        set_output_file(current_pipeline, output_file.value);
      }
    }
  }

  // Finalize last command in pipeline
  if (current_command != NULL)
  {
    Pipeline *new_pipeline = malloc(sizeof(Pipeline));
    if (new_pipeline == NULL)
    {
      snprintf(errmsg, errmsg_sz, "Memory allocation error for final pipeline");
      free_pipeline(pipeline);
      return NULL;
    }

    new_pipeline->command = current_command;
    new_pipeline->next = NULL;
    new_pipeline->input_file = NULL;
    new_pipeline->output_file = NULL;

    if (pipeline == NULL)
    {
      pipeline = new_pipeline;
    }
    else
    {
      current_pipeline->next = new_pipeline;
    }
  }

  return pipeline;
}

void free_pipeline(Pipeline *pipeline)
{
  while (pipeline != NULL)
  {
    Pipeline *next_pipeline = pipeline->next;

    // Free command arguments
    for (int i = 0; i < pipeline->command->arg_count - 1; i++)
    {
      free(pipeline->command->args[i]);
    }
    free(pipeline->command->args);

    // Free input and output file paths if they exist
    if (pipeline->input_file)
      free(pipeline->input_file);
    if (pipeline->output_file)
      free(pipeline->output_file);

    // Free command and pipeline
    free(pipeline->command);
    free(pipeline);

    pipeline = next_pipeline;
  }
}