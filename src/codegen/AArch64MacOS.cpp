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
    // Mach-O prefixes externally visible C symbols with '_'.
    //
    // C:
    //
    //     int main()
    //
    // Assembly:
    //
    //     _main:
    //
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

    // C permits reaching the end of main().
    // It is equivalent to returning 0.
    if (!emittedReturn && function.name == "main") {
        out << "    mov w0, #0\n";
        out << "    ret\n";
        return;
    }

    if (!emittedReturn) {
        throw std::runtime_error(
            "Phase 1 requires non-main functions "
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
    if (const auto* integer =
            dynamic_cast<const IntegerLiteral*>(&expression)) {

        emitIntegerLiteral(
            out,
            integer->value
        );

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

    // AArch64 return values for int are placed in W0.
    //
    // movz initializes the register.
    // movk replaces a selected 16-bit portion.
    out << "    movz w0, #" << low << '\n';

    if (high != 0) {
        out << "    movk w0, #"
            << high
            << ", lsl #16\n";
    }
}

} // namespace microcc
