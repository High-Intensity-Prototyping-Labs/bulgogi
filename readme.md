![bulgogi logo](./img/logo.png)

# bulgogi
InDev v0.7.3

## Overview
Bulgogi is a C/C++ build-system orchestrator for CubeSats.

## Synopsis
See [synopsis.txt](./synopsis.txt)

## Progress
Currently supported subcommands are:


```module add <MODULE> [TARGET]```


```bul init```


```bul module rm <MODULE> [TARGET]```


Planned subcommands:


```bul module remove <MODULE> [TARGET]```


```bul tree```


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
