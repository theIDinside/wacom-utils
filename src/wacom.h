#pragma once
#include "selection.h"
#include <span>
#include <string>
#include <variant>
#include <vector>

struct WacomDevice {
  std::string deviceName;
  std::string id;
};

struct WacomConfig {
  std::string deviceName;
};

class WacomDeviceManager {
  std::vector<WacomDevice> devices{};
  std::optional<std::string> queryDevices() noexcept;

public:
  explicit WacomDeviceManager() noexcept = default;
  void updateDeviceList() noexcept;
  bool hasDevice(std::string_view name) const noexcept;
  std::span<const WacomDevice> getDevices() const noexcept;

  static WacomDeviceManager *getDeviceManager() noexcept;
};

enum class XSetWacomCommands { MapToArea };

struct MapToAreaCommand {
  WacomConfig config;
  Selection sel;

  auto static constexpr CommandType() noexcept -> XSetWacomCommands {
    return XSetWacomCommands::MapToArea;
  }

  auto static constexpr can_verify() noexcept -> bool { return false; }
};

// When adding new commands, add them here
using WacomCommand = std::variant<MapToAreaCommand>;

enum class CommandResult { Ok, Error, NotKnown };

std::optional<WacomConfig>
parse_config(std::span<const std::string_view> input) noexcept;
CommandResult perform_command(const WacomCommand &command) noexcept;