
# MicroCC

**MicroCC** is a lightweight C compiler written from scratch in **modern C++20**, designed to explore compiler architecture, language implementation, and low-level systems programming.

The project is built incrementally, with each development phase extending a functional compiler. It covers lexical analysis, recursive-descent parsing, abstract syntax trees (ASTs), native assembly generation, and eventually intermediate representations, optimization, and register allocation.

The long-term goal is to develop a small but capable compiler that can compile real-world C programs while remaining understandable, modular, and educational.

> **Current Status: Phase 2B Complete — Expanded Expression Parsing**
>
> MicroCC currently targets **AArch64 macOS (Apple Silicon)** and compiles a subset of C directly into native ARM64 assembly.

## Features

### Currently Implemented

**Compiler Infrastructure**
- C++20 compiler implementation
- CMake and Ninja build system
- Modular frontend and backend architecture
- Command-line compiler driver
- C source file loading
- Native Mach-O executable generation
- Assembly output (`-S`)
- Token inspection (`--dump-tokens`)
- Source-location-aware compiler diagnostics
- Integration testing with CTest

**Lexical Analysis**
- Identifiers and keywords
- Decimal integer literals
- Single-character and multi-character operators
- Punctuation and grouping symbols
- Whitespace handling
- Single-line (`//`) comments
- Multi-line (`/* ... */`) comments
- Line and column tracking
- Longest-match operator tokenization

**Parsing**
- Handwritten recursive-descent parser
- Abstract syntax tree construction
- Integer-returning function definitions without parameters
- Return statements
- Operator precedence and associativity
- Parenthesized and nested expressions
- Unary and binary expressions
- Conditional expressions
- Comma expressions
- Syntax-error detection

**Supported Expression Operators**

| Category | Operators |
|----------|-----------|
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Unary | `+`, `-`, `!`, `~` |
| Relational | `<`, `<=`, `>`, `>=` |
| Equality | `==`, `!=` |
| Logical | `&&`, `\|\|`, `!` |
| Bitwise | `&`, `\|`, `^`, `~` |
| Bit Shifts | `<<`, `>>` |
| Conditional | `?:` |
| Sequencing | `,` |
| Grouping | `(`, `)` |

**Code Generation**
- Native AArch64 assembly generation
- ARM64 arithmetic instructions
- Signed integer division and remainder
- Bitwise operations and shifts
- Comparison instructions
- Conditional branching
- Short-circuit logical evaluation
- Recursive expression evaluation
- Stack-based intermediate operand storage
- 16-byte stack alignment
- Mach-O symbol naming conventions
- Automatic assembly and linking using Apple Clang

### Not Yet Implemented

- Local and global variables
- Assignments and compound assignments
- Increment/decrement operators
- `if`, `else`, `while`, and `for` statements
- Function parameters and function calls
- Pointers and arrays
- String and character literals
- Structs, unions, enums, and typedefs
- Full C type checking and conversions
- Preprocessor directives
- Intermediate representation (IR)
- Optimization passes
- Register allocation
- Additional target architectures

MicroCC is currently an educational compiler implementing a limited C subset, not a fully conforming C compiler.

---

## Quick Start

### Requirements

MicroCC currently builds and runs natively on Apple Silicon macOS.

Requirements:

- macOS on Apple Silicon (M1 or newer)
- Apple Clang with C++20 support
- CMake 3.20+
- Ninja
- Xcode Command Line Tools

Install the required build tools with Homebrew:

```bash
brew install cmake ninja
```

Install Apple's Command Line Tools if necessary:

```bash
xcode-select --install
```

### Build

Clone the repository:

```bash
git clone https://github.com/JarronAnt/MicroCC.git
cd MicroCC
```

Configure:

```bash
cmake -S . -B build -G Ninja
```

Build:

```bash
cmake --build build
```

The resulting compiler executable is located at:

```text
build/microcc
```

### Compile Your First Program

Create a file named `hello.c`:

```c
int main() {
    return 42;
}
```

Compile:

```bash
./build/microcc hello.c -o hello
```

Execute:

```bash
./hello
echo $?
```

Expected:

```text
42
```

MicroCC compiles the C source itself and generates ARM64 assembly. Apple Clang is only used afterward to assemble and link the generated output.

**Note:** Shell exit codes are limited to values from 0 to 255. Programs returning larger or negative integers cannot have their full return value inspected through `echo $?`.

---

## Expression Compilation

MicroCC supports nested integer expressions with C-style operator precedence and associativity.

### Arithmetic

```c
int main() {
    return (10 + 5) * 2 - 24 / (3 + 1);
}
```

Expected return value:

```text
24
```

### Bitwise Operations

```c
int main() {
    return (6 & 3) + (6 | 3) + (6 ^ 3);
}
```

Expected return value:

```text
14
```

### Bit Shifts

```c
int main() {
    return (3 << 3) + (32 >> 2);
}
```

Expected return value:

```text
32
```

### Logical Expressions

```c
int main() {
    return (5 > 3) && (10 != 4);
}
```

Expected return value:

```text
1
```

Logical operations return `0` for false and `1` for true.

### Short-Circuit Evaluation

Logical AND and OR correctly implement C's short-circuit evaluation rules.

```c
int main() {
    return (0 && (10 / 0)) || (1 && 5);
}
```

Expected return value:

```text
1
```

The right-hand side of `&&` is skipped when its left operand is false.

Similarly, the right-hand side of `||` is skipped when its left operand is true.

This is implemented through conditional branches in the generated assembly rather than by eagerly evaluating both operands.

### Conditional Expressions

MicroCC supports C's ternary conditional operator:

```c
int main() {
    return (5 > 3) ? 42 : 0;
}
```

Expected return value:

```text
42
```

Nested conditional expressions are also supported:

```c
int main() {
    return 0 ? 1 : 1 ? 7 : 9;
}
```

Expected return value:

```text
7
```

Conditional expressions are right-associative, and only the selected result branch is evaluated.

### Comma Expressions

```c
int main() {
    return (10, 3 + 4) * 2;
}
```

Expected return value:

```text
14
```

The left operand is evaluated and its result discarded. The right operand determines the expression's value.

### Combining Operators

```c
int main() {
    return (
        ((5 * 4) == 20) &&
        ((16 >> 2) == 4) &&
        ((7 & 3) != 0)
    ) ? 42 : 0;
}
```

Expected return value:

```text
42
```

---

## Compiler Architecture

MicroCC currently uses a straightforward frontend-to-backend compilation pipeline.

```text
              C Source Code
                    |
                    v
             +-------------+
             |    Lexer    |
             +-------------+
                    |
                    v
                 Tokens
                    |
                    v
             +-------------+
             |   Parser    |
             +-------------+
                    |
                    v
           Abstract Syntax Tree
                    |
                    v
        +-----------------------+
        | AArch64 Code Generator|
        +-----------------------+
                    |
                    v
             ARM64 Assembly
                    |
                    v
         Apple Clang Assembler
                  / Linker
                    |
                    v
          Native Mach-O Binary
```

### 1. Lexical Analysis

The lexer transforms a stream of source characters into tokens.

Example input:

```c
return 2 + 3 * 4;
```

Token stream:

```text
KwReturn        return
IntegerLiteral  2
Plus            +
IntegerLiteral  3
Star            *
IntegerLiteral  4
Semicolon       ;
```

Each token contains:

- Token kind
- Source lexeme
- Source location
- Optional integer value

Source locations are retained for compiler diagnostics.

### 2. Recursive-Descent Parsing

MicroCC uses a handwritten recursive-descent parser rather than external parser generators.

Each major operator precedence level is represented by its own parsing function.

Current precedence hierarchy, from lowest to highest:

```text
Comma               ,
Conditional         ?:
Logical OR          ||
Logical AND         &&
Bitwise OR          |
Bitwise XOR         ^
Bitwise AND         &
Equality            == !=
Relational          < <= > >=
Bit Shifts          << >>
Additive            + -
Multiplicative      * / %
Unary               + - ! ~
Primary             literals, ()
```

This ensures that:

```c
2 + 3 * 4
```

is parsed as:

```text
2 + (3 * 4)
```

rather than:

```text
(2 + 3) * 4
```

Operators at the same precedence level are generally left-associative. Conditional expressions are right-associative.

### 3. Abstract Syntax Tree

The parser produces an AST representing program structure.

Currently implemented expression nodes:

```text
Expr
├── IntegerLiteral
├── UnaryExpr
├── BinaryExpr
└── ConditionalExpr
```

Statement and declaration nodes:

```text
Program
└── FunctionDecl
    └── ReturnStmt
        └── Expr
```

Example:

```c
int main() {
    return 2 + 3 * 4;
}
```

AST representation:

```text
Program
└── FunctionDecl: main
    └── ReturnStmt
        └── BinaryExpr: Add
            ├── IntegerLiteral: 2
            └── BinaryExpr: Multiply
                ├── IntegerLiteral: 3
                └── IntegerLiteral: 4
```

AST nodes use C++ ownership semantics through `std::unique_ptr`.

### 4. AArch64 Code Generation

MicroCC currently generates ARM64 assembly targeting macOS.

Expression results are stored in the `w0` register.

Binary expressions use a simple stack-based evaluation strategy:

1. Evaluate the left operand into `w0`.
2. Save the result in a 16-byte stack slot.
3. Evaluate the right operand into `w0`.
4. Restore the left operand into `w9`.
5. Perform the operation and place the result in `w0`.

For example:

```c
int main() {
    return 2 + 3;
}
```

Generated assembly resembles:

```asm
.text

.globl _main
.p2align 2
_main:
    movz w0, #2

    sub sp, sp, #16
    str w0, [sp]

    movz w0, #3

    ldr w9, [sp]
    add sp, sp, #16

    add w0, w9, w0
    ret
```

AArch64 instructions currently used include:

| Instruction | Purpose |
|-------------|---------|
| `movz`, `movk` | Integer constant loading |
| `add`, `sub` | Arithmetic |
| `mul` | Multiplication |
| `sdiv` | Signed division |
| `msub` | Remainder calculation |
| `neg` | Unary negation |
| `and`, `orr`, `eor` | Bitwise operations |
| `mvn` | Bitwise complement |
| `lsl`, `asr` | Bit shifts |
| `cmp`, `cset` | Comparisons |
| `cbz`, `cbnz` | Conditional branches |
| `b` | Unconditional branch |
| `str`, `ldr` | Stack storage and retrieval |
| `ret` | Function return |

Logical operators and conditional expressions use branch labels to implement short-circuit evaluation.

The backend currently prioritizes correctness and simplicity over performance. Optimization and register allocation are planned for later phases.

---

## Command-Line Usage

```text
microcc [options] <input.c>
```

Available options:

| Option | Description |
|--------|-------------|
| `-o <file>` | Specify output file |
| `-S` | Generate assembly without linking |
| `--dump-tokens` | Display lexer output |
| `-h`, `--help` | Display usage information |

### Generate an Executable

```bash
./build/microcc examples/phase2b.c -o phase2b
```

Run:

```bash
./phase2b
echo $?
```

Expected return value:

```text
24
```

### Emit Assembly

```bash
./build/microcc -S examples/phase2b.c -o phase2b.s
```

Inspect the generated output:

```bash
cat phase2b.s
```

### Inspect Tokens

```bash
./build/microcc --dump-tokens examples/phase2b.c -o phase2b
```

### Inspect Native Instructions

After compiling a program:

```bash
otool -tvV phase2b
```

This displays the disassembled ARM64 machine code from the generated Mach-O executable.

---

## Testing

MicroCC uses CTest for end-to-end integration tests.

The tests exercise the entire compilation pipeline:

```text
C Source
   |
   v
MicroCC
   |
   v
ARM64 Assembly
   |
   v
Assembler / Linker
   |
   v
Executable
   |
   v
Execution and Exit Status Verification
```

Run the complete test suite:

```bash
ctest --test-dir build --output-on-failure
```

### Current Test Coverage

The Phase 2B CMake configuration registers **26 integration tests**.

| Phase | Tests | Coverage |
|-------|-------|----------|
| Phase 1 | 1 | Minimal compilation and return values |
| Phase 2A | 6 | Arithmetic, precedence, parentheses, unary operators |
| Phase 2B | 16 | Expanded operators, logical expressions, shifts, conditional expressions |
| Phase 2B Diagnostics | 3 | Expected syntax and unsupported-feature errors |
| **Total** | **26** | |

Tests include:

- Arithmetic precedence
- Parenthesized expressions
- Left associativity
- Unary expressions
- Signed division and remainder
- Relational comparisons
- Equality comparisons
- Bitwise operations
- Operator precedence across categories
- Left and right shifts
- Logical operations
- Short-circuit evaluation
- Conditional expressions
- Nested conditional expressions
- Comma expressions
- Combined expressions
- Missing punctuation diagnostics
- Unsupported assignment diagnostics

Positive integration tests compare the compiled program's exit status against the expected result.

Negative tests verify that invalid source programs are rejected with appropriate diagnostic messages.

---

## Project Structure

```text
MicroCC/
├── CMakeLists.txt
├── README.md
├── .gitignore
│
├── include/
│   └── microcc/
│       ├── Token.hpp
│       ├── Lexer.hpp
│       ├── AST.hpp
│       ├── Parser.hpp
│       └── codegen/
│           └── AArch64MacOS.hpp
│
├── src/
│   ├── main.cpp
│   ├── Lexer.cpp
│   ├── Parser.cpp
│   └── codegen/
│       └── AArch64MacOS.cpp
│
├── examples/
│   ├── return_7.c
│   ├── math.c
│   └── phase2b.c
│
└── tests/
    ├── cmake/
    │   ├── run_return_test.cmake
    │   └── run_failure_test.cmake
    │
    └── integration/
        ├── return_42.c
        ├── precedence.c
        ├── parentheses.c
        ├── associativity.c
        ├── unary.c
        ├── division.c
        ├── nesting.c
        ├── modulo.c
        ├── comparisons.c
        ├── equality.c
        ├── bitwise.c
        ├── bitwise_precedence.c
        ├── shifts.c
        ├── shift_precedence.c
        ├── logical.c
        ├── short_circuit.c
        ├── ternary.c
        ├── ternary_nested.c
        ├── ternary_middle_comma.c
        ├── comma.c
        ├── bitwise_not.c
        ├── unary_chain.c
        ├── comprehensive.c
        ├── missing_colon.c
        ├── missing_parenthesis.c
        └── assignment_not_supported.c
```

---

## Development Roadmap

MicroCC is being developed in incremental phases, each introducing a new part of the C language or compiler infrastructure.

### Phase 1 — Compiler Foundation

**Status: Complete**

- [x] CMake project setup
- [x] C++20 compiler implementation
- [x] Command-line compiler driver
- [x] Lexer and token representation
- [x] Source-location tracking
- [x] Recursive-descent parser
- [x] Initial AST
- [x] Integer-returning functions
- [x] Return statements
- [x] AArch64 assembly generation
- [x] Native executable generation
- [x] End-to-end integration testing

### Phase 2A — Arithmetic Expressions

**Status: Complete**

- [x] Binary addition and subtraction
- [x] Multiplication and division
- [x] Unary positive and negative operators
- [x] Parenthesized expressions
- [x] Operator precedence
- [x] Left associativity
- [x] Binary and unary AST nodes
- [x] Recursive arithmetic code generation
- [x] Stack-based operand preservation
- [x] Arithmetic integration tests

### Phase 2B — Expanded Expressions

**Status: Implementation Complete**

- [x] Remainder operator
- [x] Relational operators
- [x] Equality operators
- [x] Logical AND and OR
- [x] Logical NOT
- [x] Bitwise AND, OR, and XOR
- [x] Bitwise complement
- [x] Left and right shifts
- [x] Conditional operator
- [x] Nested conditional expressions
- [x] Comma operator
- [x] Complete supported-operator precedence hierarchy
- [x] Short-circuit evaluation
- [x] Conditional ARM64 branches
- [x] Expanded integration tests
- [x] Negative diagnostic tests

### Phase 3 — Variables and Control Flow

**Status: Planned**

- [ ] Local variable declarations
- [ ] Variable initialization
- [ ] Assignments
- [ ] Compound assignments
- [ ] Prefix and postfix increment/decrement
- [ ] Symbol tables
- [ ] Lexical scopes
- [ ] Local stack storage
- [ ] Lvalue and rvalue handling
- [ ] Expression statements
- [ ] Block statements
- [ ] `if` / `else`
- [ ] `while`
- [ ] `do` / `while`
- [ ] `for`
- [ ] `break` / `continue`
- [ ] Control-flow code generation

Target program:

```c
int main() {
    int sum = 0;

    for (int i = 0; i < 10; i = i + 1) {
        if (i % 2 == 0) {
            sum = sum + i;
        }
    }

    return sum;
}
```

Expected return value: `20`.

### Phase 4 — Functions and Calling Conventions

- [ ] Function parameters
- [ ] Function calls
- [ ] Argument evaluation
- [ ] Local function scopes
- [ ] Stack frames
- [ ] AArch64 calling conventions
- [ ] Recursive functions
- [ ] Return value handling
- [ ] External function declarations

### Phase 5 — C Type System

- [ ] Built-in integer types
- [ ] Signed and unsigned integers
- [ ] `char`, `short`, `int`, `long`
- [ ] `void`
- [ ] Type representation
- [ ] Type checking
- [ ] Integer promotions
- [ ] Implicit conversions
- [ ] Explicit casts
- [ ] Function types

### Phase 6 — Pointers and Arrays

- [ ] Pointer types
- [ ] Address-of operator
- [ ] Pointer dereferencing
- [ ] Pointer arithmetic
- [ ] Array types
- [ ] Array indexing
- [ ] Array-to-pointer conversion
- [ ] `sizeof`
- [ ] Memory loads and stores

### Phase 7 — Aggregate Types

- [ ] Struct definitions
- [ ] Struct member access
- [ ] Unions
- [ ] Enums
- [ ] Typedef declarations
- [ ] Type alignment
- [ ] Struct padding and layout
- [ ] Nested aggregate types

### Phase 8 — Global Storage and Interoperability

- [ ] Global variables
- [ ] Static storage
- [ ] String literals
- [ ] Character literals
- [ ] Data sections
- [ ] Read-only data sections
- [ ] Uninitialized storage
- [ ] External symbols
- [ ] C standard library interoperability
- [ ] Multi-file linking

### Phase 9 — Preprocessor

- [ ] `#include`
- [ ] Object-like macros
- [ ] Function-like macros
- [ ] `#define` and `#undef`
- [ ] Conditional compilation
- [ ] Macro expansion
- [ ] Variadic macros
- [ ] Stringification
- [ ] Token concatenation
- [ ] Built-in macros

### Phase 10 — Intermediate Representation and Optimization

- [ ] MicroCC intermediate representation
- [ ] Basic blocks
- [ ] Control-flow graphs
- [ ] IR lowering
- [ ] Constant folding
- [ ] Constant propagation
- [ ] Dead code elimination
- [ ] Copy propagation
- [ ] Common subexpression elimination
- [ ] Liveness analysis
- [ ] Register allocation
- [ ] Optimization levels
- [ ] x86-64 Linux backend

### Long-Term Goals

- Compile increasingly complex C programs
- Support a practical subset of C11
- Improve diagnostic messages and source reporting
- Add differential testing against Clang and GCC
- Introduce compiler fuzzing
- Compile small existing open-source C libraries
- Expand to larger real-world C programs
- Support multiple target architectures
- Improve code generation quality and performance

---

## Current Limitations

MicroCC is still in its early stages.

The current implementation:

- Operates primarily on signed 32-bit integer expressions.
- Accepts decimal integer literals up to `2147483647`.
- Does not yet support the `-2147483648` literal expression.
- Does not perform full C type checking.
- Does not implement local variables or assignments.
- Does not support function calls or parameters.
- Does not implement the preprocessor.
- Does not support floating-point arithmetic.
- Does not optimize generated assembly.
- Targets Apple Silicon macOS only.

As in C, signed integer overflow and division by zero are undefined behavior. The current implementation does not diagnose these cases.

Shift counts outside the valid 32-bit range and invalid signed left shifts are also not supported as defined operations. Signed right shifts are currently implemented using AArch64 arithmetic right shifts.

Some C constructs are intentionally excluded until the corresponding language and type-system phases are implemented.

---

## Project Goals

MicroCC is a hands-on compiler construction project focused on understanding the complete compilation pipeline.

Key areas of study include:

**Language Implementation**
- Lexical analysis
- Formal grammars
- Parsing algorithms
- Abstract syntax trees
- Operator precedence
- Semantic analysis
- Type systems

**Systems Programming**
- AArch64 assembly
- Calling conventions
- Stack management
- Machine-level arithmetic
- Object files and linking
- Native executable formats

**Compiler Engineering**
- Intermediate representations
- Control-flow analysis
- Optimization algorithms
- Register allocation
- Backend design
- Cross-platform code generation
- Compiler correctness testing

The emphasis is on implementing compiler components directly rather than relying on LLVM or external parser generators.

The eventual objective is a compact, understandable compiler capable of compiling useful real-world C programs.

---

## License

A license will be added as the project develops.

