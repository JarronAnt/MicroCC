
# MicroCC

**MicroCC** is a tiny C compiler written from scratch in **C++20**. It's a learning-focused project exploring how C source code becomes native machine code—from tokenization and parsing to abstract syntax trees, assembly generation, and linking.

MicroCC is being built incrementally, with each phase producing a working compiler rather than a disconnected collection of compiler components. The long-term goal is to compile useful, real-world C programs while keeping the implementation understandable.

> **Current status: Phase 2A complete — arithmetic expression parsing and AArch64 code generation.**
>
> MicroCC currently targets **Apple Silicon macOS (AArch64/ARM64)**. It compiles a deliberately small subset of C; it is **not yet a general-purpose C compiler**.

## Quick start

### Requirements

- macOS on Apple Silicon (ARM64)
- A C++20-compatible compiler (Apple Clang via Xcode Command Line Tools)
- CMake 3.20+
- Ninja (for the commands below)

If needed, install Apple's Command Line Tools with `xcode-select --install`. CMake and Ninja can be installed with Homebrew: `brew install cmake ninja`.

### Build

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

### Compile a C program

Create `examples/math.c`:

```c
int main() {
    return (10 + 5) * 2 - 24 / (3 + 1);
}
```

Compile and run it:

```sh
./build/microcc examples/math.c -o math
./math
echo $?
# 24
```

MicroCC emits AArch64 assembly and invokes **Apple Clang only to assemble and link it**. Clang does not compile the input C source.

> **Exit status note:** `echo $?` shows an exit code (0–255), not a general-purpose integer output. The example intentionally returns a value within that range. MicroCC does not support `printf` or library function calls yet.

## Supported language features

As of **Phase 2A**, MicroCC supports:

| Category | Supported syntax / behavior |
| --- | --- |
| Translation units | Zero or more `int` functions with empty parameter lists |
| Statements | `return <expression>;` |
| Literals | Decimal integer literals in the signed 32-bit positive range (`0` through `2147483647`) |
| Arithmetic | Binary `+`, `-`, `*`, `/` |
| Unary operators | Unary `+` and `-` |
| Grouping | Parentheses `(` and `)` |
| Parsing rules | C-style arithmetic precedence and left associativity |
| Whitespace/comments | Whitespace, `//` line comments, `/* ... */` block comments |
| Diagnostics | Filename, line, and column for lexical and syntax errors |
| Output | Native AArch64 macOS executable or `.s` assembly file |

Expressions are currently supported **inside return statements**. For example:

```c
int main() {
    return 20 + -(3 + 4); // returns 13
}
```

**Not yet supported:** local variables, assignments, function parameters or calls, comparisons, control flow (`if`/`while`/`for`), pointers, arrays, strings, headers, macros, `#include`, or the rest of the C type system. Integer overflow and division by zero are not diagnosed; as in C, those cases must not be relied upon.

## How it works

```text
         C source (.c)
              |
              v
         Lexer / tokens
              |
              v
     Recursive-descent parser
              |
              v
     Abstract syntax tree (AST)
              |
              v
     AArch64 macOS codegen
              |
              v
        Assembly (.s)
              |
              v
  Apple Clang (assemble + link)
              |
              v
     Native Mach-O executable
```

### Lexer

The lexer recognizes integer literals, identifiers, keywords, punctuation, arithmetic operators, and comments. Each token retains its source location for diagnostics.

### Parser and AST

MicroCC uses a **recursive-descent parser** that builds an AST instead of evaluating expressions while parsing. Phase 2A adds `UnaryExpr` and `BinaryExpr` nodes alongside integer literals.

The current expression grammar is:

```text
expression       -> additive
additive         -> multiplicative (("+" | "-") multiplicative)*
multiplicative   -> unary (("*" | "/") unary)*
unary            -> ("+" | "-") unary | primary
primary          -> INTEGER | "(" expression ")"
```

This correctly interprets `2 + 3 * 4` as `2 + (3 * 4)` and `24 / 3 / 2` as `(24 / 3) / 2`.

### Code generation

The current backend emits AArch64 assembly for Apple Silicon. Expression results use the `w0` register; intermediate left-hand operands are saved on the stack while right-hand expressions are evaluated recursively. Arithmetic uses native instructions such as `add`, `sub`, `mul`, `sdiv`, and `neg`.

For example, this C function:

```c
int main() {
    return 2 + 3;
}
```

produces assembly resembling:

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

This is a simple, correctness-first strategy; register allocation and optimization will come later.

## CLI usage

```text
microcc [options] <input.c>

Options:
  -o <file>       Set output file
  -S              Emit assembly only
  --dump-tokens   Print lexer output
  -h, --help      Show help
```

Compile to an executable:

```sh
./build/microcc examples/math.c -o math
```

Emit and inspect assembly:

```sh
./build/microcc -S examples/math.c -o math.s
cat math.s
```

Inspect lexer output:

```sh
./build/microcc --dump-tokens examples/math.c -o math
```

Inspect machine-code disassembly (after building `math`):

```sh
otool -tvV math
```

## Testing

MicroCC uses **CTest** to run end-to-end compilation tests. These tests invoke MicroCC, run the executable it produces, and verify its exit status.

```sh
ctest --test-dir build --output-on-failure
```

Current integration cases:

| Test | Expression | Expected exit code |
| --- | --- | ---: |
| Phase 1 smoke test | `42` | 42 |
| Precedence | `2 + 3 * 4` | 14 |
| Parentheses | `(2 + 3) * 4` | 20 |
| Left associativity | `24 / 3 / 2` | 4 |
| Unary operators | `20 + -(3 + 4)` | 13 |
| Signed division | `17 / -3 + 10` | 5 |
| Nested expressions | `((2 + 3) * (9 - 5)) / 2` | 10 |

## Repository layout

```text
microcc/
├── CMakeLists.txt
├── include/microcc/
│   ├── Token.hpp
│   ├── Lexer.hpp
│   ├── AST.hpp
│   ├── Parser.hpp
│   └── codegen/AArch64MacOS.hpp
├── src/
│   ├── main.cpp
│   ├── Lexer.cpp
│   ├── Parser.cpp
│   └── codegen/AArch64MacOS.cpp
├── examples/
│   ├── return_7.c
│   └── math.c
└── tests/
    ├── cmake/run_return_test.cmake
    └── integration/
        ├── return_42.c
        ├── precedence.c
        ├── parentheses.c
        ├── associativity.c
        ├── unary.c
        ├── division.c
        └── nesting.c
```

## Roadmap

- [x] **Phase 1 — Compiler skeleton:** CMake project, lexer, basic parser and AST, `int main() { return 42; }`, AArch64 assembly, native linking, integration test.
- [x] **Phase 2A — Arithmetic expression parser:** `+`, `-`, `*`, `/`, unary signs, parentheses, precedence, associativity, recursive AArch64 evaluation, additional integration tests.
- [ ] **Phase 2B — Expanded expressions:** remainder, relational/equality, logical and bitwise operators, and conditional expressions.
- [ ] **Phase 3 — Variables and control flow:** local variables, assignments, scopes, `if`, loops, `break`, `continue`.
- [ ] **Phase 4 — Functions:** parameters, function calls, return values, ABI-compatible calling conventions.
- [ ] **Phase 5 — C types:** integer types, signedness, type checking, conversions, function/pointer/array types.
- [ ] **Phase 6 — Pointers and arrays:** dereferencing, address-of, pointer arithmetic, indexing, `sizeof`.
- [ ] **Phase 7 — Aggregate types:** structs, unions, enums, typedefs, layout and alignment.
- [ ] **Phase 8 — Global data and interoperability:** global variables, static storage, string literals, data sections, libc calls.
- [ ] **Phase 9 — Preprocessing:** macros, conditional compilation, and includes.
- [ ] **Phase 10 — Intermediate representation and optimization:** MicroCC IR, compiler passes, register allocation; eventually an x86-64 Linux backend.

The milestone beyond the roadmap is **compiling small existing C projects**, followed by more substantial real-world C programs as language coverage and compatibility improve.

## Why build MicroCC?

MicroCC is intended as a hands-on exploration of:

- Compiler frontends: lexing, parsing, AST design, and diagnostics
- C language semantics and type systems
- Assembly generation and platform calling conventions
- Build toolchains, linking, and executable formats
- Intermediate representations and optimization
- Incremental language implementation and end-to-end testing

It intentionally starts with a small feature set so each compiler stage can be understood, tested, and improved independently.

