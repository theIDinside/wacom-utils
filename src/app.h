#pragma once
#include "selection.h"
#include "wacom.h"
#include <X11/Xlib.h>
#include <expected>
#include <filesystem>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace fs = std::filesystem;

struct X11Connection {
  Display *display{nullptr};
  int screen{0};
  Window root{0};

  auto isOpen() const noexcept -> bool;
  auto grabPointer() const noexcept -> void;
  auto ungrabPointer() const noexcept -> void;
};

struct ApplicationCliArgs {
  std::vector<std::string_view> cliArgs;
  operator std::span<const std::string_view>() const noexcept;
  // operator std::span<const std::string_view>() const noexcept {
  //   return std::span<const std::string_view>{cli_args.data(),
  //   cli_args.size()};
  // }
};

class ApplicationState {
  static ApplicationState *Instance;
  ApplicationCliArgs cliArgs;
  X11Connection connection;
  fs::path wacomConfigurePath;
  auto initX11() noexcept -> void;

public:
  explicit ApplicationState(ApplicationCliArgs &&args) noexcept
      : cliArgs(std::move(args)), connection() {}

  ~ApplicationState() noexcept;

  auto args() const noexcept -> const ApplicationCliArgs & { return cliArgs; }
  auto usageError(int exitCode) const -> void;

  // User-facing application features
  auto selectDevice() const noexcept -> std::optional<WacomDevice>;
  auto selectScreenArea() noexcept -> Selection;
  auto configureWacomMapping(const WacomConfig &cfg,
                             Selection selection) noexcept -> bool;

  auto static verifyHasXSetWacom() noexcept
      -> std::expected<fs::path, const char *>;
  auto static Initialize(int argc, const char **argv) noexcept -> void;
  auto static getAppInstance() noexcept -> ApplicationState &;
};