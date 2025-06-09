#include "../include/shell.h"

static void sigchld_handler(int sig)
{
    (void)sig; // suppress unused parameter warning
    // Reap zombie processes
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
        // Keep reaping
    }
}

void setup_signal_handlers(void)
{
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, NULL);
}
