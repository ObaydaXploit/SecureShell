#include "../include/shell.h"

FILE *log_file = NULL; // Global variable

/**
 * @brief Initialize shell settings and signal handlers
 */
void setup_shell(void)
{
    setup_signal_handlers();

    // Initialize stats
    memset(&stats, 0, sizeof(stats));
    stats.min_time = DBL_MAX;
}

/**
 * @brief Main shell command loop
 */
void shell_loop(void)
{
    while (1)
    {
        char line[MAX_CMD_LEN];

        display_prompt(&stats);

        if (!read_line(line, sizeof(line), stdin))
        {
            break; // EOF
        }

        if (strlen(line) == 0)
        {
            continue;
        }

        execute_line(line);
    }
}

/**
 * @brief Clean up resources and print final statistics
 */
void cleanup_shell(void)
{
    // Print on exit
    printf("%d\n", stats.blocked_cmd_count + stats.unblocked_dangerous_cmds_count);

    // Close log file if open
    if (log_file && log_file != stdout && log_file != stderr)
    {
        fclose(log_file);
        log_file = NULL;
    }
}

/**
 * @brief Main function - entry point
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit status
 */
int main(int argc, char *argv[])
{
    if (argc > 3)
    {
        fprintf(stderr, "Usage: %s [dangerous_commands_file] [log_file]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load dangerous commands if dangerous_commands_file is provided
    if (argc > 1)
    {
        load_dangerous_commands(argv[1]);
    }

    // Open log file if provided
    if (argc > 2)
    {
        log_file = fopen(argv[2], "a");
        if (!log_file)
        {
            perror("fopen (log file)");
        }
    }

    setup_shell();
    shell_loop();
    cleanup_shell();

    return EXIT_SUCCESS;
}
