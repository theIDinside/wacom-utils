#include "wacom.h"
#include "process.h"
#include "util.h"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <iterator>
#include <mutex>
#include <regex>
#include <sstream>
#include <unistd.h>

static std::vector<WacomDevice>
parse_devices(const std::string &input) noexcept {
  static std::regex pattern(R"((.+?)\s+id:\s+(\d+))");
  std::vector<WacomDevice> result{};

  auto begin = std::sregex_iterator(input.begin(), input.end(), pattern);
  auto end = std::sregex_iterator();

  // Iterate over all matches and print the results
  for (std::sregex_iterator i = begin; i != end; ++i) {
    std::smatch match = *i;
    if (match.size() == 3) { // Full match, name, and id
      std::string name = match[1].str();
      std::string id = match[2].str();
      result.emplace_back(std::move(name), std::move(id));
    }
  }
  return result;
}

std::optional<std::string> WacomDeviceManager::queryDevices() noexcept {
  auto &&[data, err] = read(
      ExecResult::exec("/usr/bin/xsetwacom",
                       std::span<const std::string>{{"--list", "devices"}}));
  if (err) {
    std::cerr << "reading device list failed: " << strerror(err) << std::endl;
    std::abort();
  }
  return data;
}

void WacomDeviceManager::updateDeviceList() noexcept {
  if (const auto res = queryDevices(); res) {
    devices = parse_devices(res.value());
  }
}

bool WacomDeviceManager::hasDevice(std::string_view name) const noexcept {
  for (const auto &[deviceName, id] : devices) {
    if (deviceName == name || id == name) {
      return true;
    }
  }
  return false;
}

std::span<const WacomDevice> WacomDeviceManager::getDevices() const noexcept {
  return devices;
}

/*static*/
WacomDeviceManager *WacomDeviceManager::getDeviceManager() noexcept {
  static WacomDeviceManager manager{};
  return &manager;
}

std::optional<WacomConfig>
parse_config(std::span<const std::string_view> args) noexcept {
  if (args.size() == 0) {
    return {};
  }

  if (args.size() == 1) {
    return WacomConfig{.device_name = std::string{args.front()}};
  }
  const auto len = wu::accumulate(
      args, [](auto acc, const auto &e) { return acc + e.size(); });
  // user forgot to put name within quotes
  std::string device_name{};
  device_name.reserve(len);
  for (const auto &s : args) {
    device_name.append(s);
  }

  if (WacomDeviceManager::getDeviceManager()->hasDevice(device_name)) {
    return WacomConfig{.device_name = std::move(device_name)};
  } else {
    return {};
  }
}

CommandResult perform_command(const WacomCommand &command) noexcept {
  return std::visit(
      [](const MapToAreaCommand &cmd) -> CommandResult {
        std::vector<std::string> args{};
        args.reserve(4);
        args.push_back("set");
        args.push_back(cmd.config.device_name);
        args.push_back("maptooutput");
        const auto [dimension, origin] = cmd.sel;
        std::stringstream ss{};
        ss << dimension.x << "x" << dimension.y << "+" << origin.x << "+"
           << origin.y;
        args.push_back(ss.str());
        const auto result = read(ExecResult::exec("xsetwacom", args));
        return CommandResult::NotKnown;
      },
      command);
}