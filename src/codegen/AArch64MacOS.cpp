
#include "microcc/codegen/AArch64MacOS.hpp"

#include <cstdint>
#include <stdexcept>
#include <string>

namespace microcc {

// ============================================================
// Generate assembly for an entire program
// ============================================================

std::string AArch64MacOSCodeGenerator::generate(
    const Program& program
) const
{
    std::ostringstream out;

    // Unique counter for branch labels.
    std::size_t nextLabel = 0;

    out << ".text\n\n";

    for (const auto& function : program.functions) {
        emitFunction(
            out,
            *function,
            nextLabel
        );

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
    // Mach-O uses an underscore prefix for C symbols.
    const std::string symbol = "_" + function.name;

    out << ".globl " << symbol << '\n';
    out << ".p2align 2\n";
    out << symbol << ":\n";

    bool emittedReturn = false;

    for (const auto& statement : function.body) {
        emitStatement(
            out,
            *statement,
            nextLabel
        );

        if (
            dynamic_cast<const ReturnStmt*>(
                statement.get()
            ) != nullptr
        ) {
            emittedReturn = true;
        }
    }

    // Reaching the end of main is equivalent to return 0.
    if (!emittedReturn && function.name == "main") {
        out << "    mov w0, #0\n";
        out << "    ret\n";
    } else if (!emittedReturn) {
        throw std::runtime_error(
            "Phase 2 requires non-main functions "
            "to contain a return statement"
        );
    }
}

// ============================================================
// Statement code generation
// ============================================================

void AArch64MacOSCodeGenerator::emitStatement(
    std::ostringstream& out,
    const Stmt& statement,
    std::size_t& nextLabel
) const
{
    if (
        const auto* ret =
            dynamic_cast<const ReturnStmt*>(&statement)
    ) {
        emitExpression(
            out,
            *ret->expression,
            nextLabel
        );

        out << "    ret\n";
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
    const Expr& expression,
    std::size_t& nextLabel
) const
{
    // ========================================================
    // Integer literals
    // ========================================================

    if (
        const auto* integer =
            dynamic_cast<const IntegerLiteral*>(&expression)
    ) {
        emitIntegerLiteral(out, integer->value);
        return;
    }

    // ========================================================
    // Unary expressions
    // ========================================================

    if (
        const auto* unary =
            dynamic_cast<const UnaryExpr*>(&expression)
    ) {
        emitExpression(
            out,
            *unary->operand,
            nextLabel
        );

        switch (unary->op) {
        case UnaryOperator::Plus:
            // +x leaves the operand unchanged.
            break;

        case UnaryOperator::Negate:
            out << "    neg w0, w0\n";
            break;

        case UnaryOperator::LogicalNot:
            // Result is 1 if operand is zero, otherwise 0.
            out << "    cmp w0, #0\n";
            out << "    cset w0, eq\n";
            break;

        case UnaryOperator::BitwiseNot:
            out << "    mvn w0, w0\n";
            break;
        }

        return;
    }

    // ========================================================
    // Conditional expression
    //
    // condition ? thenExpr : elseExpr
    //
    // Only one branch should execute.
    // ========================================================

    if (
        const auto* conditional =
            dynamic_cast<const ConditionalExpr*>(&expression)
    ) {
        const auto id = nextLabel++;

        const std::string elseLabel =
            "Lmicrocc_cond_else_" + std::to_string(id);

        const std::string endLabel =
            "Lmicrocc_cond_end_" + std::to_string(id);

        // Evaluate condition into w0.
        emitExpression(
            out,
            *conditional->condition,
            nextLabel
        );

        // If w0 == 0, jump to the else branch.
        out << "    cbz w0, " << elseLabel << '\n';

        // True branch.
        emitExpression(
            out,
            *conditional->thenExpr,
            nextLabel
        );

        // Skip the false branch.
        out << "    b " << endLabel << '\n';

        // False branch.
        out << elseLabel << ":\n";

        emitExpression(
            out,
            *conditional->elseExpr,
            nextLabel
        );

        // Both branches meet here.
        out << endLabel << ":\n";

        return;
    }

    // ========================================================
    // Binary expressions
    // ========================================================

    if (
        const auto* binary =
            dynamic_cast<const BinaryExpr*>(&expression)
    ) {
        // ====================================================
        // Comma operator
        //
        // Evaluate left, discard it, evaluate right.
        // ====================================================

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

        // ====================================================
        // Logical AND - short-circuit evaluation
        //
        // If left == 0, skip right and return 0.
        // ====================================================

        if (binary->op == BinaryOperator::LogicalAnd) {
            const auto id = nextLabel++;

            const std::string falseLabel =
                "Lmicrocc_and_false_" +
                std::to_string(id);

            const std::string endLabel =
                "Lmicrocc_and_end_" +
                std::to_string(id);

            // Evaluate left.
            emitExpression(
                out,
                *binary->left,
                nextLabel
            );

            // Left == 0 -> result is false.
            out << "    cbz w0, "
                << falseLabel << '\n';

            // Otherwise evaluate right.
            emitExpression(
                out,
                *binary->right,
                nextLabel
            );

            // Normalize right operand to 0 or 1.
            out << "    cmp w0, #0\n";
            out << "    cset w0, ne\n";

            out << "    b " << endLabel << '\n';

            out << falseLabel << ":\n";
            out << "    mov w0, #0\n";

            out << endLabel << ":\n";

            return;
        }

        // ====================================================
        // Logical OR - short-circuit evaluation
        //
        // If left != 0, skip right and return 1.
        // ====================================================

        if (binary->op == BinaryOperator::LogicalOr) {
            const auto id = nextLabel++;

            const std::string trueLabel =
                "Lmicrocc_or_true_" +
                std::to_string(id);

            const std::string endLabel =
                "Lmicrocc_or_end_" +
                std::to_string(id);

            // Evaluate left.
            emitExpression(
                out,
                *binary->left,
                nextLabel
            );

            // Left != 0 -> result is true.
            out << "    cbnz w0, "
                << trueLabel << '\n';

            // Otherwise evaluate right.
            emitExpression(
                out,
                *binary->right,
                nextLabel
            );

            // Normalize right operand to 0 or 1.
            out << "    cmp w0, #0\n";
            out << "    cset w0, ne\n";

            out << "    b " << endLabel << '\n';

            out << trueLabel << ":\n";
            out << "    mov w0, #1\n";

            out << endLabel << ":\n";

            return;
        }

        // ====================================================
        // General binary expression
        //
        // 1. Evaluate left -> w0
        // 2. Save left on the stack
        // 3. Evaluate right -> w0
        // 4. Restore left -> w9
        // 5. Apply operator -> w0
        // ====================================================

        emitExpression(
            out,
            *binary->left,
            nextLabel
        );

        // Preserve stack alignment.
        out << "    sub sp, sp, #16\n";
        out << "    str w0, [sp]\n";

        emitExpression(
            out,
            *binary->right,
            nextLabel
        );

        out << "    ldr w9, [sp]\n";
        out << "    add sp, sp, #16\n";

        // Left operand  = w9
        // Right operand = w0
        // Result        = w0

        switch (binary->op) {
        // ====================================================
        // Arithmetic
        // ====================================================

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
            // quotient = left / right
            out << "    sdiv w10, w9, w0\n";

            // remainder = left - quotient * right
            out << "    msub w0, w10, w0, w9\n";
            break;

        // ====================================================
        // Shifts
        // ====================================================

        case BinaryOperator::ShiftLeft:
            out << "    lsl w0, w9, w0\n";
            break;

        case BinaryOperator::ShiftRight:
            out << "    asr w0, w9, w0\n";
            break;

        // ====================================================
        // Relational
        // ====================================================

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

        // ====================================================
        // Equality
        // ====================================================

        case BinaryOperator::Equal:
            out << "    cmp w9, w0\n";
            out << "    cset w0, eq\n";
            break;

        case BinaryOperator::NotEqual:
            out << "    cmp w9, w0\n";
            out << "    cset w0, ne\n";
            break;

        // ====================================================
        // Bitwise
        // ====================================================

        case BinaryOperator::BitwiseAnd:
            out << "    and w0, w9, w0\n";
            break;

        case BinaryOperator::BitwiseXor:
            out << "    eor w0, w9, w0\n";
            break;

        case BinaryOperator::BitwiseOr:
            out << "    orr w0, w9, w0\n";
            break;

        // Handled separately above.
        case BinaryOperator::LogicalAnd:
        case BinaryOperator::LogicalOr:
        case BinaryOperator::Comma:
            throw std::runtime_error(
                "internal error: unexpected binary operator"
            );
        }

        return;
    }

    throw std::runtime_error(
        "unsupported expression in AArch64 backend"
    );
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

    const std::uint32_t low =
        bits & 0xFFFFU;

    const std::uint32_t high =
        (bits >> 16U) & 0xFFFFU;

    out << "    movz w0, #" << low << '\n';

    if (high != 0) {
        out << "    movk w0, #"
            << high
            << ", lsl #16\n";
    }
}

} // namespace microcc

