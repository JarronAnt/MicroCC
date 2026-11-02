#pragma once

#include "microcc/AST.hpp"

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
        const FunctionDecl& function
    ) const;

    void emitStatement(
        std::ostringstream& out,
        const Stmt& statement
    ) const;

    void emitExpression(
        std::ostringstream& out,
        const Expr& expression
    ) const;

    void emitIntegerLiteral(
        std::ostringstream& out,
        std::int64_t value
    ) const;
};

} // namespace microcc
