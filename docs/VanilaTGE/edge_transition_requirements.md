# Edge (Transition) Logic Requirements

This document summarizes the requirements for edge (transition) logic and possibilities in the text game engine, based on the provided documentation.

---

## 1. Definition and Creation
- Transitions (edges) connect two locations (nodes) in a quest.
- Multiple transitions can exist between the same pair of locations.
- Each transition can have:
  - A question or action text (e.g., "Enter the building", "Attack the guard").
  - An optional description/message displayed upon transition.
  - An order index (0–9) to control display order among available transitions.

## 2. Types of Transitions
- **Standard transitions:** Require player choice.
- **Empty transitions:** No question text; executed automatically if no other transitions are available.
- **Parallel transitions:** Multiple transitions with the same question text, possibly leading to different outcomes (used for randomization or probability).

## 3. Transition Availability
- A transition is available if all its conditions are met:
  - **General logical condition:** An expression involving quest parameters (e.g., `[p1] >= ([p2]+1) * [p15]/[p7]`).
  - **Parameter-specific conditions:** For each parameter, specify:
    - Value ranges (e.g., `[p1] in [3,5]`)
    - Values to include/exclude
    - Multiplicity (values the parameter must/must not be a multiple of)
- All logical and parameter-specific conditions must be true for the transition to be available.

## 4. Transition Passability (Traversal Limit)
- Each transition has a passability value:
  - `0` means unlimited traversals.
  - Any positive integer limits the number of times the transition can be used.
- When the traversal limit is reached, the transition becomes unavailable.

## 5. Transition Priority (Probability)
- Each transition has a priority value (can be <1, =1, >1).
- When multiple transitions are available, the probability of each being chosen is proportional to its priority.
- If one transition’s priority exceeds others by more than 100x, it is always chosen.
- Priorities can be used to create rare or common outcomes among parallel transitions.

## 6. Display and Selection
- Transitions are displayed to the player in order of their order index (lowest first).
- If multiple transitions have the same order, their display order is randomized.
- Transitions can be marked as "Always Show":
  - Displayed even if unavailable, but cannot be selected unless available (useful for hints).

## 7. Effects and Actions
- Transitions can:
  - Change quest parameters (by units, percent, fixed value, or expression).
  - Advance in-game time (e.g., "One day has passed" checkbox).
- Transitions can have custom messages shown upon execution.

## 8. Cyclic and Pseudo-cyclic Transitions
- Cycles are allowed but should be used with caution.
- If passability is 1, the player cannot traverse the same transition twice, preventing infinite loops.
- If passability >1, cycles are possible but limited.
- Empty transitions can be used for automatic multi-step progressions.

## 9. Special Features
- Transitions can be copied and reassigned between locations.
- Transitions can be moved and reordered in the editor.
- Transitions can be deleted with confirmation.

---

This requirements file summarizes all logic and possibilities related to edge (transition) handling in the text game engine, as described in the provided documentation.
