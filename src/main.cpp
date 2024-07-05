#include "app.h"
#include "process.h"
#include "util.h"
#include "wacom.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstring>
#include <iostream>
#include <mutex>

int main(int argc, const char **argv) {
  ApplicationState::Initialize(argc, argv);
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
    app.configureWacomMapping(WacomConfig{.device_name = std::move(config->id)},
                              select);
  }
  return 0;
}
