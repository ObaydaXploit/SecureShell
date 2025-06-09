#include "../include/shell.h"

/* Prompt format string */
#define PROMPT_FORMAT "#cmd:%d|#dangerous_cmd_blocked:%d|last_cmd_time:%.5f|avg_time:%.5f|min_time:%.5f|max_time:%.5f>> "

/**
 * @brief Display the shell prompt with command statistics
 * @param stats Pointer to command statistics structure
 */
void display_prompt(command_stats_t *stats)
{
    double last = 0.0, avg = 0.0, min = 0.0, max = 0.0;

    // Only show stats if at least one command has been executed
    if (stats->cmds_count > 0)
    {
        last = stats->last_time;
        avg = stats->avg_time;
        min = stats->min_time;
        max = stats->max_time;
    }

    printf(PROMPT_FORMAT,
           stats->cmds_count,
           stats->blocked_cmd_count,
           last,
           avg,
           min,
           max);
    fflush(stdout);
}
