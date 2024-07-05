#include "app.h"
#include "process.h"
#include "util.h"
#include "wacom.h"
#include <expected>
#include <format>
#include <iostream>

int main(int argc, const char **argv) {
  ApplicationState::Initialize(argc, argv);
  if (!ApplicationState::verifyHasXSetWacom()) {
    std::cerr << "could not find xsetwacom on $PATH" << std::endl;
  }
  auto &app = ApplicationState::getAppInstance();

  auto config = parse_config(app.args());
  if (config) {
    const auto select = app.selectScreenArea();
    app.configureWacomMapping(config.value(), select);
  } else {
    auto config = app.selectDevice();
    if (!config) {
      std::cout << " you picked an invalid option\n";
    }
    const auto select = app.selectScreenArea();
    app.configureWacomMapping(WacomConfig{.deviceName = std::move(config->id)},
                              select);
  }
  return 0;
}
