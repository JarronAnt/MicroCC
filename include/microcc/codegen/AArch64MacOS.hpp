
#pragma once

#include "microcc/AST.hpp"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

namespace microcc {

class AArch64MacOSCodeGenerator {
public:
    [[nodiscard]]
    std::string generate(const Program& program) const;

private:
    void emitFunction(
        std::ostringstream& out,
        const FunctionDecl& function,
        std::size_t& nextLabel
    ) const;

    void emitStatement(
        std::ostringstream& out,
        const Stmt& statement,
        std::size_t& nextLabel
    ) const;

    void emitExpression(
        std::ostringstream& out,
        const Expr& expression,
        std::size_t& nextLabel
    ) const;

    void emitIntegerLiteral(
        std::ostringstream& out,
        std::int64_t value
    ) const;
};

} // namespace microcc

