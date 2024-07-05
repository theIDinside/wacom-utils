#include "process.h"
#include "util.h"
#include <cstdlib>
#include <sys/wait.h>
#include <thread>

ExecResult::~ExecResult() noexcept {
  if (procStdout != -1) {
    close(procStdout);
  }
}

ExecResult::ExecResult(int out, ExitCode code) noexcept
    : procStdout(out), code(code) {}

static void execute(std::string cmd, const char **argv) noexcept {
  if (execv(cmd.data(),
            reinterpret_cast<char *const *>(const_cast<char **>(argv))) == -1) {
    FATAL("failed to execv");
  }
}

bool ExecResult::succcess() const noexcept { return code.code == 0; }

int ExecResult::std_out() const noexcept { return procStdout; }

/*static*/
std::unique_ptr<ExecResult>
ExecResult::exec(std::string cmd, std::span<const std::string> args) noexcept {

#ifdef WU_DEBUG
  std::cout << "executing xsetwacom: '" << cmd;
  for (const auto &arg : args) {
    std::cout << " " << arg;
  }
  std::cout << "'" << std::endl;
#endif

  int stdio[2];
  std::vector<const char *> arguments{};
  arguments.push_back(cmd.data());
  for (const auto &a : args) {
    arguments.push_back(a.c_str());
  }
  arguments.push_back(nullptr);

  if (pipe(stdio) == -1) {
    FATAL("pipe failed");
  }
  const auto pid = fork();
  switch (pid) {
  case -1: {
    FATAL("fork failed");
  } break;
  case 0: {
    if (dup2(stdio[1], STDOUT_FILENO) == -1) {
      FATAL("dup2 for stdout failed - we wouldn't be able to read stdout from "
            "child process");
    }
    close(stdio[1]);
    execute(cmd, arguments.data());
  } break;
  default:
    break;
  }

  // child stdout -> stdio[1] (which can be read from stdio[0])
  int stat;
  int exitCode = -1;
  for (;;) {
    const auto r = waitpid(pid, &stat, 0);
    if (r == -1) {
      FATAL("exec'ing child must have failed");
    }
    if (WIFEXITED(stat)) {
      exitCode = WEXITSTATUS(stat);
      break;
    }
  }
  return std::make_unique<ExecResult>(stdio[0], ExitCode{exitCode});
}

ReadResult read(std::unique_ptr<ExecResult> &&proc) noexcept {
  char buf[512];
  std::string result{};
  for (auto bytes = ::read(proc->std_out(), buf, 512);;
       ::read(proc->std_out(), buf, 512)) {
    if (bytes == -1) {
      if (result.size() == 0) {
        return ReadResult{{}, errno};
      } else {
        break;
      }
    }
    result.append(buf, bytes);

    if (bytes != 512) {
      break;
    }
  }
  return ReadResult{.data = std::move(result), .error = 0};
}