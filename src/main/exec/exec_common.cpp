#include "shell/exec.hpp"
#include <memory>
#include <variant>

namespace shell {

std::unique_ptr<Exec::Impl> make_exec_impl();

Exec::Exec() : impl_(make_exec_impl().release()) {}
Exec::~Exec() { delete impl_; }

void Exec::init_job_control() { impl_->init_job_control(); }

std::optional<ExecError> Exec::run(Sequence const& seq, Arena const& a) {
  for (auto const& it : seq.items) {
    if (auto e = run_item(it, a)) return e;
  }
  return std::nullopt;
}

std::optional<ExecError> Exec::run_item(ListItem const& item, Arena const& a) {
  if (item.background) return run_node_bg(item.node, a);
  int status = 0;
  return run_node_fg(item.node, a, status);
}

std::optional<ExecError> Exec::run_node_fg(std::size_t node, Arena const& a, int& exit_status) {
  auto const& v = a.at(node).v;

  if (auto const* c = std::get_if<Command>(&v))
    return impl_->launch_command(*c, /*bg=*/false, exit_status);

  if (auto const* p = std::get_if<Pipeline>(&v))
    return impl_->launch_pipeline(*p, /*bg=*/false, exit_status);

  if (auto const* l = std::get_if<Logical>(&v)) {
    int lhs = 0;
    if (auto e = run_node_fg(l->lhs, a, lhs)) return e;

    bool run_rhs = (l->op == LogicalOp::AND_IF) ? (lhs == 0) : (lhs != 0);
    if (!run_rhs) { exit_status = lhs; return std::nullopt; }

    int rhs = 0;
    if (auto e = run_node_fg(l->rhs, a, rhs)) return e;
    exit_status = rhs;
    return std::nullopt;
  }

  return ExecError{"unknown node type"};
}

std::optional<ExecError> Exec::run_node_bg(std::size_t node, Arena const& a) {
  auto const& v = a.at(node).v;

  if (auto const* c = std::get_if<Command>(&v)) {
    int dummy = 0;
    return impl_->launch_command(*c, /*bg=*/true, dummy);
  }

  if (auto const* p = std::get_if<Pipeline>(&v)) {
    int dummy = 0;
    return impl_->launch_pipeline(*p, /*bg=*/true, dummy);
  }

  if (auto const* l = std::get_if<Logical>(&v)) {
    return impl_->launch_logical_controller(*l, /*bg=*/true, a);
  }

  return ExecError{"unknown node type"};
}

} // namespace shell
