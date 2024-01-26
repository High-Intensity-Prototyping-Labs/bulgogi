![bulgogi logo](./img/logo.png)

# bulgogi
C v0.1.10

## Overview
Bulgogi is a build-system orchestrator written in C.

## Synopsis
See [synopsis.txt](./synopsis.txt)

## Dependencies
Credit is owed where it is due:
- [yaml/libyaml](https://github.com/yaml/libyaml) - Canonical source repository for LibYAML.

All license requirements have been satisfied and do not conflict with this project's GPLv3 license.

See `LICENSE.<dependency>` for dependency licenses and `COPYING` for a copy of this project's license.

## Progress
Currently supported subcommands are:


Planned subcommands:


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


```
bul clean               # Clean project build files 
bul clean --all         # Additionally remove generated CMakeLists.txt
bul clean --all --purge # Permanently deletes all modules (DANGER)
```



```bul build```

## Limitations
- Must be used from the root of the project.
- Only supports 1 module/project layout.
- Assumes Unix-like or Linux environment.

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

## Dependencies 
Dependencies are broken down into 2 types: `target`, `module`.

A `module` is an indivisible unit of source code and headers that can be re-used across multiple targets.

A `target` (in bulgogi) is a conceptual entity which can _either_ turn into machine code (a binary) or server as an intermediary step there (an interface).

### Inferencing
Bulgogi uses inferencing whenever possible to analyze build projects. 

By default, all bulgogi targets are executable. 

```yaml
default:
- module1
```

In the above configuration, we assume `default` to be an executable target and `module1` to be a module which contains an executable component (like a `main()` routine).

Adding a target-dependency to the `default` target yields:

```yaml
default:
- module1
- target1

target1:
- module2
```

The use of `target1` as a dependency of `default` immediately gives it library status.

Thus we can write:

```yaml
default:    # ???
- module1   # default.out
- target1   # ???

target1:    # ???
- module2   # libmodule2.a
```

The reason for the elusive `???` will be illuminated momentarily. The most important part is developing an intuition for how bulgogi targets map onto real build targets.

- `default` indicates a `default.out` is needed as an executable target - no other targets depend on it to build.
- `module1` compiles to `default.out` because it is the only module-dependency of `default` - it _must_ contain the executable component needed.
- `module2` compiles to `libmodule2.a` as it is a module-dependency of a library target - it will be linked later on.

This leaves `target1` - it never really compiles and only serves as an intermidiary to construct `default.out`.

An equivalent structure would resemble the following:

```yaml
default:
- module1
- module2
```

In bigger projects, the reason bulgogi target-libraries are desireable is because they allow for larger groupings of code (modules) to be reused across multiple targets. And this can be done without needing to compile additional targets!

When standalone libraries are desired, they _must_ be standalone/self-contained modules. Period.

### Ambiguity
In some cases, inferencing alone cannot resolve to a unique executable module:

```yaml
default:
- module1
- module2
- target1 
- target2

target1:
- module3
- module4 

target2:
- module10
- module20
```

Here, even though both `target1` and `target2` are clearly library-dependencies of the `default` target, there isn't sufficient information to determine whether it is `module1` or `module2` which contains the executable component that `default` needs (like a `main()` routine).

In this case, an error will appear saying:

```
Ambiguity encountered. Consider resolving with an executable indicator (*).
```

When this happens, an executable indicator (`*`) must be used to flag the module-dependency which contains the required executable component for the target to be executable.

Note that once a module is declared to be executable at one point in the project, it _must_ be treated the same way throughout the entire project. Otherwise you may be faced with undefined behaviour.
