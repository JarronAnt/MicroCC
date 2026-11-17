
# MicroCC

**MicroCC** is a lightweight C compiler written from scratch in **modern C++20**, built to explore compiler architecture, language implementation, and low-level systems programming.

The project follows an incremental development approach, with each phase extending a functional compiler. MicroCC implements its own lexical analyzer, recursive-descent parser, abstract syntax tree (AST), and native assembly backend without relying on LLVM or external parser generators.

The long-term goal is to build a small but capable compiler that can compile useful real-world C programs while remaining modular, understandable, and educational.

> **Current Status: Phase 3 — Variables and Control Flow Implemented**
>
> MicroCC currently targets **AArch64 macOS (Apple Silicon)** and compiles a subset of C into native ARM64 assembly and Mach-O executables.

## Features

### Currently Implemented

#### Compiler Infrastructure

- C++20 compiler implementation
- CMake and Ninja build system
- Modular compiler frontend and backend
- Command-line compiler driver
- C source file loading
- Native AArch64 assembly generation
- Mach-O executable generation
- Assembly-only output (`-S`)
- Token inspection (`--dump-tokens`)
- Source-location-aware compiler diagnostics
- Automated integration testing with CTest

#### Lexical Analysis

- Identifiers and keywords
- Decimal integer literals
- Arithmetic operators
- Comparison and logical operators
- Bitwise operators
- Assignment and compound assignment operators
- Increment and decrement operators
- Single-character and multi-character tokens
- Longest-match operator tokenization
- Single-line (`//`) and block (`/* */`) comments
- Line and column tracking
- Lexical error detection

#### Parsing and Language Features

- Handwritten recursive-descent parser
- Abstract syntax tree construction
- Operator precedence and associativity
- Integer literals and expressions
- Unary and binary expressions
- Conditional expressions
- Comma expressions
- Local variable declarations
- Variable initialization
- Assignment expressions
- Compound assignments
- Prefix and postfix increment/decrement
- Expression statements
- Block statements
- Nested lexical scopes
- Variable shadowing
- Symbol tables and variable resolution
- Return statements
- Conditional statements (`if` / `else`)
- Loops (`while`, `do-while`, `for`)
- Loop control (`break`, `continue`)

#### Supported Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Unary | `+`, `-`, `!`, `~` |
| Relational | `<`, `<=`, `>`, `>=` |
| Equality | `==`, `!=` |
| Logical | `&&`, `\|\|`, `!` |
| Bitwise | `&`, `\|`, `^`, `~` |
| Bit Shifts | `<<`, `>>` |
| Assignment | `=` |
| Compound Assignment | `+=`, `-=`, `*=`, `/=`, `%=` |
| Bitwise Assignment | `&=`, `\|=`, `^=`, `<<=`, `>>=` |
| Increment/Decrement | `++`, `--` |
| Conditional | `?:` |
| Sequencing | `,` |
| Grouping | `(`, `)` |

Logical operators implement short-circuit evaluation, and conditional expressions evaluate only the selected branch.

#### AArch64 Code Generation

- Native ARM64 assembly generation
- Mach-O symbol naming conventions
- Function prologue and epilogue generation
- Frame pointer management
- Local variable stack allocation
- Local variable loads and stores
- Stack-based expression evaluation
- 16-byte stack alignment
- Arithmetic and bitwise instructions
- Signed integer division and remainder
- Comparison and conditional branch instructions
- Short-circuit expression evaluation
- Conditional statement branching
- Loop control-flow generation
- Nested loop handling
- Shared function return epilogues
- Automatic assembly and linking through Apple Clang

### Not Yet Implemented

- Function parameters and function calls
- Recursive functions
- Global variables
- Pointers and arrays
- String and character literals
- Additional integer types and floating-point types
- Structs, unions, enums, and typedefs
- Full C type checking and conversions
- Preprocessor directives
- Intermediate representation (IR)
- Compiler optimization passes
- Register allocation
- Additional target architectures

MicroCC currently implements a limited subset of C, rather than the complete C11 language.

---

## Quick Start

### Requirements

MicroCC currently targets **macOS on Apple Silicon**.

Requirements:

- macOS on Apple Silicon (M1 or newer)
- Apple Clang with C++20 support
- CMake 3.20+
- Ninja
- Xcode Command Line Tools

Install build tools using Homebrew:

```bash
brew install cmake ninja
```

Install Apple's command-line developer tools if necessary:

```bash
xcode-select --install
```

### Clone the Repository

```bash
git clone https://github.com/JarronAnt/MicroCC.git
cd MicroCC
```

### Configure and Build

Configure the project:

```bash
cmake -S . -B build -G Ninja
```

Build the compiler:

```bash
cmake --build build
```

The resulting executable is located at:

```text
build/microcc
```

### Compile Your First Program

Create `hello.c`:

```c
int main() {
    int result = 0;

    for (int i = 1; i <= 5; i++) {
        result += i;
    }

    return result;
}
```

Compile using MicroCC:

```bash
./build/microcc hello.c -o hello
```

Execute:

```bash
./hello
echo $?
```

Expected result:

```text
15
```

MicroCC parses the C source and generates ARM64 assembly itself. Apple Clang is used only to assemble and link the generated output.

**Note:** Shell exit statuses are limited to values between 0 and 255. Larger or negative return values cannot be fully inspected through `echo $?`.

---

## Example Programs

### 1. Variables and Assignments

MicroCC supports local integer variables, initialization, and assignment expressions.

```c
int main() {
    int a = 10;
    int b = 20;

    int result = a + b;

    result *= 2;
    result -= 15;

    return result;
}
```

Expected return value: `45`.

Multiple variables can be declared together:

```c
int main() {
    int a = 2, b = 3, c = a + b;
    return c;
}
```

Expected return value: `5`.

Assignments can also be chained:

```c
int main() {
    int a = 0, b = 0, c = 0;

    a = b = c = 7;

    return a + b + c;
}
```

Expected return value: `21`.

### 2. Conditional Statements

MicroCC supports `if`, `else`, and nested conditional statements.

```c
int main() {
    int x = 15;

    if (x > 20) {
        return 1;
    } else if (x > 10) {
        return 2;
    } else {
        return 3;
    }
}
```

Expected return value: `2`.

Conditions support all previously implemented arithmetic, comparison, logical, and bitwise expressions.

### 3. While Loops

```c
int main() {
    int i = 0;
    int sum = 0;

    while (i < 5) {
        sum += i;
        i++;
    }

    return sum;
}
```

Expected return value: `10`.

### 4. Do-While Loops

```c
int main() {
    int i = 1;
    int sum = 0;

    do {
        sum += i;
        i++;
    } while (i <= 4);

    return sum;
}
```

Expected return value: `10`.

Unlike a `while` loop, a `do-while` loop executes its body at least once.

### 5. For Loops

```c
int main() {
    int sum = 0;

    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            sum += i;
        }
    }

    return sum;
}
```

Expected return value: `20`.

Variables declared in a `for` initializer remain visible for the duration of the loop and go out of scope afterward.

### 6. Break and Continue

```c
int main() {
    int sum = 0;

    for (int i = 0; i < 10; i++) {
        if (i == 7)
            break;

        if (i % 2 != 0)
            continue;

        sum += i;
    }

    return sum;
}
```

Expected return value: `12`.

MicroCC correctly handles the different `continue` destinations for `while`, `do-while`, and `for` loops.

### 7. Nested Loops

```c
int main() {
    int count = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (j == 2)
                break;

            count++;
        }
    }

    return count;
}
```

Expected return value: `6`.

Each loop maintains its own break and continue targets, allowing nested loops to operate independently.

### 8. Lexical Scoping and Shadowing

MicroCC uses lexical scopes and supports variable shadowing.

```c
int main() {
    int x = 10;

    {
        int x = 20;
        x += 5;
    }

    return x;
}
```

Expected return value: `10`.

The inner `x` occupies a separate stack slot and does not modify the outer variable.

Declaring a variable twice in the same scope produces a compilation error.

### 9. Factorial Using a Loop

MicroCC can now compile more substantial algorithms.

```c
int main() {
    int n = 5;
    int result = 1;

    for (int i = 2; i <= n; i++) {
        result *= i;
    }

    return result;
}
```

Expected return value: `120`.

Recursive factorial will become possible in Phase 4 when function calls are introduced.

### 10. Short-Circuit Expressions With Side Effects

Phase 3 introduces assignments inside expressions, making short-circuit behavior particularly important.

```c
int main() {
    int x = 0;

    int result = (0 && (x = 5)) ||
                 (1 && (x = 8));

    return x + result;
}
```

Expected return value: `9`.

The assignment `x = 5` is skipped because the first logical AND expression short-circuits.

---

## Compiler Architecture

MicroCC follows a modular compilation pipeline.

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
        +-------------------------+
        | Recursive-Descent Parser|
        |                         |
        |  - Expression Parsing   |
        |  - Statement Parsing    |
        |  - Symbol Tables        |
        |  - Lexical Scopes       |
        +-------------------------+
                     |
                     v
          Abstract Syntax Tree
                     |
                     v
        +-------------------------+
        | AArch64 Code Generator  |
        |                         |
        |  - Expression Codegen   |
        |  - Stack Frame Layout   |
        |  - Variable Access      |
        |  - Conditional Branches |
        |  - Loop Control Flow    |
        +-------------------------+
                     |
                     v
               ARM64 Assembly
                     |
                     v
             Apple Clang Driver
             Assembler + Linker
                     |
                     v
          Native Mach-O Executable
```

### 1. Lexical Analysis

The lexer transforms source text into a sequence of tokens.

Example:

```c
int x = 5;
x += 3;
```

Token stream:

```text
KwInt           int
Identifier      x
Equal           =
IntegerLiteral  5
Semicolon       ;
Identifier      x
PlusEqual       +=
IntegerLiteral  3
Semicolon       ;
```

Each token stores:

- Token kind
- Source lexeme
- Source location
- Optional integer value

Source locations allow the compiler to report filename, line, and column information for errors.

### 2. Recursive-Descent Parsing

MicroCC uses a handwritten recursive-descent parser.

Each major expression precedence level is implemented by a dedicated parsing function.

Current precedence hierarchy, from lowest to highest:

```text
Comma               ,
Assignment          = += -= *= /= %= ...
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
Unary               + - ! ~ ++ --
Postfix             ++ --
Primary             literals, variables, ()
```

Assignment and conditional expressions are right-associative.

Most arithmetic and bitwise operators are left-associative.

The parser builds an AST rather than evaluating expressions directly.

### 3. Abstract Syntax Tree

The AST represents the logical structure of the source program.

Current expression nodes:

```text
Expr
├── IntegerLiteral
├── VariableExpr
├── UnaryExpr
├── BinaryExpr
├── ConditionalExpr
├── AssignExpr
└── UpdateExpr
```

Current statement nodes:

```text
Stmt
├── ReturnStmt
├── EmptyStmt
├── ExprStmt
├── DeclStmt
├── BlockStmt
├── IfStmt
├── WhileStmt
├── DoWhileStmt
├── ForStmt
├── BreakStmt
└── ContinueStmt
```

Program structure:

```text
Program
└── FunctionDecl
    ├── Function Name
    ├── Local Stack Size
    └── Statements
```

AST nodes use `std::unique_ptr` to manage ownership.

### 4. Symbol Tables and Lexical Scoping

Phase 3 introduces compile-time symbol resolution.

The parser maintains a stack of symbol tables:

```cpp
std::vector<
    std::unordered_map<std::string, std::size_t>
> scopes_;
```

Each symbol table maps a variable identifier to its assigned local stack offset.

When entering a new block:

```c
{
    int x = 5;
}
```

the parser creates a new lexical scope.

When leaving the block, that scope is removed.

Variable lookup searches from the innermost scope outward, allowing nested blocks to access outer variables while supporting shadowing.

For example:

```c
int x = 10;

{
    int y = x + 5;
}
```

The parser resolves `x` to its outer declaration.

Undeclared identifiers and same-scope redeclarations produce compiler errors.

This is MicroCC's first implementation of basic semantic analysis, currently integrated directly into the parser.

### 5. Local Variables and Stack Frames

Each local `int` receives a unique stack offset.

For simplicity, MicroCC currently allocates an eight-byte stack slot for every 32-bit local integer.

Stack space is reserved once when entering a function.

The total local stack allocation is rounded up to a multiple of 16 bytes to maintain stack alignment.

The backend uses:

| Register | Purpose |
|----------|---------|
| `x29` | Frame pointer |
| `x30` | Link register |
| `sp` | Stack pointer |
| `w0` | Expression results and return values |
| `w9` | Arithmetic operand |
| `w10` | Scratch register |
| `w11` | Temporary value preservation |

A function prologue resembles:

```asm
stp x29, x30, [sp, #-16]!
mov x29, sp
sub sp, sp, #16
```

Local variable access uses negative offsets from `x29`.

Example:

```asm
movz x10, #8
sub x10, x29, x10
ldr w0, [x10]
```

This loads a local variable located eight bytes below the frame pointer.

Corresponding store:

```asm
movz x10, #8
sub x10, x29, x10
str w0, [x10]
```

All returns branch to a shared function epilogue:

```asm
mov sp, x29
ldp x29, x30, [sp], #16
ret
```

This ensures that the stack frame is restored when returning from nested loops or conditionals.

### 6. Control-Flow Code Generation

MicroCC generates branch labels for conditional statements and loops.

For an `if/else` statement:

```text
Evaluate Condition
       |
       v
   Condition?
    /      \
  True     False
   |         |
Then Block  Else Block
   |         |
   +----+----+
        |
        v
   Continue
```

Loops use dedicated labels for:

- Condition evaluation
- Loop body
- Increment or update expression
- Loop exit

Nested loops use a stack of loop contexts:

```cpp
struct LoopLabels {
    std::string breakLabel;
    std::string continueLabel;
};
```

This ensures `break` and `continue` affect the innermost enclosing loop.

For `for` loops, `continue` jumps to the increment expression before returning to the condition.

For `while` and `do-while` loops, `continue` jumps to the appropriate condition check.

### 7. AArch64 Instruction Selection

MicroCC currently uses the following instruction categories:

| Instructions | Purpose |
|-------------|---------|
| `mov`, `movz`, `movk` | Register moves and integer constants |
| `add`, `sub`, `neg` | Arithmetic |
| `mul` | Multiplication |
| `sdiv`, `msub` | Division and remainder |
| `and`, `orr`, `eor`, `mvn` | Bitwise operations |
| `lsl`, `asr` | Bit shifts |
| `cmp`, `cset` | Comparisons |
| `cbz`, `cbnz` | Conditional branches |
| `b` | Unconditional branching |
| `ldr`, `str` | Memory loads and stores |
| `stp`, `ldp` | Paired register storage |
| `ret` | Function return |

The current backend favors simple, readable code generation over optimization.

Register allocation and more sophisticated instruction selection are planned for later phases.

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

### Compile an Executable

```bash
./build/microcc examples/sum_even.c -o sum_even
```

Run:

```bash
./sum_even
echo $?
```

Expected:

```text
20
```

### Generate Assembly

```bash
./build/microcc -S examples/sum_even.c -o sum_even.s
```

Inspect the generated assembly:

```bash
cat sum_even.s
```

### Inspect Tokens

```bash
./build/microcc --dump-tokens examples/sum_even.c -o sum_even
```

### Inspect Native Machine Instructions

```bash
otool -tvV sum_even
```

This disassembles the generated Mach-O executable, allowing inspection of the native ARM64 machine instructions.

---

## Testing

MicroCC uses CTest for automated compiler integration testing.

Each positive integration test performs the following:

```text
C Test Program
       |
       v
MicroCC Compilation
       |
       v
Generated ARM64 Assembly
       |
       v
Assembly and Linking
       |
       v
Native Executable
       |
       v
Program Execution
       |
       v
Expected Exit Code Comparison
```

Negative tests verify that invalid source programs produce the expected diagnostics.

### Run Tests

```bash
ctest --test-dir build --output-on-failure
```

List registered tests:

```bash
ctest --test-dir build -N
```

### Test Coverage

The Phase 3 CMake configuration registers **55 integration tests**, including all retained Phase 1, 2A, and 2B tests.

| Phase | Tests | Coverage |
|-------|------:|----------|
| Phase 1 | 1 | Compiler foundation and basic returns |
| Phase 2A | 6 | Arithmetic expression parsing |
| Phase 2B | 18 | Expanded expressions and diagnostics |
| Phase 3 | 30 | Variables, assignments, scopes, control flow, diagnostics |
| **Total** | **55** | |

### Phase 3 Positive Tests

23 positive tests cover:

- Local variables and initialization
- Assignment chaining
- Compound assignments
- Prefix and postfix operators
- Variable shadowing
- Conditional statements
- Dangling-else association
- While loops
- Do-while loops
- For loops
- Continue and break behavior
- Nested loops
- Infinite loops with explicit exits
- Assignment expressions
- Multiple declarators
- Logical short-circuiting with side effects
- Conditional expressions containing assignments
- Comma expressions in loops
- Early returns
- Nested assignments

### Phase 3 Negative Tests

Seven negative tests verify:

- Undeclared variables
- Same-scope redeclaration
- Accessing a variable outside its `for` scope
- `break` outside a loop
- `continue` outside a loop
- Invalid assignment targets
- Invalid increment/decrement targets

The full test suite should be run on Apple Silicon macOS to validate native execution of generated programs.

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
│   ├── phase2b.c
│   └── sum_even.c
│
└── tests/
    ├── cmake/
    │   ├── run_return_test.cmake
    │   └── run_failure_test.cmake
    │
    └── integration/
        ├── Phase 1 tests
        ├── Phase 2A tests
        ├── Phase 2B tests
        └── Phase 3 tests
```

---

## Development Roadmap

MicroCC is developed incrementally, with each phase expanding the supported language features and compiler infrastructure.

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

- [x] Addition and subtraction
- [x] Multiplication and division
- [x] Unary positive and negative operators
- [x] Parenthesized expressions
- [x] Operator precedence
- [x] Left associativity
- [x] Recursive expression parsing
- [x] Binary and unary AST nodes
- [x] Stack-based arithmetic code generation
- [x] Arithmetic integration tests

### Phase 2B — Expanded Expressions

**Status: Complete**

- [x] Remainder operator
- [x] Relational operators
- [x] Equality operators
- [x] Logical AND and OR
- [x] Logical NOT
- [x] Bitwise AND, OR, XOR, and complement
- [x] Left and right shifts
- [x] Conditional operator
- [x] Nested conditional expressions
- [x] Comma operator
- [x] Expanded precedence hierarchy
- [x] Short-circuit evaluation
- [x] Conditional ARM64 branches
- [x] Expanded integration tests
- [x] Negative diagnostic tests

### Phase 3 — Variables and Control Flow

**Status: Implemented — Native Test Verification Pending**

- [x] Local variable declarations
- [x] Variable initialization
- [x] Multiple declarators
- [x] Assignment expressions
- [x] Compound assignments
- [x] Prefix and postfix increment/decrement
- [x] Symbol tables
- [x] Lexical scopes
- [x] Variable shadowing
- [x] Stack slot allocation
- [x] Frame pointer-based local variable access
- [x] Basic lvalue validation
- [x] Expression statements
- [x] Block statements
- [x] Empty statements
- [x] `if` / `else`
- [x] `while`
- [x] `do-while`
- [x] `for`
- [x] `break` / `continue`
- [x] Nested loop handling
- [x] Shared function epilogue
- [x] Control-flow code generation
- [x] Expanded positive integration tests
- [x] Expanded negative diagnostic tests

### Phase 4 — Functions and Calling Conventions

**Status: Planned**

- [ ] Function declarations and definitions with parameters
- [ ] Function call expressions
- [ ] Argument evaluation and passing
- [ ] AArch64 calling convention implementation
- [ ] Register-based argument passing
- [ ] Stack-based argument passing
- [ ] Return value handling
- [ ] Recursive function calls
- [ ] Forward declarations
- [ ] External function declarations
- [ ] Function symbol resolution
- [ ] Multi-function programs
- [ ] Calling convention integration tests

Target program:

```c
int factorial(int n) {
    if (n <= 1)
        return 1;

    return n * factorial(n - 1);
}

int main() {
    return factorial(5);
}
```

Expected return value: `120`.

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
- [ ] Improved semantic diagnostics

### Phase 6 — Pointers and Arrays

- [ ] Pointer types
- [ ] Address-of operator
- [ ] Pointer dereferencing
- [ ] Pointer arithmetic
- [ ] Array types
- [ ] Array indexing
- [ ] Array-to-pointer decay
- [ ] `sizeof`
- [ ] Memory loads and stores
- [ ] Pointer-related type checking

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
- [ ] Standard C library interoperability
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
- [ ] Dominator analysis
- [ ] Liveness analysis
- [ ] Register allocation
- [ ] Optimization levels
- [ ] x86-64 Linux backend

### Long-Term Goals

- Support a practical subset of C11
- Compile increasingly complex C programs
- Compile small existing open-source C libraries
- Improve diagnostic messages
- Separate semantic analysis from parsing
- Introduce differential testing against Clang and GCC
- Add compiler fuzzing
- Develop a platform-independent IR
- Implement optimization passes
- Improve generated machine-code quality
- Support multiple target architectures
- Progress toward compiling useful real-world C projects

---

## Current Limitations

MicroCC remains an educational C subset compiler.

Current limitations include:

- Local variables are restricted to signed 32-bit `int`.
- Each local currently uses an eight-byte stack slot.
- Stack slots are not reused between non-overlapping scopes.
- Decimal integer literals must fit the positive signed 32-bit range.
- Full C type checking and integer conversions are not implemented.
- Assignment and increment/decrement currently target local variables only.
- Function calls and function parameters are not supported.
- Pointer types and arrays are not supported.
- Global variables and static storage are not supported.
- String literals and character literals are not supported.
- The preprocessor is not implemented.
- Floating-point arithmetic is not implemented.
- Generated assembly is not optimized.
- Only the AArch64 macOS target is supported.

Uninitialized local variables must not be read before receiving a value.

Signed integer overflow, division by zero, and invalid signed shift operations remain undefined behavior. The compiler does not yet provide complete semantic diagnostics for these cases.

The backend is intentionally simple and uses stack storage for local variables and temporary expression results.

These limitations will be addressed incrementally as the compiler's type system, semantic analysis, and backend evolve.

---

## Project Goals

MicroCC is designed as a hands-on compiler engineering project exploring the complete compilation pipeline.

### Compiler Frontend

- Lexical analysis
- Recursive-descent parsing
- Formal grammars
- AST design
- Operator precedence
- Symbol tables
- Lexical scoping
- Semantic analysis
- Type systems

### Systems Programming

- Modern C++20
- AArch64 assembly
- Calling conventions
- Stack frame management
- Register usage
- Memory access
- Object files and linking
- Native executable formats

### Compiler Backend

- Instruction selection
- Control-flow code generation
- Intermediate representations
- Control-flow graphs
- Optimization passes
- Register allocation
- Backend portability

### Compiler Correctness

- Unit and integration testing
- Positive and negative compilation tests
- Assembly inspection
- Differential testing
- Compiler fuzzing
- Regression testing

The emphasis is on implementing compiler components directly, understanding their behavior, and gradually improving correctness and language coverage.

The eventual objective is a compact, understandable compiler capable of compiling useful real-world C programs.

---

## License

A license will be added as the project develops.

