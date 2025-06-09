#include "../include/shell.h"

// Global variables
char dangerous_cmds[MAX_DANGEROUS_CMDS][MAX_CMD_LEN];
size_t dangerous_cmds_count = 0;
command_stats_t stats = {0};

/**
 * Extracts the base command (first word) from a command string
 *
 * @param cmd Full command string
 * @param base_cmd Output buffer for base command
 */
static void extract_base_command(const char *cmd, char *base_cmd)
{
    sscanf(cmd, "%s", base_cmd);
}

/**
 * Loads dangerous commands from a file
 *
 * @param filename Path to file containing dangerous commands
 * @return Number of dangerous commands loaded
 */
size_t load_dangerous_commands(const char *filename)
{
    dangerous_cmds_count = 0;
    FILE *file = fopen(filename, "r");

    if (!file)
    {
        // Continue without dangerous command checking
        return 0;
    }

    char line[MAX_CMD_LEN];
    while (dangerous_cmds_count < MAX_DANGEROUS_CMDS &&
           read_line(line, sizeof(line), file))
    {
        if (strlen(line) > 0)
        {
            strncpy(dangerous_cmds[dangerous_cmds_count], line, MAX_CMD_LEN);
            dangerous_cmds[dangerous_cmds_count][MAX_CMD_LEN - 1] = '\0';
            dangerous_cmds_count++;
        }
    }

    fclose(file);
    return dangerous_cmds_count;
}

/**
 * Checks if a command is considered dangerous
 *
 * @param cmd Command to check
 * @param dangerous_cmd_index Pointer to store index of matching dangerous command
 * @return 0: No match, 1: Base command match (warning), 2: Exact match (block)
 */
int is_dangerous_command(const char *cmd, int *dangerous_cmd_index)
{
    char cmd_base[MAX_CMD_LEN];
    extract_base_command(cmd, cmd_base);

    int found_base = 0;

    for (size_t i = 0; i < dangerous_cmds_count; i++)
    {
        char dangerous_cmd_base[MAX_CMD_LEN];
        extract_base_command(dangerous_cmds[i], dangerous_cmd_base);

        if (strcmp(cmd_base, dangerous_cmd_base) == 0)
        {
            found_base = 1;
            *dangerous_cmd_index = i;

            if (strcmp(cmd, dangerous_cmds[i]) == 0)
            {
                return 2; // Exact match - block the command
            }
        }
    }

    if (found_base)
    {
        return 1; // Base match only - warn but allow
    }

    return 0; // No match at all - safe command
}

/**
 * Checks pipeline for dangerous commands
 *
 * @param pipeline Pipeline to check
 * @return 0: Safe to execute, -1: Block execution
 */
int check_dangerous_pipeline(pipeline_t *pipeline)
{
    if (dangerous_cmds_count == 0)
    {
        return 0; // No dangerous commands loaded
    }

    for (int i = 0; i < pipeline->cmd_count; i++)
    {
        char cmd_str[MAX_CMD_LEN];
        reconstruct_command_string(&pipeline->commands[i], cmd_str);

        int dangerous_cmd_index = -1;
        int matching_level = is_dangerous_command(cmd_str, &dangerous_cmd_index);

        if (matching_level == 2)
        {
            // Exact match - block execution
            printf("ERR: Dangerous command detected (\"%s\"). Execution prevented.\n",
                   dangerous_cmds[dangerous_cmd_index]);
            fflush(stdout);
            stats.blocked_cmd_count++;
            return -1; // Block execution
        }
        else if (matching_level == 1)
        {
            // Base match - warn but continue
            printf("WARNING: Command similar to dangerous command (\"%s\"). Proceed with caution.\n",
                   dangerous_cmds[dangerous_cmd_index]);
            fflush(stdout);
            stats.unblocked_dangerous_cmds_count++;
        }
    }

    return 0; // Safe to execute
}
