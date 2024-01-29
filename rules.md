**rules.md**

# Rules & Patterns

If it already was not confusing to have to write a meta-build-system configuration for each project, `rules.yaml` takes abstraction a step further.

In brief, the purpose of writing a `rules.yaml` scheme is to define valid project structures. If `project.yaml` does not match the pattern, it is invalid.

Logically, `rules.yaml` are written in tandem with the `builder` so that valid projects build correctly. 

## Syntax
In its most ideal form, `rules.yaml` closely resembles the `project.yaml` in structure and nature. The `rules.yaml` can be thought of as a map or template for many `project.yaml`.

```yaml
--- # rules.yaml
lib:
- lib 
- dir 

exe:
- lib 
- dir 
- exedir
---

--- # project.yaml 
libmodule1:
- src 
```

Because an arbitrary set of potential project structures must be representable, the syntax includes shorthand notation for patterns which would be tediuous or impossible to represent directly.

- (`*`): Wildcard - Matches any symbol at that level.
- (`**`): Recursive wildcard - Matches any symbol recursively.
- (`..`): Vinculum - Repeat indefinitely at that level.
