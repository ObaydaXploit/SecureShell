# SecureShell - Advanced Security-Focused Shell

A feature-rich, security-hardened, educatioinal shell implementation in C that provides command execution with built-in safety mechanisms, advanced matrix operations, pipeline support, and comprehensive performance monitoring.

## 🌟 Features

### 🛡️ Security Features
- **Dangerous Command Filtering**: Configurable blacklist system that blocks or warns about potentially harmful commands
- **Command Validation**: Three-tier security system (safe, warning, blocked)
- **Execution Prevention**: Automatic blocking of exact matches to dangerous command patterns
- **Audit Logging**: Comprehensive command execution logging with timestamps

### ⚡ Core Shell Capabilities
- **Pipeline Support**: Full pipe implementation (`cmd1 | cmd2`)
- **Background Execution**: Process management with `&` operator
- **Built-in Commands**: Custom implementations of essential shell utilities
- **Error Redirection**: Support for stderr redirection (`2> file`)
- **Signal Handling**: Robust SIGCHLD handling for zombie process cleanup

### 📊 Performance Monitoring
- **Real-time Statistics**: Live command execution metrics in prompt
- **Execution Timing**: Precise command timing with microsecond accuracy
- **Performance Analytics**: Min/max/average execution time tracking
- **Command Counting**: Total executed and blocked command statistics

### 🧮 Advanced Built-in Commands

#### Matrix Calculator (`mcalc`)
- **Parallel Processing**: Multi-threaded matrix operations for optimal performance
- **Matrix Operations**: Addition and subtraction of multiple matrices
- **Flexible Input**: Parse matrices in format `(rows,cols:val1,val2,...)`
- **Error Handling**: Comprehensive input validation and compatibility checking

#### Custom Tee (`my_tee`)
- **Standard Compliance**: Drop-in replacement for Unix `tee` command
- **Append Mode**: Support for `-a` flag to append to files
- **Multiple Outputs**: Write to multiple files simultaneously

#### Enhanced `cd` and `exit`
- **Home Directory Support**: Automatic HOME environment variable handling
- **Exit Codes**: Proper exit status management

## 🏗️ Architecture

### Core Components
```
├── main.c               # Shell initialization and main loop
├── shell.h/types.h      # Type definitions and function declarations
├── prompt.c             # Prompt display with the prompt string format
├── read_line.c          # Input processing with whitespace handling
├── parse_command.c      # Command parsing and pipeline construction
├── execute_command.c    # Command execution engine
├── builtins.c           # Built-in command implementations
├── dangerous_commands.c # Security filtering system
├── signals.c            # Signal handling
├── stats.c              # Performance statistics
├── utils.c              # Utility functions
└── logging.c            # Audit logging system
```

### Key Data Structures

```c
// Command representation
typedef struct {
    char **args;       // Command arguments (argv-style)
    int argc;          // Argument count
    char *stderr_file; // Error redirection target
} command_t;

// Pipeline support
typedef struct {
    command_t commands[2]; // Left and right side of pipe
    int cmd_count;         // 1 for simple, 2 for piped
    int is_background;     // Background execution flag
} pipeline_t;

// Performance statistics
typedef struct {
    int cmds_count;                     // Total commands executed
    int blocked_cmd_count;              // Dangerous commands blocked
    double last_time, min_time, max_time, avg_time;
    int unblocked_dangerous_cmds_count; // Warned but allowed commands
} command_stats_t;
```

## 🚀 Usage

### Basic Usage
```bash
# Compile and run
make
./shell [dangerous_commands_file] [log_file]

# Interactive prompt with live statistics
#cmd:5|#dangerous_cmd_blocked:1|last_cmd_time:0.00234|avg_time:0.00198|min_time:0.00123|max_time:0.00456>>
```

### Matrix Operations
```bash
# Add two 2x2 matrices
mcalc (2,2:1,2,3,4) (2,2:5,6,7,8) ADD
# Output: (2,2:6,8,10,12)

# Subtract multiple matrices
mcalc (3,3:1,2,3,4,5,6,7,8,9) (3,3:1,1,1,1,1,1,1,1,1) (3,3:0,1,0,1,0,1,0,1,0) SUB
```

### Pipeline Operations
```bash
# Simple pipe
ls -la | grep txt

# Background execution
long_running_command &

# Error redirection
command_with_errors 2> error.log

# A combination
ls -la 2> err1.txt | grep md 2> err2.txt
```

### Security Features
```bash
# Dangerous command blocking
rm -rf /  # Blocked if in dangerous_commands.txt
# Output: ERR: Dangerous command detected ("rm -rf /"). Execution prevented.

# Similar command warning
rm file.txt  # Warning if "rm" base command is dangerous
# Output: WARNING: Command similar to dangerous command ("rm -rf /"). Proceed with caution.
```

## 🔧 Configuration

### Dangerous Commands File
Create a text file with one potentially dangerous command per line:
```
rm -rf /
sudo rm -rf /
mkfs
dd if=/dev/zero
chmod 777 /
```

### Command Line Arguments
```bash
./shell                                    # Basic mode
./shell dangerous_cmds.txt                 # With security filtering
./shell dangerous_cmds.txt execution.log   # With logging
```

## 🔒 Security Model

### Three-Tier Protection
1. **Exact Match (BLOCKED)**: Command exactly matches dangerous pattern → execution prevented
2. **Base Match (WARNING)**: Base command matches dangerous pattern → warning issued, execution allowed
3. **No Match (SAFE)**: Command not in dangerous list → normal execution

### Input Validation
- **Space Validation**: Prevents consecutive spaces and tabs (`ERR_SPACE`)
- **Argument Limits**: Maximum 7 arguments per command (`ERR_ARGS`)
- **Matrix Validation**: Comprehensive matrix format checking (`ERR_MAT_INPUT`)

## 📈 Performance Features

### Statistics Display
The shell prompt shows real-time performance metrics:
- **cmd**: Total commands executed
- **dangerous_cmd_blocked**: Commands blocked by security system
- **last_cmd_time**: Execution time of last command (seconds)
- **avg_time**: Average execution time across all commands
- **min_time**: Fastest command execution time
- **max_time**: Slowest command execution time

### Logging Format
```
command_string : 0.00234 sec
ls -la | grep test : 0.00596 sec
mcalc (2,2:1,2,3,4) (2,2:5,6,7,8) ADD : 0.00006 sec
```

## 🛠️ Building

### Requirements
- GCC compiler with C99 support
- POSIX-compliant system (Linux/Unix)
- pthread library for matrix operations
- Standard C library with math support

### Compilation
```bash
# Basic compilation
gcc -std=c99 -Wall -Wextra -pthread -lm -o shell src/*.c

# With debugging
gcc -std=c99 -Wall -Wextra -g -pthread -lm -o shell src/*.c

# Optimized build
gcc -std=c99 -O2 -pthread -lm -o shell src/*.c
```

## 🔍 Error Handling

The shell provides specific error messages for different failure modes:
- `ERR_SPACE`: Consecutive spaces or tabs in input
- `ERR_ARGS`: Too many command arguments (>7)
- `ERR_MAT_INPUT`: Invalid matrix format or incompatible dimensions
- Standard system errors with `perror()` for system call failures

## 🎯 Advanced Features

### Multi-threaded Matrix Operations
- **Parallel Addition**: Tree-based parallel computation for multiple matrix addition
- **Left-associative Subtraction**: Efficient handling of matrix subtraction chains
- **Memory Management**: Automatic cleanup of intermediate results
- **Error Recovery**: Graceful handling of thread creation failures

### Signal Safety
- **SIGCHLD Handling**: Automatic zombie process cleanup
- **Signal Masking**: Proper signal handling during critical operations
- **Restart Semantics**: System call restart on signal interruption

### Memory Management
- **Dynamic Allocation**: Efficient memory usage for commands and matrices
- **Cleanup on Error**: Proper resource deallocation on failure paths
- **Leak Prevention**: Comprehensive memory management throughout

## 🤝 Contributing

When contributing to this project:
1. Follow the existing code style and structure
2. Add comprehensive error handling for new features
3. Update documentation for any new commands or features
4. Test security features thoroughly
5. Ensure thread safety for concurrent operations

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

### MIT License Summary
- ✅ Commercial use allowed
- ✅ Modification allowed  
- ✅ Distribution allowed
- ✅ Private use allowed
- ❌ No warranty provided
- ❌ No liability assumed

Feel free to use, modify, and distribute this code in your own projects!

---

**SecureShell** - Where security meets performance in command-line computing.
