
#pragma once

#include "microcc/AST.hpp"

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace microcc {

class AArch64MacOSCodeGenerator {
public:
    [[nodiscard]]
    std::string generate(const Program& program) const;

private:
    struct LoopLabels {
        std::string breakLabel;
        std::string continueLabel;
    };

    void emitFunction(
        std::ostringstream& out,
        const FunctionDecl& function,
        std::size_t& nextLabel
    ) const;

    void emitStatement(
        std::ostringstream& out,
        const Stmt& statement,
        std::size_t& nextLabel,
        const std::string& returnLabel,
        std::vector<LoopLabels>& loops
    ) const;

    void emitExpression(
        std::ostringstream& out,
        const Expr& expression,
        std::size_t& nextLabel
    ) const;

    void emitBinaryOperation(
        std::ostringstream& out,
        BinaryOperator op
    ) const;

    void emitIntegerLiteral(
        std::ostringstream& out,
        std::int64_t value
    ) const;

    void emitLocalAddress(
        std::ostringstream& out,
        std::size_t offset
    ) const;

    void emitLoadLocal(
        std::ostringstream& out,
        std::size_t offset
    ) const;

    void emitStoreLocal(
        std::ostringstream& out,
        std::size_t offset
    ) const;
};

} // namespace microcc

