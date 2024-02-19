C v0.2.5

# Synopsis 
Bulgogi reads YAML files and understands them in terms of targets and dependencies.

## Everything is a target 
Every entry in the YAML file is a target.

```yaml 
---
- target1
- target2
- target3
---
```

Targets optionally have dependencies.

```yaml 
---
- target1:
  - dep1
  - dep2
  - dep3
---
```

These can nest each other until you run out of memory or UINT32 overflows.

```yaml 
---
- target1:
  - dep1:
    - subdep1:
        - subsubdep1:
            - subsubsubdep1
            - subsubsubdep2
            - subsubsubdep3
---
```

## References
Bulgogi also understands references to other targets.

```yaml 
---
- target1:
  - dep1
  - dep2
  - dep3

- target2:
  - target1
---
```

As above, so below.

```yaml 
---
- target2:
  - target1:
    - dep1
    - dep2
    - dep3
---
```

## Scope 
All target dependencies are privately scoped.

```yaml 
---
- target1:
  - src     # private
  - inc     # private

- target2:
  - src     # private 
  - inc     # private
---
```

Only root-level target declarations are publicly visible in every scope.

```yaml 
---
- target1:  # public
  - src
  - inc

- level0:
  - level1:
    - level2:
      - level3:
        - level4:
            - target1 # Reference found
---
```

## No Forward Declarations
For simplicty, readiblity and the preservation of logical order, forward declarations are not possible when using references.


```yaml 
---
- target0:
  - target1     # Private, isolated instance

- target1       # First public declaration

- target2:
  - target1     # Reference to public target1
---
```

## Core
The bulgogi core is composed of the following members:

```c 
bul_core_s {
    /** Number of targets */
    size_t size;
    /** Targets */
    bul_target_s *targets;

    /** Plus some other internally used members */
}
```

## Target 
A target is composed of the following members:

```c 
bul_target_s {
    /** Target name */
    char *name;
    /** Number of deps */
    size_t size;
    /** Unique Target ID */
    bul_id_t id;
    /** Target dependencies */
    bul_id_t *deps;

}
```
