#include "../include/shell.h"

/**
 * @brief Allocate and initialize a matrix with the given dimensions.
 * @param rows Number of rows
 * @param cols Number of columns
 * @return Pointer to allocated matrix or NULL on failure
 */
static matrix_t *create_matrix(int rows, int cols)
{
    matrix_t *mat = malloc(sizeof(matrix_t));
    if (!mat)
        return NULL;

    mat->rows = rows;
    mat->cols = cols;
    mat->data = calloc(rows * cols, sizeof(double));
    if (!mat->data)
    {
        free(mat);
        return NULL;
    }
    return mat;
}

/**
 * @brief Free the memory associated with a matrix.
 * @param mat Pointer to the matrix to free
 */
static void free_matrix(matrix_t *mat)
{
    if (mat)
    {
        free(mat->data);
        free(mat);
    }
}

/**
 * @brief Parse a matrix from a string in the format "(rows,cols:val1,val2,...,valN)".
 * @param str Input string to parse
 * @return Dynamically allocated matrix or NULL on format/parse error
 */
static matrix_t *parse_matrix(const char *str)
{
    if (!str || str[0] != '(' || !strchr(str, ')'))
    {
        return NULL;
    }

    // Find the parts
    const char *comma1 = strchr(str + 1, ',');
    const char *colon = strchr(str + 1, ':');
    const char *end = strchr(str + 1, ')');

    if (!comma1 || !colon || !end || comma1 >= colon || colon >= end)
    {
        return NULL;
    }

    // Parse dimensions
    int rows = atoi(str + 1);
    int cols = atoi(comma1 + 1);

    if (rows <= 0 || cols <= 0)
    {
        return NULL;
    }

    matrix_t *mat = create_matrix(rows, cols);
    if (!mat)
        return NULL;

    // Parse values
    const char *values = colon + 1;
    char *values_copy = strndup(values, end - values);
    if (!values_copy)
    {
        free_matrix(mat);
        return NULL;
    }

    char *token = strtok(values_copy, ",");
    int index = 0;

    while (token && index < rows * cols)
    {
        mat->data[index] = atof(token);
        token = strtok(NULL, ",");
        index++;
    }

    free(values_copy);

    if (index != rows * cols)
    {
        free_matrix(mat);
        return NULL;
    }

    return mat;
}

/**
 * @brief Check if two matrices have the same dimensions.
 * @param m1 First matrix
 * @param m2 Second matrix
 * @return 1 if compatible, 0 otherwise
 */
static int matrices_compatible(matrix_t *m1, matrix_t *m2)
{
    return (m1->rows == m2->rows && m1->cols == m2->cols);
}

/**
 * @brief Thread function to perform matrix addition or subtraction.
 * @param arg Pointer to thread_arg_t structure containing inputs and operation type
 * @return Pointer to result matrix or NULL on error/incompatibility
 */
static void *matrix_operation_thread(void *arg)
{
    thread_arg_t *args = (thread_arg_t *)arg;

    if (!matrices_compatible(args->left, args->right))
    {
        return NULL;
    }

    args->result = create_matrix(args->left->rows, args->left->cols);
    if (!args->result)
        return NULL;

    int size = args->left->rows * args->left->cols;

    if (args->operation == 'A')
    { // ADD
        for (int i = 0; i < size; i++)
        {
            args->result->data[i] = args->left->data[i] + args->right->data[i];
        }
    }
    else if (args->operation == 'S')
    { // SUB
        for (int i = 0; i < size; i++)
        {
            args->result->data[i] = args->left->data[i] - args->right->data[i];
        }
    }

    return args->result;
}

/**
 * @brief Compute the result of a sequence of matrix operations (addition or subtraction) in parallel.
 * @param matrices Array of matrix pointers
 * @param count Number of matrices
 * @param operation 'A' for addition, 'S' for subtraction
 * @return Result matrix or NULL on failure
 */
static matrix_t *compute_matrices_parallel(matrix_t **matrices, int count, char operation)
{
    if (count == 1)
    {
        // Base case: return copy of single matrix
        matrix_t *result = create_matrix(matrices[0]->rows, matrices[0]->cols);
        if (!result)
            return NULL;

        int size = matrices[0]->rows * matrices[0]->cols;
        memcpy(result->data, matrices[0]->data, size * sizeof(double));
        return result;
    }

    if (count == 2)
    {
        // Base case: compute two matrices
        thread_arg_t arg = {matrices[0], matrices[1], NULL, operation};
        matrix_operation_thread(&arg);
        return arg.result;
    }

    if (operation == 'S')
    {
        // For subtraction, we use a left-associative parallel approach
        // Split into two halves: left half and right half
        // Compute: (left_result) - (right_result)
        // where left_result = matrices[0] - matrices[1] - ... - matrices[mid-1]
        // and right_result = matrices[mid] + matrices[mid+1] + ... + matrices[count-1]

        int mid = count / 2;
        if (mid == 0)
            mid = 1; // Ensure at least one matrix in left part

        // Recursively compute left part (subtraction)
        matrix_t *left_result = compute_matrices_parallel(matrices, mid, 'S');
        if (!left_result)
            return NULL;

        // Recursively compute right part (addition, since we'll subtract the sum)
        matrix_t *right_result = compute_matrices_parallel(&matrices[mid], count - mid, 'A');
        if (!right_result)
        {
            free_matrix(left_result);
            return NULL;
        }

        // Now compute left_result - right_result
        thread_arg_t arg = {left_result, right_result, NULL, 'S'};
        matrix_operation_thread(&arg);

        // Clean up intermediate results
        free_matrix(left_result);
        free_matrix(right_result);

        return arg.result;
    }
    else
    {
        // Parallel tree code for ADD operations
        int pairs = count / 2;
        int remainder = count % 2;

        matrix_t **next_level = malloc((pairs + remainder) * sizeof(matrix_t *));
        if (!next_level)
            return NULL;

        pthread_t *threads = malloc(pairs * sizeof(pthread_t));
        thread_arg_t *args = malloc(pairs * sizeof(thread_arg_t));

        if (!threads || !args)
        {
            free(next_level);
            free(threads);
            free(args);
            return NULL;
        }

        // Create threads for pairs
        for (int i = 0; i < pairs; i++)
        {
            args[i].left = matrices[i * 2];
            args[i].right = matrices[i * 2 + 1];
            args[i].result = NULL;
            args[i].operation = operation;

            if (pthread_create(&threads[i], NULL, matrix_operation_thread, &args[i]) != 0)
            {
                for (int j = 0; j < i; j++)
                {
                    pthread_join(threads[j], NULL);
                    if (args[j].result)
                        free_matrix(args[j].result);
                }
                free(next_level);
                free(threads);
                free(args);
                return NULL;
            }
        }

        // Wait for all threads and collect results
        for (int i = 0; i < pairs; i++)
        {
            pthread_join(threads[i], NULL);
            next_level[i] = args[i].result;

            if (!next_level[i])
            {
                for (int j = 0; j < pairs; j++)
                {
                    if (next_level[j])
                        free_matrix(next_level[j]);
                }
                free(next_level);
                free(threads);
                free(args);
                return NULL;
            }
        }

        // Handle odd matrix (if any)
        if (remainder)
        {
            next_level[pairs] = matrices[count - 1];
        }

        free(threads);
        free(args);

        // Recursive call for next level
        matrix_t *final_result = compute_matrices_parallel(next_level, pairs + remainder, operation);

        // Clean up intermediate results (but not the odd one if it exists)
        for (int i = 0; i < pairs; i++)
        {
            free_matrix(next_level[i]);
        }

        free(next_level);
        return final_result;
    }
}

/**
 * @brief Print a matrix to stdout in the format "(rows,cols:val1,val2,...)".
 * @param mat Pointer to matrix to print
 */

static void print_matrix(matrix_t *mat)
{
    printf("(%d,%d:", mat->rows, mat->cols);

    int size = mat->rows * mat->cols;
    for (int i = 0; i < size; i++)
    {
        if (i > 0)
            printf(",");

        // Check if it's a whole number
        if (mat->data[i] == (int)mat->data[i])
        {
            printf("%d", (int)mat->data[i]);
        }
        else
        {
            printf("%.10g", mat->data[i]);
        }
    }

    printf(")\n");
}

/**
 * @brief Matrix calculator built-in command
 * @param cmd Command structure
 * @return 0 on success, 1 on error
 */
static int builtin_mcalc(command_t *cmd)
{
    if (cmd->argc < 4)
    {
        printf("ERR_MAT_INPUT\n");
        return 1;
    }

    // Last argument should be operation
    char *operation = cmd->args[cmd->argc - 1];
    if (strcmp(operation, "ADD") != 0 && strcmp(operation, "SUB") != 0)
    {
        printf("ERR_MAT_INPUT\n");
        return 1;
    }

    int matrix_count = cmd->argc - 2; // Exclude command name and operation
    if (matrix_count < 2)
    {
        printf("ERR_MAT_INPUT\n");
        return 1;
    }

    // Parse all matrices
    matrix_t **matrices = malloc(matrix_count * sizeof(matrix_t *));
    if (!matrices)
    {
        return 1;
    }

    for (int i = 0; i < matrix_count; i++)
    {
        matrices[i] = parse_matrix(cmd->args[i + 1]);
        if (!matrices[i])
        {
            // Clean up and return error
            for (int j = 0; j < i; j++)
            {
                free_matrix(matrices[j]);
            }
            free(matrices);
            printf("ERR_MAT_INPUT\n");
            return 1;
        }
    }

    // Check compatibility
    for (int i = 1; i < matrix_count; i++)
    {
        if (!matrices_compatible(matrices[0], matrices[i]))
        {
            for (int j = 0; j < matrix_count; j++)
            {
                free_matrix(matrices[j]);
            }
            free(matrices);
            printf("ERR_MAT_INPUT\n");
            return 1;
        }
    }

    char op = (strcmp(operation, "ADD") == 0) ? 'A' : 'S';
    matrix_t *result = compute_matrices_parallel(matrices, matrix_count, op);

    if (!result)
    {
        for (int i = 0; i < matrix_count; i++)
        {
            free_matrix(matrices[i]);
        }
        free(matrices);
        return 1;
    }

    // Print result
    print_matrix(result);

    // Clean up
    for (int i = 0; i < matrix_count; i++)
    {
        free_matrix(matrices[i]);
    }
    free(matrices);
    free_matrix(result);

    return 0;
}

/**
 * @brief Custom tee implementation
 * @param cmd Command structure
 * @return 0 on success, 1 on error
 */
static int builtin_my_tee(command_t *cmd)
{
    int append = 0, start = 1;

    if (cmd->args[1] && strcmp(cmd->args[1], "-a") == 0)
    {
        append = 1;
        start = 2;
    }

    int fds[(append == 0) ? MAX_ARGS - 1 : MAX_ARGS - 2], count = 0;

    for (int i = start; i < cmd->argc; ++i)
    {
        fds[count] = open(cmd->args[i], O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC), DEFAULT_FILE_PERMISSIONS);

        if (fds[count] < 0)
        {
            perror(cmd->args[i]);
            // Close already opened fds before returning
            for (int j = 0; j < count; ++j)
                close(fds[j]);
            return 1;
        }
        count++;
    }

    char buf[BUFFER_SIZE];
    ssize_t n;
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0)
    {
        write(STDOUT_FILENO, buf, n);
        for (int i = 0; i < count; ++i)
        {
            write(fds[i], buf, n);
        }
    }

    for (int i = 0; i < count; ++i)
    {
        close(fds[i]);
    }

    return 0;
}

/**
 * @brief Change directory built-in command
 * @param cmd Command structure
 * @return 0 on success, 1 on error
 */
static int builtin_cd(command_t *cmd)
{
    char *path = (cmd->argc > 1) ? cmd->args[1] : getenv("HOME");

    if (!path)
    {
        fprintf(stderr, "cd: HOME not set\n");
        return 1;
    }

    if (chdir(path) == -1)
    {
        perror("cd");
        return 1;
    }

    return 0;
}

/**
 * @brief Exit shell built-in command
 * @param cmd Command structure
 * @return Does not return (calls exit)
 */
static int builtin_exit(command_t *cmd)
{
    int exit_code = 0;
    if (cmd->argc > 1)
    {
        exit_code = atoi(cmd->args[1]);
    }

    cleanup_shell();

    exit(exit_code);
}

/**
 * @brief Check if a command is a built-in
 * @param cmd_str Command string to check
 * @return 1 if built-in, 0 otherwise
 */
int is_builtin(const char *cmd_str)
{
    return (strcmp(cmd_str, "cd") == 0 || strcmp(cmd_str, "exit") == 0) ||
           strcmp(cmd_str, "my_tee") == 0 ||
           strcmp(cmd_str, "mcalc") == 0;
}

/**
 * @brief Execute a built-in command
 * @param cmd Command structure
 * @return Command exit status
 */
int execute_builtin(command_t *cmd)
{
    if (strcmp(cmd->args[0], "cd") == 0)
    {
        return builtin_cd(cmd);
    }
    else if (strcmp(cmd->args[0], "exit") == 0)
    {
        return builtin_exit(cmd);
    }
    else if (strcmp(cmd->args[0], "my_tee") == 0)
    {
        return builtin_my_tee(cmd);
    }
    else if (strcmp(cmd->args[0], "mcalc") == 0)
    {
        return builtin_mcalc(cmd);
    }

    return -1; // Should never reach here
}
