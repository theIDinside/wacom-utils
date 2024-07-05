# wacom-utils

Utilities that intends making use of Wacom Intuos BT easier on Linux.

Current features:

- Click and drag to select an area on screen, which the Wacom tablet then gets mapped to

### Contents

- [Usage](#usage)
  - [Use WU](#use-wu)
  - [Build WU](#building-wu-wacom-utils)
- [Releases](#releases)
  - [Version 1.0](#version-10)

## Usage

### Building wu (wacom-utils)

Build requirements:

- cmake
- libX11-devel (fedora), libx11-dev (debian)
- C++ compiler that supports at least c++20

```bash
  # Configure dependencies

  # On Fedora (rpm)
  sudo dnf install libX11-dev

  # On Debian (Ubuntu etc)
  sudo apt-get install libx11-dev
```

Wacom Utils _may_ add additional dependencies, but 3rd party deps are always a nightmarish hell hole. But it would be nice to have some more UI stuff, but WU can probably get away with using X11 directly.

```bash
  # Build using Ninja
  git clone https://github.com/theIDinside/wacom-utils
  cd wacom-utils
  mkdir build && cd build
  cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
  cmake --build .

  # Build using standard CMake
  git clone https://github.com/theIDinside/wacom-utils && cd wacom-utils
  mkdir build && cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release
  cmake --build .
```

### Use WU

After you've built WU go to the build directory (called $PATH_TO_BUILD_DIR in these docs) and execute:

```bash
  $PATH_TO_BUILD_DIR/bin/wu
```

Of if you know the exact ID of your wacom device you can do

```bash
  $PATH_TO_BUILD_DIR/bin/wu "IdOrDeviceName"
```

Then do what `wu` tells you to do.

## Releases

### Version 1.0

Version 1.0 adds the functionality of clicking & dragging between two points on the screen, and mapping the wacom to this area specifically.

Version 1.0 does not contain a user interface nor does it show where you are currently dragging/mapping. This will come in future releases.

Some of the source code might look over engineered (because they are). But I'm thinking in the future we'd probably want wacom-utils to run as a service. At least there
should definitely be a UI, at which point we probably want this design.
