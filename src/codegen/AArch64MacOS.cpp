
#include "microcc/codegen/AArch64MacOS.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace microcc {

namespace {

// ============================================================
// Unique assembly labels
// ============================================================

std::string label(
    std::size_t& next,
    const char* category
)
{
    return "Lmicrocc_" +
           std::string(category) +
           "_" +
           std::to_string(next++);
}

// ============================================================
// Stack allocation
//
// Split larger stack allocations into encodable,
// 16-byte-aligned AArch64 immediate chunks.
// ============================================================

void emitAdjustStack(
    std::ostringstream& out,
    std::size_t bytes
)
{
    while (bytes != 0) {
        const auto chunk =
            bytes > 4080 ? 4080 : bytes;

        out << "    sub sp, sp, #"
            << chunk << '\n';

        bytes -= chunk;
    }
}

} // namespace

// ============================================================
// Generate an entire program
// ============================================================

std::string AArch64MacOSCodeGenerator::generate(
    const Program& program
) const
{
    std::ostringstream out;
    std::size_t nextLabel = 0;

    out << ".text\n\n";

    for (const auto& fn : program.functions) {
        emitFunction(out, *fn, nextLabel);
        out << '\n';
    }

    return out.str();
}

// ============================================================
// Function code generation
// ============================================================

void AArch64MacOSCodeGenerator::emitFunction(
    std::ostringstream& out,
    const FunctionDecl& function,
    std::size_t& nextLabel
) const
{
    const auto end = label(nextLabel, "fn_end");

    // Mach-O uses an underscore prefix.
    const std::string symbol = "_" + function.name;

    out << ".globl " << symbol << '\n';
    out << ".p2align 2\n";
    out << symbol << ":\n";

    // --------------------------------------------------------
    // Function prologue
    // --------------------------------------------------------

    // Save frame pointer and link register.
    out << "    stp x29, x30, [sp, #-16]!\n";

    // Establish the frame pointer.
    out << "    mov x29, sp\n";

    // Reserve all local variable slots.
    emitAdjustStack(out, function.stackSize);

    // --------------------------------------------------------
    // Function body
    // --------------------------------------------------------

    std::vector<LoopLabels> loops;

    for (const auto& stmt : function.body) {
        emitStatement(
            out,
            *stmt,
            nextLabel,
            end,
            loops
        );
    }

    // Falling off main returns zero.
    //
    // Other non-void functions reaching the end have
    // undefined return values in C. We emit zero as
    // a safe Phase 3 placeholder.
    out << "    mov w0, #0\n";

    // --------------------------------------------------------
    // Shared function epilogue
    // --------------------------------------------------------

    out << end << ":\n";
    out << "    mov sp, x29\n";
    out << "    ldp x29, x30, [sp], #16\n";
    out << "    ret\n";
}

// ============================================================
// Statement code generation
// ============================================================

void AArch64MacOSCodeGenerator::emitStatement(
    std::ostringstream& out,
    const Stmt& stmt,
    std::size_t& nextLabel,
    const std::string& returnLabel,
    std::vector<LoopLabels>& loops
) const
{
    // --------------------------------------------------------
    // Return statement
    // --------------------------------------------------------

    if (const auto* ret =
            dynamic_cast<const ReturnStmt*>(&stmt)) {

        emitExpression(
            out,
            *ret->expression,
            nextLabel
        );

        // Return value is in w0.
        out << "    b " << returnLabel << '\n';
        return;
    }

    // --------------------------------------------------------
    // Empty statement
    // --------------------------------------------------------

    if (dynamic_cast<const EmptyStmt*>(&stmt))
        return;

    // --------------------------------------------------------
    // Expression statement
    // --------------------------------------------------------

    if (const auto* expr =
            dynamic_cast<const ExprStmt*>(&stmt)) {

        emitExpression(
            out,
            *expr->expression,
            nextLabel
        );

        return;
    }

    // --------------------------------------------------------
    // Local declarations
    // --------------------------------------------------------

    if (const auto* decl =
            dynamic_cast<const DeclStmt*>(&stmt)) {

        for (const auto& var : decl->variables) {
            if (var.initializer) {
                emitExpression(
                    out,
                    *var.initializer,
                    nextLabel
                );

                emitStoreLocal(out, var.offset);
            }
        }

        return;
    }

    // --------------------------------------------------------
    // Block statements
    // --------------------------------------------------------

    if (const auto* block =
            dynamic_cast<const BlockStmt*>(&stmt)) {

        for (const auto& child : block->statements) {
            emitStatement(
                out,
                *child,
                nextLabel,
                returnLabel,
                loops
            );
        }

        return;
    }

    // --------------------------------------------------------
    // If / Else
    // --------------------------------------------------------

    if (const auto* ifStmt =
            dynamic_cast<const IfStmt*>(&stmt)) {

        const auto elseLabel =
            label(nextLabel, "if_else");

        const auto endLabel =
            label(nextLabel, "if_end");

        emitExpression(
            out,
            *ifStmt->condition,
            nextLabel
        );

        // Skip then-branch when condition == 0.
        out << "    cbz w0, "
            << elseLabel << '\n';

        emitStatement(
            out,
            *ifStmt->thenBranch,
            nextLabel,
            returnLabel,
            loops
        );

        out << "    b " << endLabel << '\n';

        out << elseLabel << ":\n";

        if (ifStmt->elseBranch) {
            emitStatement(
                out,
                *ifStmt->elseBranch,
                nextLabel,
                returnLabel,
                loops
            );
        }

        out << endLabel << ":\n";
        return;
    }

    // --------------------------------------------------------
    // While
    // --------------------------------------------------------

    if (const auto* loop =
            dynamic_cast<const WhileStmt*>(&stmt)) {

        const auto testLabel =
            label(nextLabel, "while_test");

        const auto endLabel =
            label(nextLabel, "while_end");

        // Continue jumps to the condition.
        out << testLabel << ":\n";

        emitExpression(
            out,
            *loop->condition,
            nextLabel
        );

        out << "    cbz w0, "
            << endLabel << '\n';

        loops.push_back({
            endLabel,
            testLabel
        });

        emitStatement(
            out,
            *loop->body,
            nextLabel,
            returnLabel,
            loops
        );

        loops.pop_back();

        out << "    b " << testLabel << '\n';
        out << endLabel << ":\n";

        return;
    }

    // --------------------------------------------------------
    // Do-While
    // --------------------------------------------------------

    if (const auto* loop =
            dynamic_cast<const DoWhileStmt*>(&stmt)) {

        const auto bodyLabel =
            label(nextLabel, "do_body");

        const auto testLabel =
            label(nextLabel, "do_test");

        const auto endLabel =
            label(nextLabel, "do_end");

        out << bodyLabel << ":\n";

        loops.push_back({
            endLabel,
            testLabel
        });

        emitStatement(
            out,
            *loop->body,
            nextLabel,
            returnLabel,
            loops
        );

        loops.pop_back();

        // Continue jumps here, not to the body.
        out << testLabel << ":\n";

        emitExpression(
            out,
            *loop->condition,
            nextLabel
        );

        out << "    cbnz w0, "
            << bodyLabel << '\n';

        out << endLabel << ":\n";
        return;
    }

    // --------------------------------------------------------
    // For
    // --------------------------------------------------------

    if (const auto* loop =
            dynamic_cast<const ForStmt*>(&stmt)) {

        const auto testLabel =
            label(nextLabel, "for_test");

        const auto stepLabel =
            label(nextLabel, "for_step");

        const auto endLabel =
            label(nextLabel, "for_end");

        // Initialization runs once.
        if (loop->initializer) {
            emitStatement(
                out,
                *loop->initializer,
                nextLabel,
                returnLabel,
                loops
            );
        }

        // Condition runs before each iteration.
        out << testLabel << ":\n";

        if (loop->condition) {
            emitExpression(
                out,
                *loop->condition,
                nextLabel
            );

            out << "    cbz w0, "
                << endLabel << '\n';
        }

        // Loop body.
        loops.push_back({
            endLabel,
            stepLabel
        });

        emitStatement(
            out,
            *loop->body,
            nextLabel,
            returnLabel,
            loops
        );

        loops.pop_back();

        // Continue must execute the increment.
        out << stepLabel << ":\n";

        if (loop->increment) {
            emitExpression(
                out,
                *loop->increment,
                nextLabel
            );
        }

        out << "    b " << testLabel << '\n';
        out << endLabel << ":\n";

        return;
    }

    // --------------------------------------------------------
    // Break
    // --------------------------------------------------------

    if (dynamic_cast<const BreakStmt*>(&stmt)) {
        if (loops.empty()) {
            throw std::runtime_error(
                "break outside loop"
            );
        }

        out << "    b "
            << loops.back().breakLabel
            << '\n';

        return;
    }

    // --------------------------------------------------------
    // Continue
    // --------------------------------------------------------

    if (dynamic_cast<const ContinueStmt*>(&stmt)) {
        if (loops.empty()) {
            throw std::runtime_error(
                "continue outside loop"
            );
        }

        out << "    b "
            << loops.back().continueLabel
            << '\n';

        return;
    }

    throw std::runtime_error(
        "unsupported statement in AArch64 backend"
    );
}

// ============================================================
// Expression code generation
// ============================================================

void AArch64MacOSCodeGenerator::emitExpression(
    std::ostringstream& out,
    const Expr& expr,
    std::size_t& nextLabel
) const
{
    // --------------------------------------------------------
    // Integer literals
    // --------------------------------------------------------

    if (const auto* integer =
            dynamic_cast<const IntegerLiteral*>(&expr)) {

        emitIntegerLiteral(out, integer->value);
        return;
    }

    // --------------------------------------------------------
    // Variable access
    // --------------------------------------------------------

    if (const auto* var =
            dynamic_cast<const VariableExpr*>(&expr)) {

        emitLoadLocal(out, var->offset);
        return;
    }

    // --------------------------------------------------------
    // Prefix and postfix ++ / --
    // --------------------------------------------------------

    if (const auto* update =
            dynamic_cast<const UpdateExpr*>(&expr)) {

        emitLoadLocal(out, update->offset);

        // Postfix must return the original value.
        if (!update->prefix) {
            out << "    mov w11, w0\n";
        }

        out << (
            update->increment
                ? "    add w0, w0, #1\n"
                : "    sub w0, w0, #1\n"
        );

        emitStoreLocal(out, update->offset);

        if (!update->prefix) {
            out << "    mov w0, w11\n";
        }

        return;
    }

    // --------------------------------------------------------
    // Assignment expressions
    // --------------------------------------------------------

    if (const auto* assign =
            dynamic_cast<const AssignExpr*>(&expr)) {

        // Evaluate the right-hand side.
        emitExpression(
            out,
            *assign->value,
            nextLabel
        );

        // Simple assignment: x = value
        if (assign->op == AssignOperator::Set) {
            emitStoreLocal(out, assign->offset);
            return;
        }

        // Compound assignments:
        //
        // Save RHS, load old LHS, compute, then store.

        out << "    sub sp, sp, #16\n";
        out << "    str w0, [sp]\n";

        emitLoadLocal(out, assign->offset);

        // Old left value.
        out << "    mov w9, w0\n";

        // Restore RHS.
        out << "    ldr w0, [sp]\n";
        out << "    add sp, sp, #16\n";

        BinaryOperator op;

        switch (assign->op) {
        case AssignOperator::Add:
            op = BinaryOperator::Add;
            break;

        case AssignOperator::Subtract:
            op = BinaryOperator::Subtract;
            break;

        case AssignOperator::Multiply:
            op = BinaryOperator::Multiply;
            break;

        case AssignOperator::Divide:
            op = BinaryOperator::Divide;
            break;

        case AssignOperator::Remainder:
            op = BinaryOperator::Remainder;
            break;

        case AssignOperator::BitwiseAnd:
            op = BinaryOperator::BitwiseAnd;
            break;

        case AssignOperator::BitwiseOr:
            op = BinaryOperator::BitwiseOr;
            break;

        case AssignOperator::BitwiseXor:
            op = BinaryOperator::BitwiseXor;
            break;

        case AssignOperator::ShiftLeft:
            op = BinaryOperator::ShiftLeft;
            break;

        case AssignOperator::ShiftRight:
            op = BinaryOperator::ShiftRight;
            break;

        case AssignOperator::Set:
            throw std::runtime_error(
                "invalid compound assignment"
            );
        }

        emitBinaryOperation(out, op);

        // The assigned value remains in w0.
        emitStoreLocal(out, assign->offset);
        return;
    }

    // --------------------------------------------------------
    // Unary expressions
    // --------------------------------------------------------

    if (const auto* unary =
            dynamic_cast<const UnaryExpr*>(&expr)) {

        emitExpression(
            out,
            *unary->operand,
            nextLabel
        );

        switch (unary->op) {
        case UnaryOperator::Plus:
            break;

        case UnaryOperator::Negate:
            out << "    neg w0, w0\n";
            break;

        case UnaryOperator::LogicalNot:
            out << "    cmp w0, #0\n";
            out << "    cset w0, eq\n";
            break;

        case UnaryOperator::BitwiseNot:
            out << "    mvn w0, w0\n";
            break;
        }

        return;
    }

    // --------------------------------------------------------
    // Conditional ?:
    // --------------------------------------------------------

    if (const auto* cond =
            dynamic_cast<const ConditionalExpr*>(&expr)) {

        const auto elseLabel =
            label(nextLabel, "cond_else");

        const auto endLabel =
            label(nextLabel, "cond_end");

        emitExpression(
            out,
            *cond->condition,
            nextLabel
        );

        out << "    cbz w0, "
            << elseLabel << '\n';

        emitExpression(
            out,
            *cond->thenExpr,
            nextLabel
        );

        out << "    b " << endLabel << '\n';
        out << elseLabel << ":\n";

        emitExpression(
            out,
            *cond->elseExpr,
            nextLabel
        );

        out << endLabel << ":\n";
        return;
    }

    // --------------------------------------------------------
    // Binary expressions
    // --------------------------------------------------------

    if (const auto* binary =
            dynamic_cast<const BinaryExpr*>(&expr)) {

        // Comma: evaluate left, discard it, evaluate right.
        if (binary->op == BinaryOperator::Comma) {
            emitExpression(
                out,
                *binary->left,
                nextLabel
            );

            emitExpression(
                out,
                *binary->right,
                nextLabel
            );

            return;
        }

        // Logical AND/OR: short-circuit evaluation.
        if (
            binary->op == BinaryOperator::LogicalAnd ||
            binary->op == BinaryOperator::LogicalOr
        ) {
            const bool isAnd =
                binary->op == BinaryOperator::LogicalAnd;

            const auto shortLabel =
                label(nextLabel, "logical_short");

            const auto endLabel =
                label(nextLabel, "logical_end");

            emitExpression(
                out,
                *binary->left,
                nextLabel
            );

            out << (
                isAnd
                    ? "    cbz w0, "
                    : "    cbnz w0, "
            ) << shortLabel << '\n';

            emitExpression(
                out,
                *binary->right,
                nextLabel
            );

            out << "    cmp w0, #0\n";
            out << "    cset w0, ne\n";
            out << "    b " << endLabel << '\n';

            out << shortLabel << ":\n";
            out << (
                isAnd
                    ? "    mov w0, #0\n"
                    : "    mov w0, #1\n"
            );

            out << endLabel << ":\n";
            return;
        }

        // General binary expression.
        emitExpression(
            out,
            *binary->left,
            nextLabel
        );

        out << "    sub sp, sp, #16\n";
        out << "    str w0, [sp]\n";

        emitExpression(
            out,
            *binary->right,
            nextLabel
        );

        out << "    ldr w9, [sp]\n";
        out << "    add sp, sp, #16\n";

        emitBinaryOperation(out, binary->op);
        return;
    }

    throw std::runtime_error(
        "unsupported expression in AArch64 backend"
    );
}

// ============================================================
// Binary operation instruction selection
//
// Input:  w9 = left, w0 = right
// Output: w0 = result
// ============================================================

void AArch64MacOSCodeGenerator::emitBinaryOperation(
    std::ostringstream& out,
    BinaryOperator op
) const
{
    switch (op) {
    case BinaryOperator::Add:
        out << "    add w0, w9, w0\n";
        break;

    case BinaryOperator::Subtract:
        out << "    sub w0, w9, w0\n";
        break;

    case BinaryOperator::Multiply:
        out << "    mul w0, w9, w0\n";
        break;

    case BinaryOperator::Divide:
        out << "    sdiv w0, w9, w0\n";
        break;

    case BinaryOperator::Remainder:
        out << "    sdiv w10, w9, w0\n";
        out << "    msub w0, w10, w0, w9\n";
        break;

    case BinaryOperator::ShiftLeft:
        out << "    lsl w0, w9, w0\n";
        break;

    case BinaryOperator::ShiftRight:
        out << "    asr w0, w9, w0\n";
        break;

    case BinaryOperator::Less:
        out << "    cmp w9, w0\n";
        out << "    cset w0, lt\n";
        break;

    case BinaryOperator::LessEqual:
        out << "    cmp w9, w0\n";
        out << "    cset w0, le\n";
        break;

    case BinaryOperator::Greater:
        out << "    cmp w9, w0\n";
        out << "    cset w0, gt\n";
        break;

    case BinaryOperator::GreaterEqual:
        out << "    cmp w9, w0\n";
        out << "    cset w0, ge\n";
        break;

    case BinaryOperator::Equal:
        out << "    cmp w9, w0\n";
        out << "    cset w0, eq\n";
        break;

    case BinaryOperator::NotEqual:
        out << "    cmp w9, w0\n";
        out << "    cset w0, ne\n";
        break;

    case BinaryOperator::BitwiseAnd:
        out << "    and w0, w9, w0\n";
        break;

    case BinaryOperator::BitwiseXor:
        out << "    eor w0, w9, w0\n";
        break;

    case BinaryOperator::BitwiseOr:
        out << "    orr w0, w9, w0\n";
        break;

    case BinaryOperator::LogicalAnd:
    case BinaryOperator::LogicalOr:
    case BinaryOperator::Comma:
        throw std::runtime_error(
            "internal error: non-arithmetic binary op"
        );
    }
}

// ============================================================
// Integer literals
// ============================================================

void AArch64MacOSCodeGenerator::emitIntegerLiteral(
    std::ostringstream& out,
    std::int64_t value
) const
{
    const auto bits =
        static_cast<std::uint32_t>(value);

    out << "    movz w0, #"
        << (bits & 0xFFFFU)
        << '\n';

    if ((bits >> 16U) != 0) {
        out << "    movk w0, #"
            << (bits >> 16U)
            << ", lsl #16\n";
    }
}

// ============================================================
// Local variable address
//
// Local variables live below x29.
// x10 = x29 - offset
// ============================================================

void AArch64MacOSCodeGenerator::emitLocalAddress(
    std::ostringstream& out,
    std::size_t offset
) const
{
    const auto bits =
        static_cast<std::uint64_t>(offset);

    out << "    movz x10, #"
        << (bits & 0xFFFFU)
        << '\n';

    for (
        unsigned shift = 16;
        shift < 64;
        shift += 16
    ) {
        const auto chunk =
            (bits >> shift) & 0xFFFFU;

        if (chunk != 0) {
            out << "    movk x10, #"
                << chunk
                << ", lsl #"
                << shift
                << '\n';
        }
    }

    out << "    sub x10, x29, x10\n";
}

// ============================================================
// Local variable load
// ============================================================

void AArch64MacOSCodeGenerator::emitLoadLocal(
    std::ostringstream& out,
    std::size_t offset
) const
{
    emitLocalAddress(out, offset);

    out << "    ldr w0, [x10]\n";
}

// ============================================================
// Local variable store
// ============================================================

void AArch64MacOSCodeGenerator::emitStoreLocal(
    std::ostringstream& out,
    std::size_t offset
) const
{
    emitLocalAddress(out, offset);

    out << "    str w0, [x10]\n";
}

} // namespace microcc

