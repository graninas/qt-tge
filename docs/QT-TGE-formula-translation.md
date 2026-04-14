# Formula Translator Requirements for Qt-TGE

This document outlines the requirements and design considerations for the formula translator component in the Qt-TGE text game engine and editor.

---

## 1. Purpose
- The formula translator parses, validates, and evaluates mathematical and logical expressions used throughout the game engine.
- It is used for:
  - Transition (edge) conditions
  - Parameter and variable value assignments
  - Display formatting (inline expressions in text)
  - Selector logic for choosing descriptions

## 2. Supported Operations
- **Arithmetic:** `+`, `-`, `*`, `/`, `div`, `mod`
- **Comparison:** `=`, `<>`, `<`, `>`, `<=`, `>=`
- **Logical:** `and`, `or`
- **Range and set:** `to`, `in`
- **Parentheses** for grouping and precedence

## 3. Operands
- Numeric constants (integers, possibly floats)
- Parameter and variable references: `[p1]`, `[p2]`, `[varName]`, etc.
- Ranges: `[1..7]`, `[1..49;51..98;100]`
- Parameter-based ranges: `[p2] to [p8]`

## 4. Expression Types and Contexts
- **Transition availability:** General and parameter-specific conditions (must evaluate to boolean)
- **Parameter/variable value changes:** (must evaluate to a number)
- **Display formatting:** Inline expressions in curly braces (e.g., `{([p5] div 30)+1}`)
- **Selector logic:** Used to pick a description from a pack

## 5. Special Features
- **Random value selection** from ranges (e.g., `[1..64]` picks a random value in range)
- **Set exclusion** (e.g., `[1..49;51..98;100]` excludes 50 and 99)
- **Logical expressions** return 1 for true, 0 for false (usable in arithmetic)
- **Division by zero** sets parameter to its maximum possible value
- **Parentheses** for explicit precedence
- **Whitespace** is ignored

## 6. Syntax and Parsing
- **Square brackets** for parameter/variable references and ranges
- **Curly braces** for inline expressions in text
- **Parentheses** for grouping
- **Infix notation** for all operators

## 7. Error Handling
- Invalid expressions must be detected and reported
- Division by zero must be handled gracefully
- Out-of-range or undefined parameter/variable references must be reported
- Selector errors must be handled (reaction to be defined)

## 8. Extensibility
- The system should allow for easy addition of new operators or functions in the future

## 9. Example Expressions
- `[p1] >= ([p2]+1) * [p15]/[p7]`
- `([p1]>=30) and ([p2]=1)`
- `([p3]>80)`
- `([p1] in (0 to [p4]) and ([p2]=1))`
- `(([-2*3]) and (1<2))`
- `{([p5] div 30)+1}`
- `[1..64]`
- `[p2] to [p8]`
- `[1..49;51..98;100]`

---

This requirements file is based on the analysis of the original game engine documentation and the Qt-TGE design document. It should serve as a foundation for the design and implementation of the formula translation and evaluation subsystem.
