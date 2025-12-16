// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#if defined(__linux__)

#include "shell/exec.hpp"
#include <cerrno>
#include <cstring>
#include <memory>
#include <sys/wait.h>
#include <unistd.h>

namespace shell {

static std::optional<ExecError> sys_err(std::string where) {
  return ExecError{where + ": " + std::strerror(errno)};
}

class ExecLinux final : public Exec::Impl {
public:
  void init_job_control() override {
    // TODO: Implement job control
  }

  std::optional<ExecError> run_command(std::vector<std::string> const& argv) override {
    if (argv.empty()) return ExecError{"empty command"};

    // Prepare C-style argv
    std::vector<char*> c_argv;
    c_argv.reserve(argv.size() + 1);
    for (auto const& arg : argv) {
      c_argv.push_back(const_cast<char*>(arg.c_str()));
    }
    c_argv.push_back(nullptr);

    pid_t pid = ::fork();
    if (pid < 0) return sys_err("fork");

    if (pid == 0) {
      // Child process
      ::execvp(c_argv[0], c_argv.data());
      // If execvp returns, it failed
      std::_Exit(127);
    }

    // Parent process - wait for child
    int status = 0;
    if (::waitpid(pid, &status, 0) < 0) {
      return sys_err("waitpid");
    }

    return std::nullopt;
  }
};

std::unique_ptr<Exec::Impl> make_exec_impl() {
  return std::make_unique<ExecLinux>();
}

} // namespace shell

#endif // __linux__
