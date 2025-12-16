// Copyright (c) 2024 William Sollers
// SPDX-License-Identifier: BSD-2-Clause

#include "shell/exec.hpp"
#include <memory>

namespace shell {

std::unique_ptr<Exec::Impl> make_exec_impl();

Exec::Exec() : impl_(make_exec_impl().release()) {}
Exec::~Exec() { delete impl_; }

void Exec::init_job_control() { impl_->init_job_control(); }

std::optional<ExecError> Exec::run_command(std::vector<std::string> const& argv) {
  return impl_->run_command(argv);
}

} // namespace shell
