#include "microcc/codegen/AArch64MacOS.hpp"

#include <cstdint>
#include <stdexcept>

namespace microcc {

std::string AArch64MacOSCodeGenerator::generate(
    const Program& program
) const
{
    std::ostringstream out;

    out << ".text\n\n";

    for (const auto& function : program.functions) {
        emitFunction(out, *function);
        out << '\n';
    }

    return out.str();
}

void AArch64MacOSCodeGenerator::emitFunction(
    std::ostringstream& out,
    const FunctionDecl& function
) const
{
    const std::string symbol = "_" + function.name;

    out << ".globl " << symbol << '\n';
    out << ".p2align 2\n";
    out << symbol << ":\n";

    bool emittedReturn = false;

    for (const auto& statement : function.body) {
        emitStatement(out, *statement);

        if (dynamic_cast<const ReturnStmt*>(
                statement.get()) != nullptr) {
            emittedReturn = true;
        }
    }

    if (!emittedReturn && function.name == "main") {
        out << "    mov w0, #0\n";
        out << "    ret\n";
        return;
    }

    if (!emittedReturn) {
        throw std::runtime_error(
            "Phase 2 requires non-main functions "
            "to contain an explicit return statement"
        );
    }
}

void AArch64MacOSCodeGenerator::emitStatement(
    std::ostringstream& out,
    const Stmt& statement
) const
{
    if (const auto* returnStmt =
            dynamic_cast<const ReturnStmt*>(&statement)) {

        emitExpression(
            out,
            *returnStmt->expression
        );

        // The result is in w0.
        out << "    ret\n";
        return;
    }

    throw std::runtime_error(
        "unsupported statement in AArch64 backend"
    );
}

void AArch64MacOSCodeGenerator::emitExpression(
    std::ostringstream& out,
    const Expr& expression
) const
{
    // ========================================================
    // Integer literal
    // ========================================================

    if (const auto* integer =
            dynamic_cast<const IntegerLiteral*>(&expression)) {

        emitIntegerLiteral(out, integer->value);
        return;
    }

    // ========================================================
    // Unary expression
    // ========================================================

    if (const auto* unary =
            dynamic_cast<const UnaryExpr*>(&expression)) {

        emitExpression(out, *unary->operand);

        switch (unary->op) {
        case UnaryOperator::Plus:
            // +x changes nothing.
            break;

        case UnaryOperator::Negate:
            out << "    neg w0, w0\n";
            break;
        }

        return;
    }

    // ========================================================
    // Binary expression
    // ========================================================

    if (const auto* binary =
            dynamic_cast<const BinaryExpr*>(&expression)) {

        // Evaluate the left expression.
        // The result is placed in w0.
        emitExpression(out, *binary->left);

        // Preserve the left result on the stack.
        //
        // Allocate 16 bytes to maintain stack alignment.
        out << "    sub sp, sp, #16\n";
        out << "    str w0, [sp]\n";

        // Evaluate right expression.
        //
        // This may recursively evaluate additional
        // binary expressions and use the stack.
        emitExpression(out, *binary->right);

        // Restore left operand into w9.
        //
        // Right operand remains in w0.
        out << "    ldr w9, [sp]\n";
        out << "    add sp, sp, #16\n";

        // Perform the requested arithmetic.
        //
        // Left:  w9
        // Right: w0
        // Result: w0

        switch (binary->op) {
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
        }

        return;
    }

    throw std::runtime_error(
        "unsupported expression in AArch64 backend"
    );
}

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
