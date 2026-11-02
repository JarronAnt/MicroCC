#include "microcc/Lexer.hpp"
#include "microcc/Parser.hpp"
#include "microcc/Token.hpp"
#include "microcc/codegen/AArch64MacOS.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <spawn.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

extern char** environ;

namespace {

struct Options {
    std::filesystem::path inputFile;
    std::filesystem::path outputFile;

    bool emitAssemblyOnly = false;
    bool dumpTokens = false;
};

void printUsage(std::string_view executable)
{
    std::cout
        << "MicroCC - tiny C compiler\n\n"
        << "Usage:\n"
        << "  " << executable
        << " [options] <input.c>\n\n"
        << "Options:\n"
        << "  -o <file>       Set output file\n"
        << "  -S              Emit assembly only\n"
        << "  --dump-tokens   Print lexer output\n"
        << "  -h, --help      Show this help\n";
}

Options parseArguments(
    int argc,
    char** argv
)
{
    Options options;

    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            std::exit(0);
        }

        if (arg == "-S") {
            options.emitAssemblyOnly = true;
            continue;
        }

        if (arg == "--dump-tokens") {
            options.dumpTokens = true;
            continue;
        }

        if (arg == "-o") {
            if (i + 1 >= argc) {
                throw std::runtime_error(
                    "-o requires an output path"
                );
            }

            options.outputFile = argv[++i];
            continue;
        }

        if (!arg.empty() && arg.front() == '-') {
            throw std::runtime_error(
                "unknown option: " +
                std::string(arg)
            );
        }

        if (!options.inputFile.empty()) {
            throw std::runtime_error(
                "MicroCC Phase 1 accepts one input file"
            );
        }

        options.inputFile = arg;
    }

    if (options.inputFile.empty()) {
        throw std::runtime_error(
            "no input file provided"
        );
    }

    if (options.outputFile.empty()) {
        if (options.emitAssemblyOnly) {
            options.outputFile = options.inputFile;
            options.outputFile.replace_extension(".s");
        } else {
            options.outputFile = "a.out";
        }
    }

    return options;
}

std::string readFile(
    const std::filesystem::path& path
)
{
    std::ifstream input(
        path,
        std::ios::binary
    );

    if (!input) {
        throw std::runtime_error(
            "unable to open input file: " +
            path.string()
        );
    }

    std::ostringstream buffer;
    buffer << input.rdbuf();

    return buffer.str();
}

void writeFile(
    const std::filesystem::path& path,
    std::string_view contents
)
{
    std::ofstream output(
        path,
        std::ios::binary
    );

    if (!output) {
        throw std::runtime_error(
            "unable to open output file: " +
            path.string()
        );
    }

    output << contents;

    if (!output) {
        throw std::runtime_error(
            "failed while writing output file: " +
            path.string()
        );
    }
}

int runProcess(
    const std::vector<std::string>& arguments
)
{
    if (arguments.empty()) {
        throw std::runtime_error(
            "attempted to execute empty command"
        );
    }

    std::vector<char*> argv;
    argv.reserve(arguments.size() + 1);

    for (const auto& argument : arguments) {
        argv.push_back(
            const_cast<char*>(argument.c_str())
        );
    }

    argv.push_back(nullptr);

    pid_t pid {};

    const int spawnResult =
        posix_spawnp(
            &pid,
            argv[0],
            nullptr,
            nullptr,
            argv.data(),
            environ
        );

    if (spawnResult != 0) {
        throw std::runtime_error(
            "failed to execute: " +
            arguments.front()
        );
    }

    int status = 0;

    if (waitpid(pid, &status, 0) < 0) {
        throw std::runtime_error(
            "failed waiting for child process"
        );
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return 1;
}

void dumpTokens(
    const std::vector<microcc::Token>& tokens
)
{
    for (const auto& token : tokens) {
        std::cout
            << token.location.line
            << ":"
            << token.location.column
            << "  "
            << microcc::tokenKindName(token.kind);

        if (!token.lexeme.empty()) {
            std::cout
                << "  `"
                << token.lexeme
                << "`";
        }

        if (token.integerValue.has_value()) {
            std::cout
                << "  value="
                << *token.integerValue;
        }

        std::cout << '\n';
    }
}

} // namespace

int main(
    int argc,
    char** argv
)
{
    try {
        const Options options =
            parseArguments(argc, argv);

        const std::string source =
            readFile(options.inputFile);

        microcc::Lexer lexer(
            source,
            options.inputFile.string()
        );

        auto tokens = lexer.tokenize();

        if (options.dumpTokens) {
            dumpTokens(tokens);
        }

        microcc::Parser parser(
            std::move(tokens),
            options.inputFile.string()
        );

        const microcc::Program program =
            parser.parseProgram();

        const microcc::AArch64MacOSCodeGenerator codeGenerator;

        const std::string assembly =
            codeGenerator.generate(program);

        if (options.emitAssemblyOnly) {
            writeFile(
                options.outputFile,
                assembly
            );

            return 0;
        }

        const auto temporaryAssembly =
            std::filesystem::temp_directory_path() /
            (
                "microcc-" +
                std::to_string(
                    static_cast<long long>(getpid())
                ) +
                ".s"
            );

        writeFile(
            temporaryAssembly,
            assembly
        );

        const int clangResult =
            runProcess({
                "clang",
                "-arch",
                "arm64",
                temporaryAssembly.string(),
                "-o",
                options.outputFile.string()
            });

        std::error_code ec;
        std::filesystem::remove(
            temporaryAssembly,
            ec
        );

        if (clangResult != 0) {
            std::cerr
                << "microcc: clang failed with exit code "
                << clangResult
                << '\n';

            return clangResult;
        }

        return 0;

    } catch (const std::exception& error) {
        std::cerr
            << "microcc: "
            << error.what()
            << '\n';

        return 1;
    }
}
