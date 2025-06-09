#include "../include/shell.h"

/**
 * @brief Trim leading and trailing whitespace from a string
 * @param str String to trim (modified in place)
 */
static void trim_spaces(char *str)
{
    if (!str || !*str)
        return;

    // Trim leading
    char *start = str;
    while (isspace((unsigned char)*start))
        start++;

    // Move to start
    if (start != str)
        memmove(str, start, strlen(start) + 1);

    // Trim trailing
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
}

/**
 * @brief Read a line from input stream with proper handling
 * @param buffer Buffer to store the line
 * @param size Size of the buffer
 * @param stream Input stream to read from
 * @return Pointer to buffer on success, NULL on EOF or error
 */
char *read_line(char *buffer, size_t size, FILE *stream)
{
    if (!fgets(buffer, size, stream))
    {
        return NULL; // EOF or error
    }

    // Remove newline if present
    char *newline = strchr(buffer, '\n');
    if (newline)
    {
        *newline = '\0';
    }
    else
    {
        // If we filled the buffer and there's no newline, flush the rest
        int c;
        while ((c = fgetc(stream)) != '\n' && c != EOF)
        {
            // Consume remaining characters
        }
    }

    // Trim leading/trailing spaces
    trim_spaces(buffer);
    return buffer;
}
