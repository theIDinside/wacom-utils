#pragma once
#include "selection.h"
#include "wacom.h"
#include <X11/Xlib.h>
#include <iostream>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

struct X11Connection {
  Display *display{nullptr};
  int screen{0};
  Window root{0};

  auto isOpen() const noexcept -> bool;
  auto grabPointer() const noexcept -> void;
  auto ungrabPointer() const noexcept -> void;
};

struct ApplicationCliArgs {
  std::vector<std::string_view> cli_args;
  operator std::span<const std::string_view>() const noexcept;
  // operator std::span<const std::string_view>() const noexcept {
  //   return std::span<const std::string_view>{cli_args.data(),
  //   cli_args.size()};
  // }
};

class ApplicationState {
  static ApplicationState *Instance;
  ApplicationCliArgs cli_args;
  X11Connection connection;

  auto initX11() noexcept -> void;

public:
  explicit ApplicationState(ApplicationCliArgs &&args) noexcept
      : cli_args(std::move(args)), connection() {}

  ~ApplicationState() noexcept;

  auto args() const noexcept -> const ApplicationCliArgs & { return cli_args; }
  auto usageError(int exitCode) const -> void;

  // User-facing application features
  auto selectDevice() const noexcept -> std::optional<WacomDevice>;
  auto selectScreenArea() noexcept -> Selection;
  auto configureWacomMapping(const WacomConfig &cfg,
                             Selection selection) noexcept -> bool;

  auto static Initialize(int argc, const char **argv) noexcept -> void;
  auto static getAppInstance() noexcept -> ApplicationState &;
};