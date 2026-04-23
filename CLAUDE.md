# Castle of No Return — Claude Coding Rules

## 1. Always use braces on if statements

Prefer:
```c
if (!map_is_walkable(&g->map, ex, ey)) {
    continue;
}
```

Never:
```c
if (!map_is_walkable(&g->map, ex, ey)) continue;
```

This applies to all control flow: `if`, `else`, `for`, `while`.

---

## 2. Single space around operators — no alignment padding

Prefer:
```c
int a = 5;
int bc = 10;
int def = 15;
```

Never:
```c
int a   = 5;
int bc  = 10;
int def = 15;
```

This applies to variable declarations, struct field assignments, and all `=` usage.

---

## 3. Function signatures on one line

Prefer:
```c
static void handle_slot_result(SlotResult result, int slot, int slot_is_save) {
```

Never:
```c
static void handle_slot_result(SlotResult result, int slot,
    int slot_is_save) {
```

If a signature is too long, shorten the parameter names rather than wrapping.

---

## 9. How to propose and apply changes

For every change, follow this exact sequence:

1. **Describe** the change briefly in text — what file, what it does, why
2. **Call the tool** — use Edit or Write directly. Do not paste code blocks in chat as a substitute
3. **The VS Code diff prompt is the approval step** — the extension shows a diff with Yes / No / Tell Claude what to do instead. Wait for that response before proceeding
4. **Never chain tool calls without a prompt in between** — one tool call, one approval, then stop and wait

---

## 4. Minimal code per change set

- Provide the smallest possible change per step
- Wait for an explicit "next step" prompt before providing the next change
- Never combine multiple features into one code block
- Never add unrequested changes alongside a requested change

---

## 5. Explain each change

For every code change, briefly explain:
- What the change does
- Why it is needed
- Any side effects or dependencies

---

## 6. Be maximally specific about code location

When instructing where to add or modify code, always specify:
- The exact filename and path
- The exact function name
- The exact surrounding lines to find the insertion point

Prefer:

> In `c_rpg/src/game/actions.c` in `action_resolve_player` replace:
> ```c
> g->level_cleared = 0;
> ```
> with:
> ```c
> g->level_cleared = 1;
> ```

Never:

> Add this after the level cleared block.

---

## 8. Use GLOB_RECURSE for CMake source collection

Never list source files individually in `add_executable`. Always use:

```cmake
file(GLOB_RECURSE SOURCES src/*.c)
add_executable(wolf ${SOURCES})
```

This scales as new `.c` files are added without requiring CMakeLists.txt edits.

---

## 7. All game state changes must be reflected in save/load

Any new game state added or any struct that is part of persistent game state must also be:

- Serialized in `save_game()` in `src/systems/save_load.c`
- Deserialized in `load_game()` in `src/systems/save_load.c`

Omitting this causes data to silently reset to zero on load.