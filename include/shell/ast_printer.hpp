#pragma once

#include <iosfwd>
#include "shell/ast.hpp"

namespace wshell {

/// Pretty-print any AST node to an output stream.
void print_ast(const ASTNode& node, std::ostream& os, int indent = 0);

/// Pretty-print a whole program.
void print_program(const ProgramNode& program, std::ostream& os);

/// Convenience: return pretty-printed AST as a string.
std::string to_string(const ASTNode& node);

/// Convenience: return pretty-printed ProgramNode as a string.
std::string to_string(const ProgramNode& program);

} // namespace wshell

