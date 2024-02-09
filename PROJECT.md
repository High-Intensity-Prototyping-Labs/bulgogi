# Project Configuration
Bulgogi projects are managed by the `project.yaml` file.

## The `project.yaml` layout 
Bulgogi Law #1 is that everything is a target.

```yaml 
target1:
- module1
- module2 

executable:
- target1
```

In this very basic demo, it can be seen that targets are simultaneously declared and hierarchically organized according to the project's dependency structure.

### Inferencing
Targets are neither declared as dependent nor standalone - these properties are inferred by their position in the hierarchy. In the few cases that it is desireable to force an out-of-hierarchy target type, see `Target Hints`.

Based on the layout above, we can infer that `target1`, `module1`, `module2` and `executable` are all targets belonging to the project.

The `executable` depends on `target1`, which itself is composed of `module1` and `module2`. This establishes the following hierarchy:

```
executable 
    \_ target1 
        \_ module1 
        \_ module2
```

The location of the source files are yet to be defined. As the configuration scheme for this is builder-dependent, a review of bulgogi's default C builder will be demonstrated.

### Target Scope
Mappings are always private with respect to their parent.

This means the children of mappings (i.e.: that which labels are mapped _to_) are visible to each other but not to any levels above.

```yaml 
target1:
- module1
- module2 

executable:
- target1
```

At the top of this example, `module1` and `module2` are mapped to `target1`. This would scope `target1` to the document itself (global) and scope `module1` and `module2` to `target1`.

This does not mean `module1` and `module2` cannot be mapped elsewhere, but their lack of visibility means they can be re-defined differently.

Further, `target1` is then mapped to `executable`. As `target1` was previously declared in a scope that is visible to `executable`, the relationship between both `target1` entries in the document can be inferred properly.

## The Default Builder (`bulc`)
The following directions apply specifically to bulgogi's default builder, `bulc`.

### Expected Layout 
For each target, bulgogi expects one or more of the following entries:

1. A `src` entry containing lists of the source-file patterns (see `Globbing`).
2. A `inc` entry containing lists of the header-file patterns (see `Globbing#Recursive`).
3. A `pri` entry containing lists of private header-file patterns (see `Private Headers`).
4. A `dep` entry containing lists of target-dependencies needed by the target.

Absent an explicitly entry for these targets, bulc will assume or infer the following:

1. Given no entries for the target:
  - Bulgogi will first look for a root-level directory matching the target name.
  - If a matching directory is found, it will then proceed to look for `src` and `inc` directories.
  - If matching directories are found, it will automatically match the following source and header patterns:
    - `'src/*.c'`
    - `'src/inc/**.h'`
    - `'inc/**.h'`

2. Given only a `dep` target entry:
  - Bulc will assume this is an `INTERFACE` library (see [CMake `add_library#interface-libraries`](https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries)).
  - Any targets listed in `dep` must be discoverable in the global namespace of the project to be linked.
  - Otherwise, see (1) for what will happen to each unmatched entry in `dep`.

3. Given only a `src` target entry:
  - Bulc will find all matching source files in the provided pattern (see `Globbing`).

4. Given only a `inc` target entry:
  - Bulc will find all matching header files in the provided pattern (see `Globbing#Recursive`).
  - These will be registered as a header-only library (see [CMake `add_library#interface-libraries`](https://cmake.org/cmake/help/latest/command/add_library.html#interface-libraries)).
  - If no other targets depend on this one, no outputs will be produced.

5. Given only a `pri` target entry:
  - See (4) - with the caveat that any inclusion under another target's `dep` will not expose any of the headers.

6. Given a `dep` and a `src` target entry:
  - See (2) - with the following caveats:
      - If no other targets depend on this one, bulc will assume this to be an executable target.
      - If at least one other target depends on this one, bulc will assume this to be a standalone library.

7. Given a `dep` and a `inc` target entry:
  - See (4).
  - Bulc will find all matching header files in the provided `inc` pattern (see `Globbing#Recursive`).
  - Target which depend on this one will be exposed to all `dep` entry targets 

NOTE: These properties are not inherent to bulgogi itself. They are requirements imposed by `bulc`.

### Declaring Sources
Consider the following example:

```yaml 

target1:
- src:
  - 'target1/src/*.c'

target2:
- src:
  - 'target2/src/*.c'
- dep:
  - target1

```

Notice that `src` and `inc` are re-used between targets. Which is the 'real' `src` and `inc`?

#### Private Scope 
Every top-level target mapping (i.e.: the first appearance of `target1:`) is scoped to the current document. This means no other documents share `target1` as part of their namespaces.

The same goes for both instances of `src` and `inc` - they are privately scoped with respect to `target1` and `target2` separately, which means they do not clash.

If sources are re-used between different targets, a source-target can be defined for this purpose:

```yaml

target_src: 'target/src/*c'

target1:
- src:
  - target_src

target2:
- src:
  - target_src 

```

This is slightly defeatist as both targets would be the same aside from their names.

### Declaring Headers 


```yaml
name: 'My Project'
version: 'v1.0.0'

target1:
- __src__: 'target1/src/*.c'
- __inc__: 'target1/inc/**.h'

executable:
- target1 
- __src__: 'src/main.c'
```

