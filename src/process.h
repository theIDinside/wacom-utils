#pragma once
#include <memory>
#include <optional>
#include <span>
#include <thread>
#include <unistd.h>
#include <vector>

struct ExitCode {
  int code;
};

class ExecResult {
  int procStdout;
  ExitCode code;

public:
  ExecResult(int out, ExitCode code) noexcept;
  ~ExecResult() noexcept;

  auto succcess() const noexcept -> bool;
  auto std_out() const noexcept -> int;
  // executes `cmd` with the cli arguments `args`.
  auto static exec(std::string cmd, std::span<const std::string> args) noexcept
      -> std::unique_ptr<ExecResult>;
};

struct ReadResult {
  std::optional<std::string> data;
  int error;
};

ReadResult read(std::unique_ptr<ExecResult> &&proc) noexcept;