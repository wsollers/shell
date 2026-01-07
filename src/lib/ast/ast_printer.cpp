#include "shell/ast_printer.hpp"

#include <sstream>

namespace wshell {

namespace {

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

void indent(std::ostream& os, int level) {
    for (int i = 0; i < level; ++i)
        os << "  ";
}

void print_redirection(const Redirection& r, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    switch (r.kind) {
        case RedirectKind::Input:
            os << "< ";
            break;
        case RedirectKind::OutputTruncate:
            os << "> ";
            break;
        case RedirectKind::OutputAppend:
            os << ">> ";
            break;
    }
    os << r.target << "\n";
}

void print_command(const CommandNode& cmd, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Command: " << cmd.command_name;
    if (cmd.background)
        os << " &";
    os << "\n";

    if (!cmd.arguments.empty()) {
        indent(os, indent_level + 1);
        os << "Args:";
        for (const auto& arg : cmd.arguments)
            os << " " << arg;
        os << "\n";
    }

    if (!cmd.redirections.empty()) {
        indent(os, indent_level + 1);
        os << "Redirections:\n";
        for (const auto& r : cmd.redirections)
            print_redirection(r, os, indent_level + 2);
    }
}

// Forward declaration
void print_node(const StatementNode& stmt, std::ostream& os, int indent_level);

void print_pipeline(const PipelineNode& pipe, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Pipeline:\n";
    for (const auto& cmd : pipe.commands)
        print_command(cmd, os, indent_level + 1);
}

void print_sequence(const SequenceNode& seq, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Sequence:\n";
    for (const auto& stmt : seq.statements)
        print_node(stmt, os, indent_level + 1);
}

// -----------------------------------------------------------------------------
// Core variant printer
// -----------------------------------------------------------------------------

void print_node(const StatementNode& stmt, std::ostream& os, int indent_level) {
    std::visit(
        [&](auto const& node) {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, CommentNode>) {
                indent(os, indent_level);
                os << "Comment: " << node.text << "\n";

            } else if constexpr (std::is_same_v<T, AssignmentNode>) {
                indent(os, indent_level);
                os << "Assignment: " << node.variable << " = " << node.value << "\n";

            } else if constexpr (std::is_same_v<T, CommandNode>) {
                print_command(node, os, indent_level);

            } else if constexpr (std::is_same_v<T, PipelineNode>) {
                print_pipeline(node, os, indent_level);

            } else if constexpr (std::is_same_v<T, SequenceNode>) {
                print_sequence(node, os, indent_level);

            } else {
                indent(os, indent_level);
                os << "<Unknown node>\n";
            }
        },
        stmt);
}

}  // namespace

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void print_statement(const StatementNode& stmt, std::ostream& os, int indent_level) {
    print_node(stmt, os, indent_level);
}

void print_program(const ProgramNode& program, std::ostream& os) {
    for (const auto& stmt : program.statements)
        print_node(stmt, os, 0);
}

std::string to_string(const StatementNode& stmt) {
    std::ostringstream oss;
    print_node(stmt, oss, 0);
    return oss.str();
}

std::string to_string(const ProgramNode& program) {
    std::ostringstream oss;
    print_program(program, oss);
    return oss.str();
}

}  // namespace wshell