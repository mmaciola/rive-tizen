# rive-tizen
This repo contains Rive animation library that extends SKIA Runtime Engine and can be compiled for tizen.

## Build
### Prepare Sub Modules
Rive-tizen extends [rive-cpp](https://github.com/rive-app/rive-cpp) and [skia](https://github.com/google/skia).
Clone submodule projects and build it on rive-tizen repo:
```
git submodule update --init --recursive
```

### Build skia
Run make_skia.sh specifying options:
```
./make_skia.sh [cpu/gpu] [tizen]
```

### Build project
Run meson to configure and ninja to build and install:
```
meson build; ninja -C build install
```

### GBS Building
While building with GBS there might be a problem related to submodules. If you face a problem, run `./make_submodules_static.sh` script. It will remove submodules, add them as local files and commit with a message "static submodules" or squash into head.
```
git submodule update --init --recursive
./make_submodules_static.sh
gbs build -A armv7l
```