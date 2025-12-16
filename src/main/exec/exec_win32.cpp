// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#if defined(_WIN32)

#include "shell/exec.hpp"
#include <memory>
#include <windows.h>

namespace shell {

class ExecWin32 final : public Exec::Impl {
public:
  void init_job_control() override {
    // No job control on Windows for now
  }

  std::optional<ExecError> run_command(std::vector<std::string> const& argv) override {
    if (argv.empty()) return ExecError{"empty command"};

    // Build command line
    std::string cmdline;
    for (auto const& arg : argv) {
      if (!cmdline.empty()) cmdline += " ";
      cmdline += arg;
    }

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi{};

    if (!CreateProcessA(nullptr, const_cast<char*>(cmdline.c_str()),
                       nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
      return ExecError{"CreateProcess failed"};
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return std::nullopt;
  }
};

std::unique_ptr<Exec::Impl> make_exec_impl() {
  return std::make_unique<ExecWin32>();
}

} // namespace shell

#endif // _WIN32

#include "shell/exec.hpp"
#include <memory>
#include <optional>

namespace shell {

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
