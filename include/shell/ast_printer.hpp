#pragma once

#include <iosfwd>
#include <string>
#include "shell/ast.hpp"

namespace wshell {

// Pretty-print a StatementNode (the variant)
void print_statement(const StatementNode& stmt, std::ostream& os, int indent = 0);

// Pretty-print a ProgramNode
void print_program(const ProgramNode& program, std::ostream& os);

// Convenience: return pretty-printed StatementNode as a string
std::string to_string(const StatementNode& stmt);

// Convenience: return pretty-printed ProgramNode as a string
std::string to_string(const ProgramNode& program);

} // namespace wshell