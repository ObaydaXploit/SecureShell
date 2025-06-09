#include "../include/shell.h"

/**
 * Checks if a string contains consecutive spaces or a tab
 *
 * @param str String to check
 * @return 1 if consecutive spaces found, 0 otherwise
 */
int has_consecutive_spaces(const char *str)
{
    while (*str)
    {
        if (*str == '\t' || (*str == ' ' && *(str + 1) == ' '))
        {
            return 1; // Found consecutive spaces
        }
        str++;
    }
    return 0;
}

int tokenize(char *str_src, char **tokens_dest, int max_args)
{
    int count = 0;
    char *ptr = str_src;

    while (*ptr && count < max_args)
    {
        // Skip leading whitespace
        while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n'))
            ptr++;

        if (!*ptr)
            break;

        char *token_start = ptr;

        if (*ptr == '"')
        {
            // Handle quoted string
            ptr++;             // Skip opening quote
            token_start = ptr; // Start after the quote

            // Find closing quote
            while (*ptr && *ptr != '"')
                ptr++;

            if (*ptr == '"')
            {
                *ptr = '\0'; // Null-terminate the token (removing closing quote)
                ptr++;       // Move past the closing quote
            }
            // If no closing quote found, treat rest as the token
        }
        else
        {
            // Handle unquoted token
            while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n')
                ptr++;

            if (*ptr)
            {
                *ptr = '\0'; // Null-terminate the token
                ptr++;       // Move to next character
            }
        }

        tokens_dest[count++] = token_start;
    }

    if (count >= max_args && *ptr)
    {
        return -1; // Too many tokens
    }

    tokens_dest[count] = NULL;
    return count;
}

/**
 * Reconstructs command string from command_t structure
 *
 * @param cmd Command structure
 * @param cmd_str Output buffer for reconstructed command
 */
void reconstruct_command_string(const command_t *cmd, char *cmd_str)
{
    cmd_str[0] = '\0';

    for (int i = 0; i < cmd->argc; i++)
    {
        if (i > 0)
        {
            strcat(cmd_str, " ");
        }
        strcat(cmd_str, cmd->args[i]);
    }
}
