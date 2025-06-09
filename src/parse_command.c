#include "../include/shell.h"

/**
 * @brief Check if command line ends with background operator (&)
 * @param line Pointer to command line string (may be modified)
 * @return 1 if background operator found, 0 otherwise
 */
static int check_background(char **line)
{
    char *pos = *line + strlen(*line) - 1;

    // Skip trailing whitespace
    while (pos > *line && isspace(*pos))
    {
        pos--;
    }

    if (*pos == '&')
    {
        *pos = '\0'; // Remove the &
        return 1;
    }
    return 0;
}

/**
 * @brief Split command line on pipe character
 * @param line Command line string to split
 * @param left_cmd Pointer to store left command
 * @param right_cmd Pointer to store right command
 * @return 1 if pipe found, 0 otherwise
 */
static int split_on_pipe(char *line, char **left_cmd, char **right_cmd)
{
    char *pipe_pos = strchr(line, '|');
    if (!pipe_pos)
    {
        *left_cmd = line;
        *right_cmd = NULL;
        return 0;
    }

    *pipe_pos = '\0';
    *left_cmd = line;
    *right_cmd = pipe_pos + 1;
    return 1;
}

/**
 * @brief Parse a single command string into command structure
 * @param cmd_str Command string to parse
 * @param cmd Command structure to populate
 * @return 0 on success, -1 on error
 */
static int parse_single_command(const char *cmd_str, command_t *cmd)
{
    char *tokens[MAX_ARGS + 1]; // +1 for NULL terminator
    char *cmd_copy = strdup(cmd_str);
    if (!cmd_copy)
    {
        return -1;
    }

    int token_count = tokenize(cmd_copy, tokens, MAX_ARGS);
    if (token_count == -1)
    {
        printf("ERR_ARGS\n");
        fflush(stdout);
        free(cmd_copy);
        return -1;
    }

    cmd->stderr_file = NULL;
    cmd->args = malloc((token_count + 1) * sizeof(char *));
    if (!cmd->args)
    {
        free(cmd_copy);
        return -1;
    }

    int arg_index = 0;

    for (int i = 0; i < token_count; ++i)
    {
        if (strcmp(tokens[i], "2>") == 0 && i + 1 < token_count)
        {
            cmd->stderr_file = strdup(tokens[++i]);
        }
        else
        {
            cmd->args[arg_index++] = strdup(tokens[i]);
        }
    }

    cmd->args[arg_index] = NULL;
    cmd->argc = arg_index;

    free(cmd_copy);
    return 0;
}

/**
 * @brief Create and initialize a pipeline structure
 * @return Pointer to new pipeline structure
 */
static pipeline_t *create_pipeline(void)
{
    pipeline_t *pipeline = malloc(sizeof(pipeline_t));
    if (!pipeline)
    {
        perror("malloc");
        exit(1);
    }
    return pipeline;
}

/**
 * @brief Parse command line into pipeline structure
 * @param line Command line string to parse
 * @return Pointer to pipeline structure, or NULL on error
 */
pipeline_t *parse_line(const char *line)
{
    char *line_copy = strdup(line);

    // Step 1: Check background
    int is_bg = check_background(&line_copy);

    // Step 2: Split on pipe
    char *left_cmd, *right_cmd;
    int has_pipe = split_on_pipe(line_copy, &left_cmd, &right_cmd);

    // Step 3: Create pipeline
    pipeline_t *pipeline = create_pipeline();
    pipeline->is_background = is_bg;
    pipeline->cmd_count = has_pipe ? 2 : 1;

    // Step 4: Parse commands - check for errors
    if (parse_single_command(left_cmd, &pipeline->commands[0]) == -1)
    {
        free(pipeline);
        free(line_copy);
        return NULL; // Error encountered
    }

    if (has_pipe)
    {
        if (parse_single_command(right_cmd, &pipeline->commands[1]) == -1)
        {
            // Clean up first command
            command_t *cmd = &pipeline->commands[0];
            for (int j = 0; j < cmd->argc; j++)
            {
                free(cmd->args[j]);
            }
            free(cmd->args);
            if (cmd->stderr_file)
            {
                free(cmd->stderr_file);
            }

            free(pipeline);
            free(line_copy);
            return NULL; // Error encountered
        }
    }

    free(line_copy);
    return pipeline;
}

/**
 * @brief Free memory allocated for pipeline structure
 * @param pipeline Pipeline to free
 */
void free_pipeline(pipeline_t *pipeline)
{
    if (!pipeline)
        return;

    for (int i = 0; i < pipeline->cmd_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        if (cmd->args)
        {
            for (int j = 0; j < cmd->argc; j++)
            {
                free(cmd->args[j]);
            }
            free(cmd->args);
        }

        if (cmd->stderr_file)
        {
            free(cmd->stderr_file);
        }
    }

    free(pipeline);
}
