#include "../include/shell.h"

void log_command_execution(FILE *file, const char *command_str, double elapsed_time)
{
    if (!file)
        return;

    fprintf(file, "%s : %.5f sec\n", command_str, elapsed_time);
    fflush(file);
}
