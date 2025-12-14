#pragma once
#include "shell/ast.hpp"
#include <optional>
#include <string>

namespace shell {

struct ExecError {
  std::string msg;
};

class Exec {
public:
  Exec();
  ~Exec();

  // Call once in interactive mode to set up job control (no-op if non-interactive).
  void init_job_control();

  // Execute a whole Sequence (list of items)
  std::optional<ExecError> run(Sequence const& seq, Arena const& a);

  // Common AST wiring accessible to implementations:
  std::optional<ExecError> run_node_fg(std::size_t node, Arena const& a, int& exit_status);

  // Implementation interface for platform-specific code
  struct Impl {
    virtual ~Impl() = default;
    virtual void init_job_control() = 0;
    virtual std::optional<ExecError> launch_command(Command const& cmd, bool bg, int& exit_status) = 0;
    virtual std::optional<ExecError> launch_pipeline(Pipeline const& p, bool bg, int& exit_status) = 0;
    virtual std::optional<ExecError> launch_logical_controller(Logical const& l, bool bg, Arena const& a) = 0;
  };

private:
  Impl* impl_{nullptr};

  // Internal AST wiring:
  std::optional<ExecError> run_item(ListItem const& item, Arena const& a);
  std::optional<ExecError> run_node_bg(std::size_t node, Arena const& a);
};

} // namespace shell
