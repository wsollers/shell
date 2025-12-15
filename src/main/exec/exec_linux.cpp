#if defined(__linux__)

#include "shell/exec.hpp"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <optional>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace shell {

static std::optional<ExecError> sys_err(std::string where) {
  return ExecError{where + ": " + std::strerror(errno)};
}

static void ignore_job_control_signals() {
  std::signal(SIGINT,  SIG_IGN);
  std::signal(SIGQUIT, SIG_IGN);
  std::signal(SIGTSTP, SIG_IGN);
  std::signal(SIGTTIN, SIG_IGN);
  std::signal(SIGTTOU, SIG_IGN);
  std::signal(SIGCHLD, SIG_IGN); // skeleton
}

static void restore_default_signals() {
  std::signal(SIGINT,  SIG_DFL);
  std::signal(SIGQUIT, SIG_DFL);
  std::signal(SIGTSTP, SIG_DFL);
  std::signal(SIGTTIN, SIG_DFL);
  std::signal(SIGTTOU, SIG_DFL);
  std::signal(SIGCHLD, SIG_DFL);
}

static std::optional<ExecError> apply_redirs(Command const& cmd) {
  for (auto const& r : cmd.redirs) {
    int fd = -1;
    switch (r.kind) {
      case RedirKind::IN:
        fd = ::open(r.target.c_str(), O_RDONLY);
        if (fd < 0) return sys_err("open <");
        if (::dup2(fd, STDIN_FILENO) < 0) return sys_err("dup2 <");
        ::close(fd);
        break;
      case RedirKind::OUT_TRUNC:
        fd = ::open(r.target.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) return sys_err("open >");
        if (::dup2(fd, STDOUT_FILENO) < 0) return sys_err("dup2 >");
        ::close(fd);
        break;
      case RedirKind::OUT_APPEND:
        fd = ::open(r.target.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) return sys_err("open >>");
        if (::dup2(fd, STDOUT_FILENO) < 0) return sys_err("dup2 >>");
        ::close(fd);
        break;
      case RedirKind::HEREDOC:
        return ExecError{"heredoc (<<) not implemented yet"};
    }
  }
  return std::nullopt;
}

static std::vector<char*> make_argv(std::vector<std::string> const& argv, std::vector<std::string>& storage) {
  storage = argv;
  std::vector<char*> out;
  out.reserve(storage.size() + 1);
  for (auto& s : storage) out.push_back(s.data());
  out.push_back(nullptr);
  return out;
}

class ExecLinux final : public Exec::Impl {
public:
  void init_job_control() override {
    tty_fd_ = STDIN_FILENO;
    interactive_ = ::isatty(tty_fd_);
    if (!interactive_) return;

    ignore_job_control_signals();

    shell_pgid_ = ::getpid();
    (void)::setpgid(shell_pgid_, shell_pgid_);
    (void)::tcsetpgrp(tty_fd_, shell_pgid_);
  }

  std::optional<ExecError> launch_command(Command const& cmd, bool bg, int& exit_status) override {
    if (cmd.argv.empty()) return ExecError{"empty command"};

    pid_t pid = ::fork();
    if (pid < 0) return sys_err("fork");

    if (pid == 0) {
      restore_default_signals();

      ::setpgid(0, 0);
      pid_t pgid = ::getpid();

      if (!bg && interactive_) (void)::tcsetpgrp(tty_fd_, pgid);

      if (auto e = apply_redirs(cmd)) {
        (void)::write(STDERR_FILENO, e->msg.c_str(), e->msg.size());
        (void)::write(STDERR_FILENO, "\n", 1);
        _exit(127);
      }

      std::vector<std::string> storage;
      auto argvp = make_argv(cmd.argv, storage);
      ::execvp(argvp[0], argvp.data());
      _exit(127);
    }

    ::setpgid(pid, pid);

    if (bg) return std::nullopt;

    if (interactive_) (void)::tcsetpgrp(tty_fd_, pid);

    int status = 0;
    if (::waitpid(pid, &status, 0) < 0) return sys_err("waitpid");

    if (interactive_) (void)::tcsetpgrp(tty_fd_, shell_pgid_);

    if (WIFEXITED(status)) exit_status = WEXITSTATUS(status);
    else if (WIFSIGNALED(status)) exit_status = 128 + WTERMSIG(status);
    else exit_status = 1;

    return std::nullopt;
  }

  std::optional<ExecError> launch_pipeline(Pipeline const& p, bool bg, int& exit_status) override {
    if (p.cmds.empty()) return ExecError{"empty pipeline"};

    int prev_read = -1;
    pid_t pgid = -1;
    std::vector<pid_t> pids;
    pids.reserve(p.cmds.size());

    for (std::size_t i = 0; i < p.cmds.size(); ++i) {
      int pipefd[2] = {-1, -1};
      bool last = (i + 1 == p.cmds.size());
      if (!last && ::pipe(pipefd) < 0) return sys_err("pipe");

      pid_t pid = ::fork();
      if (pid < 0) return sys_err("fork");

      if (pid == 0) {
        restore_default_signals();

        if (pgid == -1) ::setpgid(0, 0);
        else ::setpgid(0, pgid);

        pid_t my_pgid = ::getpgrp();
        if (!bg && interactive_) (void)::tcsetpgrp(tty_fd_, my_pgid);

        if (prev_read != -1) ::dup2(prev_read, STDIN_FILENO);
        if (!last) ::dup2(pipefd[1], STDOUT_FILENO);

        if (prev_read != -1) ::close(prev_read);
        if (!last) { ::close(pipefd[0]); ::close(pipefd[1]); }

        if (auto e = apply_redirs(p.cmds[i])) {
          (void)::write(STDERR_FILENO, e->msg.c_str(), e->msg.size());
          (void)::write(STDERR_FILENO, "\n", 1);
          _exit(127);
        }

        std::vector<std::string> storage;
        auto argvp = make_argv(p.cmds[i].argv, storage);
        ::execvp(argvp[0], argvp.data());
        _exit(127);
      }

      if (pgid == -1) pgid = pid;
      ::setpgid(pid, pgid);
      pids.push_back(pid);

      if (prev_read != -1) ::close(prev_read);
      if (!last) {
        ::close(pipefd[1]);
        prev_read = pipefd[0];
      } else {
        prev_read = -1;
      }
    }

    if (bg) return std::nullopt;

    if (interactive_) (void)::tcsetpgrp(tty_fd_, pgid);

    int last_status = 0;
    for (pid_t pid : pids) {
      int st = 0;
      if (::waitpid(pid, &st, 0) < 0) return sys_err("waitpid pipeline");
      last_status = st;
    }

    if (interactive_) (void)::tcsetpgrp(tty_fd_, shell_pgid_);

    if (WIFEXITED(last_status)) exit_status = WEXITSTATUS(last_status);
    else if (WIFSIGNALED(last_status)) exit_status = 128 + WTERMSIG(last_status);
    else exit_status = 1;

    return std::nullopt;
  }

  std::optional<ExecError> launch_logical_controller(Logical const& l, bool /*bg*/, Arena const& a) override {
    pid_t pid = ::fork();
    if (pid < 0) return sys_err("fork logical-controller");

    if (pid == 0) {
      restore_default_signals();
      ::setpgid(0, 0);

      Exec e;
      e.init_job_control();

      int lhs = 0;
      if (auto er = e.run_node_fg(l.lhs, a, lhs)) _exit(127);

      bool run_rhs = (l.op == LogicalOp::AND_IF) ? (lhs == 0) : (lhs != 0);
      if (!run_rhs) _exit(lhs);

      int rhs = 0;
      if (auto er = e.run_node_fg(l.rhs, a, rhs)) _exit(127);
      _exit(rhs);
    }

    ::setpgid(pid, pid);
    return std::nullopt;
  }

private:
  pid_t shell_pgid_{-1};
  int tty_fd_{-1};
  bool interactive_{false};
};

std::unique_ptr<Exec::Impl> make_exec_impl() {
  return std::make_unique<ExecLinux>();
}

} // namespace shell

#endif // __linux__
