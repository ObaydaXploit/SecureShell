#ifndef SHELL_H
#define SHELL_H

#include "types.h"

// Global variables (extern declarations)
extern char dangerous_cmds[MAX_DANGEROUS_CMDS][MAX_CMD_LEN];
extern size_t dangerous_cmds_count;
extern command_stats_t stats;
extern FILE *log_file;

/* Shell core functions */
void setup_shell(void);
void shell_loop(void);
void cleanup_shell(void);

/* Command parsing and execution */
char *read_line(char *buffer, size_t size, FILE *stream);
pipeline_t *parse_line(const char *line);
void free_pipeline(pipeline_t *pipeline);
int execute_line(const char *line);
int execute_pipeline(pipeline_t *pipeline);

/* Built-in commands */
int is_builtin(const char *cmd_str);
int execute_builtin(command_t *cmd);

/* Signal handling */
void setup_signal_handlers(void);

/* Utilities */
int has_consecutive_spaces(const char *str);
int tokenize(char *str_src, char **tokens_dest, int max_args);
void reconstruct_command_string(const command_t *cmd, char *cmd_str);

/* Dangerous commands */
size_t load_dangerous_commands(const char *filename);
int check_dangerous_pipeline(pipeline_t *pipeline);
int is_dangerous_command(const char *cmd, int *dangerous_cmd_index);

/* Statistics and logging */
void display_prompt(command_stats_t *stats);
void update_command_stats(command_stats_t *stats, double elapsed_time);
void log_command_execution(FILE *file, const char *command_str, double elapsed_time);

#endif // SHELL_H
