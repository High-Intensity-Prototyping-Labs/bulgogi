![bulgogi logo](./img/logo.png)

# bulgogi
InDev v0.7.5

## Overview
Bulgogi is a C/C++ build-system orchestrator for CubeSats.

## Synopsis
See [synopsis.txt](./synopsis.txt)

## Dependencies
Credit is owed where it is due:
- [CLIUtils/CLI11](https://github.com/CLIUtils/CLI11) - Command line parser for C++11.
- [jbeder/yaml-cpp](https://github.com/jbeder/yaml-cpp) - A YAML parser and emitter in C++.
- [pantor/inja](https://github.com/pantor/inja) - A template engine for Modern C++.
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++.

All license requirements have been satisfied and do not conflict with this project's GPLv3 license.

See `LICENSE.<project>.txt` for dependency licenses and `COPYING` for a copy of this project's license.

## Progress
Currently supported subcommands are:


```module add <MODULE> [TARGET]```


```bul init```


```
bul module rm <MODULE> [TARGET]             # Remove module and contents from project
bul module rm --cached <MODULE> [TARGET]    # Only remove entry in project.yaml

bul module remove <MODULE> [TARGET]         # Alias for `bul rm ...`
```


```bul tree```


```
bul generate            # Generates CMakeLists
bul generate --create   # Creates modules dirs if missing
```


Planned subcommands:


```bul build```


```bul clean```

## Usage
### Getting Started
All bulgogi projects need to be initialized in the current working directory.

```
bul init 
```

This will automatically create the `project.yaml` file in the current directory.

### Creating the first module 
Bulgogi understands build projects in terms of _modules_ and _targets_.

```
bul module add --create module1
```

This command _spawns_ the `module1` directory along with its subdirectories `src`, `inc` and `src/inc`. More on this later.

The important thing to note is that the `--create` flag was passed - this signaled to the CLI that it should not look for an existing directory.

### Adding existing code to the project 
If the directory structure of the imported module is supported, it is simply a matter of moving the directory into the project root and adding it.

```
bul module add imported_module
```

This assumes `imported_module` is the name of the directory with the compatible directory layout for the project.

### Module layouts 
Bulgogi currently only supports 1 module layout.

```
project
    \_ module1
    |   \_ src
    |   \_ inc
    |
    \_ imported_module 
    |   \_ src 
    |   \_ inc 
    |
    \_ project.yaml
```

This is generally how things should look. There are subtleties and details embedded within this overview that will be covered later.

### Generating the CMakeLists 
Now that a very simplistic project structure has been made (1 target, 1 module), it's time to generate the build files.

```
bul gen 
```

You should now have a `module1/` directory populated with a `CMakeLists.txt` file, as well as one in the project root.
