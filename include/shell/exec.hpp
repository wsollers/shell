// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#pragma once
#include <optional>
#include <string>
#include <vector>

namespace shell {

struct ExecError {
  std::string msg;
};

// Simplified exec interface without AST dependencies
class Exec {
public:
  Exec();
  ~Exec();

  // Call once in interactive mode to set up job control
  void init_job_control();

  // Execute a simple command (argv[0] is the program)
  std::optional<ExecError> run_command(std::vector<std::string> const& argv);

  // Implementation interface for platform-specific code
  struct Impl {
    virtual ~Impl() = default;
    virtual void init_job_control() = 0;
    virtual std::optional<ExecError> run_command(std::vector<std::string> const& argv) = 0;
  };

private:
  Impl* impl_{nullptr};
};

} // namespace shell
