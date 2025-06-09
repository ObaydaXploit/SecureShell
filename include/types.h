#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <signal.h>
#include <bits/sigaction.h>
#include <asm-generic/signal-defs.h>
#include <ctype.h>
#include <pthread.h>
#include <math.h>
#include <float.h>
#include <sys/time.h>

/* Constants */
#define MAX_CMD_LEN 1024
#define MAX_ARGS 7
#define MAX_DANGEROUS_CMDS 100
#define BUFFER_SIZE 1024
#define DEFAULT_FILE_PERMISSIONS 0644

/**
 * @brief Structure representing a single command with its arguments
 */
typedef struct
{
    char **args;       // argv-style array of pointers to each argument string
    int argc;          // how many strings are in args
    char *stderr_file; // if non-NULL, filename to redirect stderr (“2> file”)
} command_t;

/**
 * @brief Structure representing a pipeline of commands
 */
typedef struct
{
    command_t commands[2]; // at most two commands: [0]=left side, [1]=right side
    int cmd_count;         // 1 for a single command, 2 if there’s a pipe
    int is_background;     // whether the pipeline ends with ‘&’
} pipeline_t;

typedef struct
{
    int cmds_count;                     // Total number of commands executed
    int blocked_cmd_count;              // Number of dangerous commands blocked
    double last_time;                   // Execution time of the last command
    double min_time;                    // Minimum execution time
    double max_time;                    // Maximum execution time
    double avg_time;                    // Average execution time
    double total_time;                  // Accumulated time for computing average
    int unblocked_dangerous_cmds_count; // Count of commands that are similar to the dangerous commands
} command_stats_t;

/**
 * @brief Structure representing a matrix for calculations
 */
typedef struct
{
    int rows;     // Number of matrix rows
    int cols;     // Number of matrix columns
    double *data; // Matrix data in row-major order
} matrix_t;

/**
 * @brief Thread argument structure for matrix operations
 */
typedef struct
{
    matrix_t *left;   // Left operand matrix
    matrix_t *right;  // Right operand matrix
    matrix_t *result; // Result matrix
    char operation;   // Operation type: 'A' for ADD, 'S' for SUB
} thread_arg_t;

#endif // TYPES_H
