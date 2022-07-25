#!/bin/sh

mv submodule submodule_
git submodule deinit -f submodule/skia
git submodule deinit -f submodule/rive-cpp
git rm --cached submodule/skia
git rm --cached submodule/rive-cpp
rm -rf .git/modules/submodule/skia
rm -rf .git/modules/submodule/rive-cpp
mv submodule_ submodule

# due to licencing and to minimize, remove unneeded files
# rive-cpp
rm -rf submodule/rive-cpp/dependencies
rm -rf submodule/rive-cpp/dev
rm -rf submodule/rive-cpp/rivinfo
rm -rf submodule/rive-cpp/skia
rm -rf submodule/rive-cpp/tess
rm -rf submodule/rive-cpp/test
rm -rf submodule/rive-cpp/viewer
# skia
rm -rf submodule/skia/bezel
rm -rf submodule/skia/bench
rm -rf submodule/skia/build
rm -rf submodule/skia/build_overrides
rm -rf submodule/skia/client_utils
rm -rf submodule/skia/demos.skia.org
rm -rf submodule/skia/dm
rm -rf submodule/skia/docker
rm -rf submodule/skia/docs
rm -rf submodule/skia/example
rm -rf submodule/skia/fuzz
rm -rf submodule/skia/gm
rm -rf submodule/skia/infra
rm -rf submodule/skia/platform_tools
rm -rf submodule/skia/resources
rm -rf submodule/skia/samplecode
rm -rf submodule/skia/site
rm -rf submodule/skia/specs
rm -rf submodule/skia/toolchain

git add submodule/skia/*
git add submodule/rive-cpp/*
git add submodule/skia/.*

# create seperate commit
git commit -m "static submodules"

# squash into existing
git reset --soft HEAD~1
git commit --amend --no-edit