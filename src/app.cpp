#include "app.h"
#include "process.h"
#include "selection.h"
#include "util.h"
#include "wacom.h"
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstdlib> // for getenv

#include <format>
#include <mutex>
#include <string>

static constexpr auto UsageString =
    R"(wu <"device name" || id>
Then click and drag the desired area you want to map your device to.)";

using namespace std::string_view_literals;
std::once_flag AppStateInitFlag;

static void print_usage() noexcept { std::cout << UsageString << std::endl; }

bool X11Connection::isOpen() const noexcept { return display != nullptr; }
void X11Connection::grabPointer() const noexcept {
  XGrabPointer(display, root, False,
               ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
               GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
}

void X11Connection::ungrabPointer() const noexcept {
  XUngrabPointer(display, CurrentTime);
}

ApplicationCliArgs::operator std::span<const std::string_view>()
    const noexcept {
  return std::span<const std::string_view>{cliArgs.data(), cliArgs.size()};
}

static ApplicationCliArgs createArgs(int argc, const char **argv) noexcept {
  if (argc == 1) {
    return ApplicationCliArgs{{}};
  }

  std::vector<std::string_view> args{};
  args.reserve(argc - 1);
  for (auto i = 1; i < argc; ++i) {
    args.push_back(std::string_view{argv[i]});
  }

  return ApplicationCliArgs{std::move(args)};
}

ApplicationState::~ApplicationState() noexcept {
  XCloseDisplay(connection.display);
}

auto ApplicationState::initX11() noexcept -> void {
  if (connection.isOpen()) {
    return;
  }
  // Open connection to the X server
  connection.display = XOpenDisplay(NULL);
  if (!connection.isOpen()) {
    std::cerr << "Unable to open X display" << std::endl;
    exit(1);
  }
  connection.screen = DefaultScreen(connection.display);
  connection.root = DefaultRootWindow(connection.display);
}

void ApplicationState::usageError(int exitCode) const {
  print_usage();
  exit(exitCode);
}

std::optional<WacomDevice> ApplicationState::selectDevice() const noexcept {
  std::cout << "Pick device from list:\n";
  auto choice = 0;
  auto idx = 0;
  const auto devices = WacomDeviceManager::getDeviceManager()->getDevices();
  for (const auto &device : devices) {
    if (idx == 0) {
      std::cout << "[" << idx++ << "]*: " << device.deviceName << std::endl;
    } else {
      std::cout << "[" << idx++ << "]: " << device.deviceName << std::endl;
    }
  }
  std::cout << "* is the default choice (just press enter):\nChoice: ";
  std::string input;
  std::getline(std::cin, input);
  if (input.empty()) {
    return devices.front();
  }
  const auto parse =
      std::from_chars(input.data(), input.data() + input.size(), choice, 10);
  if (parse.ec != std::errc()) {
    return {};
  }
  if (choice >= 0 && choice < idx) {
    return devices[choice];
  }
  return {};
}

Selection ApplicationState::selectScreenArea() noexcept {
  std::cout << "Please click and drag to select an area on the screen."
            << std::endl;
  connection.grabPointer();
  XEvent event;
  ActiveSelection active_sel{};
  while (true) {
    XNextEvent(connection.display, &event);
    if (event.type == ButtonPress && event.xbutton.button == Button1) {
      active_sel.on_click(event.xbutton.x_root, event.xbutton.y_root);
    } else if (event.type == MotionNotify && active_sel.selecting()) {
      active_sel.on_move(event.xbutton.x_root, event.xbutton.y_root);
    } else if (event.type == ButtonRelease && event.xbutton.button == Button1) {
      active_sel.on_release(event.xbutton.x_root, event.xbutton.y_root);
      break;
    }
  }
  connection.ungrabPointer();
  return active_sel.selection();
}

bool ApplicationState::configureWacomMapping(const WacomConfig &cfg,
                                             Selection selection) noexcept {
  const auto [width, height] = selection.dimensions;
  const auto [x, y] = selection.origin;

  const auto args =
      std::vector<std::string>{{"set", cfg.deviceName, "maptooutput",
                                wu_format("{}x{}+{}+{}", width, height, x, y)}};
  auto exec = ExecResult::exec("/usr/bin/xsetwacom", args);

  if (exec->succcess()) {
    std::cout << "Selected area: " << width << "x" << height << "+" << x << "+"
              << y << std::endl;
    return true;
  } else {
    std::cerr << "Failed to configure mapping to " << width << "x" << height
              << "+" << x << "+" << y << std::endl;
    return false;
  }
}

/*static*/
std::expected<fs::path, const char *>
ApplicationState::verifyHasXSetWacom() noexcept {
  const auto pathEnv = std::getenv("PATH");
  if (pathEnv == nullptr) {
    std::cerr << "PATH environment variable not found." << std::endl;
    return std::unexpected{"Could not determine if $PATH environment variable "
                           "existed. Can't resolve xsetwacom because of it."};
  }

  const auto paths = wu::split_string(pathEnv, ':');

  fs::path path = "/";
  for (const auto &p : paths) {
    path = fs::path{p} / "xsetwacom";
    if (fs::exists(path)) {
      return path;
    }
  }
  return std::unexpected{"Could not find xsetwacom on $PATH"};
}

/*static*/ void ApplicationState::Initialize(int argc,
                                             const char **argv) noexcept {
  std::call_once(AppStateInitFlag, [=]() {
    Instance = new ApplicationState{createArgs(argc, argv)};
    WacomDeviceManager::getDeviceManager()->updateDeviceList();
    Instance->initX11();
  });
}

/*static*/ ApplicationState &ApplicationState::getAppInstance() noexcept {
  if (Instance == nullptr) {
    FATAL("Application state has not been initialized yet");
  }
  return *Instance;
}

ApplicationState *ApplicationState::Instance = nullptr;