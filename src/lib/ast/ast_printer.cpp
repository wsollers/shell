#include "shell/ast_printer.hpp"
#include <sstream>

namespace wshell {

namespace {

void indent(std::ostream& os, int level) {
    for (int i = 0; i < level; ++i)
        os << "  ";
}

void print_redirection(const Redirection& r, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    switch (r.kind) {
        case RedirectKind::Input:  os << "< ";  break;
        case RedirectKind::Output: os << "> ";  break;
        case RedirectKind::Append: os << ">> "; break;
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

void print_pipeline(const PipelineNode& pipe, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Pipeline:\n";
    for (const auto& cmd : pipe.commands)
        print_ast(*cmd, os, indent_level + 1);
}

void print_sequence(const SequenceNode& seq, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Sequence:\n";
    for (const auto& stmt : seq.statements)
        print_ast(*stmt, os, indent_level + 1);
}

void print_assignment(const AssignmentNode& a, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Assignment: " << a.variable << " = " << a.value << "\n";
}

void print_comment(const CommentNode& c, std::ostream& os, int indent_level) {
    indent(os, indent_level);
    os << "Comment: " << c.text << "\n";
}

} // namespace

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

void print_ast(const ASTNode& node, std::ostream& os, int indent_level) {
    if (auto* c = dynamic_cast<const CommentNode*>(&node))
        return print_comment(*c, os, indent_level);

    if (auto* a = dynamic_cast<const AssignmentNode*>(&node))
        return print_assignment(*a, os, indent_level);

    if (auto* cmd = dynamic_cast<const CommandNode*>(&node))
        return print_command(*cmd, os, indent_level);

    if (auto* p = dynamic_cast<const PipelineNode*>(&node))
        return print_pipeline(*p, os, indent_level);

    if (auto* s = dynamic_cast<const SequenceNode*>(&node))
        return print_sequence(*s, os, indent_level);

    indent(os, indent_level);
    os << "<Unknown AST node>\n";
}

void print_program(const ProgramNode& program, std::ostream& os) {
    for (const auto& stmt : program.statements)
        std::visit([&](auto const& ptr) { print_ast(*ptr, os, 0); }, stmt);
}

std::string to_string(const ASTNode& node) {
    std::ostringstream oss;
    print_ast(node, oss, 0);
    return oss.str();
}

std::string to_string(const ProgramNode& program) {
    std::ostringstream oss;
    print_program(program, oss);
    return oss.str();
}

} // namespace wshell