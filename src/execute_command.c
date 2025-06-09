#include "../include/shell.h"

/**
 * @brief Setup stderr redirection for a command
 * @param cmd Command with potential stderr redirection
 */
static void setup_error_redirection(const command_t *cmd)
{
    if (cmd->stderr_file)
    {
        int stderr_fd = open(cmd->stderr_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (stderr_fd == -1)
        {
            perror("open stderr file");
            exit(1);
        }
        dup2(stderr_fd, STDERR_FILENO);
        close(stderr_fd);
    }
}

/**
 * @brief Setup pipe for left side of pipeline
 * @param pipefd Pipe file descriptors
 */
static void setup_pipe_left(int pipefd[2])
{
    close(pipefd[0]);
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
}

/**
 * @brief Setup pipe for right side of pipeline
 * @param pipefd Pipe file descriptors
 */
static void setup_pipe_right(int pipefd[2])
{
    close(pipefd[1]);
    dup2(pipefd[0], STDIN_FILENO);
    close(pipefd[0]);
}

/**
 * @brief Fork and execute external command
 * @param cmd Command to execute
 * @param is_background Whether to run in background
 * @return Exit status or special codes
 */
static int fork_and_execute_external(const command_t *cmd, int is_background)
{
    pid_t pid = fork();

    if (pid == 0)
    {
        setup_error_redirection(cmd);
        execvp(cmd->args[0], cmd->args);
        perror("execvp");
        exit(127);
    }
    else if (pid > 0)
    {
        if (!is_background)
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
                return WEXITSTATUS(status);
            return -1;
        }
        return -2;
    }
    else
    {
        perror("fork");
        return -1;
    }
}

/**
 * @brief Execute a simple (non-piped) command
 * @param cmd Command to execute
 * @param is_background Whether to run in background
 * @return Exit status
 */
static int execute_simple_command(command_t *cmd, int is_background)
{
    if (is_builtin(cmd->args[0]))
    {
        return execute_builtin(cmd);
    }
    else
    {
        return fork_and_execute_external(cmd, is_background);
    }
}

/**
 * @brief Execute piped commands
 * @param pipeline Pipeline containing two commands
 * @return Exit status
 */
static int execute_piped_commands(pipeline_t *pipeline)
{
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return -1;
    }

    // Left side of pipe
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        setup_pipe_left(pipefd);
        setup_error_redirection(&pipeline->commands[0]);

        if (is_builtin(pipeline->commands[0].args[0]))
        {
            exit(execute_builtin(&pipeline->commands[0]));
        }
        else
        {
            execvp(pipeline->commands[0].args[0], pipeline->commands[0].args);
            perror("execvp");
            exit(127);
        }
    }

    // Right side of pipe
    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        setup_pipe_right(pipefd);
        setup_error_redirection(&pipeline->commands[1]);

        if (is_builtin(pipeline->commands[1].args[0]))
        {
            exit(execute_builtin(&pipeline->commands[1]));
        }
        else
        {
            execvp(pipeline->commands[1].args[0], pipeline->commands[1].args);
            perror("execvp");
            exit(127);
        }
    }

    // Close pipe ends in parent
    close(pipefd[0]);
    close(pipefd[1]);

    // Wait for children
    if (!pipeline->is_background)
    {
        int status1, status2;
        waitpid(pid1, &status1, 0);
        waitpid(pid2, &status2, 0);

        // If either command failed with 127 (command not found), return 127
        if ((WIFEXITED(status1) && WEXITSTATUS(status1) == 127) || (WIFEXITED(status2) && WEXITSTATUS(status2) == 127))
        {
            return 127;
        }

        if (WIFEXITED(status2))
            return WEXITSTATUS(status2);
        return -1;
    }

    return -2;
}

/**
 * @brief Execute a pipeline (single command or pipe)
 * @param pipeline Pipeline to execute
 * @return Exit status
 */
int execute_pipeline(pipeline_t *pipeline)
{
    if (pipeline->cmd_count == 1)
    {
        return execute_simple_command(&pipeline->commands[0], pipeline->is_background);
    }
    else
    {
        return execute_piped_commands(pipeline);
    }
}

/**
 * @brief Execute a command line
 * @param line Command line string
 * @return Exit status
 */
int execute_line(const char *line)
{
    if (has_consecutive_spaces(line))
    {
        printf("ERR_SPACE\n");
        return -1;
    }

    pipeline_t *pipeline = parse_line(line);
    if (!pipeline)
    {
        return -1; // Error during parsing, already printed
    }

    // Check for dangerous commands
    if (check_dangerous_pipeline(pipeline) == -1)
    {
        free_pipeline(pipeline);
        return -1; // Dangerous command blocked
    }

    // Time the command execution
    struct timeval start, end;
    gettimeofday(&start, NULL);

    int result = execute_pipeline(pipeline);

    // Only update stats and log if command got executed
    if (result >= 0 && result != 127)
    {
        gettimeofday(&end, NULL);
        ++stats.cmds_count; // Update executed commands count
        double elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        update_command_stats(&stats, elapsed_time);
        log_command_execution(log_file, line, elapsed_time);
    }

    free_pipeline(pipeline);
    return result;
}
