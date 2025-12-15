# wshell Configuration System

## Overview

The wshell configuration system provides bash-like variable management with security-first design. Variables are stored in a per-user configuration file and loaded at shell startup.

## Configuration File

### Location
- **Unix/Linux/macOS**: `~/.wshellrc`
- **Windows**: `%USERPROFILE%\.wshellrc`

### Format

The configuration file uses a simple `key=value` format, similar to bash:

```bash
# Comments start with #
VARIABLE_NAME=value

# Quotes are optional but can be used
GREETING="Hello from wshell!"
MESSAGE='Single quoted value'

# Whitespace around = is trimmed
EDITOR = vim

# Values can contain = signs
PATH=/usr/bin:/usr/local/bin

# Empty lines are ignored

```

### Variable Naming Rules

Following bash conventions:
- Must start with a letter (a-z, A-Z) or underscore (_)
- Can contain letters, digits, and underscores
- Case-sensitive
- Cannot start with a digit

**Valid names**: `VAR`, `my_var`, `_private`, `VAR123`  
**Invalid names**: `123var`, `my-var`, `var.name`

## Security Features

The configuration system follows AI security guidelines with:

### Input Validation
- **File size limit**: 1 MB maximum
- **Line length limit**: 10 KB per line
- **Variable count limit**: 10,000 maximum
- **Variable name length**: 1,000 characters maximum
- **Variable value length**: 100,000 characters maximum

### Safe Parsing
- Validates all variable names
- Handles malformed input gracefully
- Returns detailed error messages with line numbers
- No code execution - pure data parsing

### Error Handling
```cpp
auto result = Config::load_from_file(path);
if (result.has_value()) {
    // Success - use result->variables()
} else {
    // Error - check result.error().code and .message
}
```

## API Usage

### Loading Configuration

```cpp
#include "shell/config.hpp"

// Load from default location (~/.wshellrc)
auto path = shell::Config::default_config_path();
auto config = shell::Config::load_from_file(path);

if (config.has_value()) {
    // Configuration loaded successfully
    auto const& vars = config->variables();
}
```

### Parsing from String

```cpp
std::string content = "VAR1=value1\nVAR2=value2";
auto config = shell::Config::parse(content);
```

### Reading Variables

```cpp
Config config = /* ... */;

// Get a variable
if (auto* value = config.get("EDITOR")) {
    std::cout << "EDITOR=" << *value << "\n";
}

// Check if variable exists
if (config.has("PATH")) {
    // Variable exists
}

// Get all variables
for (auto const& [name, value] : config.variables()) {
    std::cout << name << "=" << value << "\n";
}
```

### Setting Variables

```cpp
Config config;

// Set a variable
if (config.set("MY_VAR", "my_value")) {
    // Success
} else {
    // Invalid variable name
}

// Unset a variable
config.unset("MY_VAR");

// Clear all variables
config.clear();
```

## Error Codes

| Error Code | Description |
|------------|-------------|
| `FILE_NOT_FOUND` | Configuration file doesn't exist |
| `PERMISSION_DENIED` | Cannot read configuration file |
| `PARSE_ERROR` | Syntax error in configuration |
| `INVALID_VARIABLE_NAME` | Variable name doesn't follow rules |
| `FILE_TOO_LARGE` | File exceeds size limit |
| `IO_ERROR` | I/O error reading file |

## Example Configuration

See `.wshellrc.example` for a complete example:

```bash
# Editor preferences
EDITOR=vim
PAGER=less

# Paths
PROJECT_DIR=/home/user/projects

# Colors
COLORTERM=truecolor
TERM=xterm-256color

# Custom settings
LOG_LEVEL=info
GREETING="Hello from wshell!"
```

## Integration

The configuration system is automatically loaded when wshell starts. You can see loaded variables with:

```bash
$ wshell echo test
wshell version 0.1.0
Loaded 11 variables from /home/user/.wshellrc
Configuration variables:
  EDITOR=vim
  PAGER=less
  ...
```

## Future Enhancements

Planned features:
- Variable expansion in commands (`$VAR` or `${VAR}`)
- Export to environment variables
- `set` and `unset` commands
- Variable persistence (saving runtime changes)
- Shell functions and aliases
