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
- Numeric constants (integers, floats)
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
- **Division by zero** is an error (see rules below)
- **Parentheses** for explicit precedence
- **Whitespace** is ignored

## 6. Syntax and Parsing
- **Square brackets** for parameter/variable references and ranges
- **Curly braces** for inline expressions in text
- **Parentheses** for grouping
- **Infix notation** for all operators

## 7. Rules and Error Handling
- **Division by zero:** Treated as an error. The expression must not evaluate or assign a value if division by zero occurs.
- **Integer and float mix:** If an operation mixes integer and float, the result is a float.
- **Float precision:** Floats are limited to 3 digits after the decimal point (e.g., 1.234).
- **Ambiguous precedence:** Expressions with ambiguous operator precedence are not allowed and must produce an error. Parentheses are required to clarify such cases.
- **Unary minus:** Not supported. Use the `neg()` function for negation instead (e.g., `neg([p1])`).
- **Invalid expressions:** Must be detected and reported.
- **Out-of-range or undefined parameter/variable references:** Must be reported as errors.
- **Selector errors:** Must be handled (reaction to be defined).

## 9. Example Expressions
- `[p1] >= ([p2]+1) * [p15]/[p7]`
- `([p1]>=30) and ([p2]=1)`
- `([p3]>80)`
- `([p1] in (0 to [p4]) and ([p2]=1))`
- `((neg(2*3)) and (1<2))`
- `{([p5] div 30)+1}`
- `[1..64]`
- `[p2] to [p8]`
- `[1..49;51..98;100]`

