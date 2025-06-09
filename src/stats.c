#include "../include/shell.h"

/**
 * Updates command execution statistics
 *
 * @param stats Pointer to statistics structure
 * @param elapsed_time Execution time of the last command
 */
void update_command_stats(command_stats_t *stats, double elapsed_time)
{
    stats->last_time = elapsed_time;
    stats->total_time += elapsed_time;
    stats->avg_time = stats->total_time / stats->cmds_count;

    // Update min/max times
    if (elapsed_time < stats->min_time)
    {
        stats->min_time = elapsed_time;
    }
    if (elapsed_time > stats->max_time)
    {
        stats->max_time = elapsed_time;
    }
}
