#if defined(_WIN32)

#include "shell/exec.hpp"
#include <memory>
#include <optional>

namespace shell {

struct Exec::Impl {
  virtual ~Impl() = default;
  virtual void init_job_control() = 0;
  virtual std::optional<ExecError> launch_command(Command const& cmd, bool bg, int& exit_status) = 0;
  virtual std::optional<ExecError> launch_pipeline(Pipeline const& p, bool bg, int& exit_status) = 0;
  virtual std::optional<ExecError> launch_logical_controller(Logical const& l, bool bg, Arena const& a) = 0;
};

static std::unique_ptr<Exec::Impl> make_exec_impl();

class ExecWin32 final : public Exec::Impl {
public:
  void init_job_control() override {}

  std::optional<ExecError> launch_command(Command const&, bool, int&) override {
    return ExecError{"ExecWin32: launch_command not implemented yet"};
  }
  std::optional<ExecError> launch_pipeline(Pipeline const&, bool, int&) override {
    return ExecError{"ExecWin32: launch_pipeline not implemented yet"};
  }
  std::optional<ExecError> launch_logical_controller(Logical const&, bool, Arena const&) override {
    return ExecError{"ExecWin32: logical background controller not implemented yet"};
  }
};

std::unique_ptr<Exec::Impl> make_exec_impl() {
  return std::make_unique<ExecWin32>();
}

} // namespace shell

#endif // _WIN32
