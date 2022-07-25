#!/bin/sh
set -e

if [ $# -eq 0 ]; then
    printf "This script makes a skia submodule into submodule/skia. Please choose\nwhether to compile cpu or cpu version, for desktop or tizen.\n"
    printf "Usage: ./make_skia.sh [cpu/gpu] [tizen]"
    printf "\n\n"
    exit 0
fi

make_for_gpu=false
make_for_tizen=false

for arg; do
    if [ $arg = "cpu" ]; then
        make_for_gpu=false
    elif [ $arg = "gpu" ]; then
        make_for_gpu=true
    elif [ $arg = "tizen" ]; then
        make_for_tizen=true
    fi
done

printf "Will make %s version, for %s.\n\n" $([ $make_for_gpu = true ] && echo "GPU" || echo "CPU") $([ $make_for_tizen = true ] && echo "TIZEN" || echo "LINUX")

cd submodule/skia || { echo "SKIA submodule doesn't exist.\nDid you run: git submodule update --init --recursive"; exit 1; }
printf "Directory: %s\n" "${PWD}"

if [ $make_for_tizen = false ]; then
    # for linux only. In tizen we don't need external dependencies.
    python3 tools/git-sync-deps
    gn_path='bin/gn'
else
    gn_path='/bin/gn'
fi

# build static for host
$gn_path gen out/Shared --args=" \
    extra_cflags=[\"-fno-rtti\", \"-flto\", \"-O3\", \"-DSK_DISABLE_SKPICTURE\", \"-DSK_DISABLE_TEXT\", \"-DSK_DISABLE_AAA\"] \
    is_clang=true \
    is_official_build=true \
    is_component_build=true \
    skia_use_gl=$make_for_gpu \
    skia_enable_gpu=$make_for_gpu \
    skia_use_zlib=false \
    skia_enable_fontmgr_empty=false \
    skia_use_libpng_encode=false \
    skia_use_libpng_decode=false \
    skia_enable_skgpu_v1=$make_for_gpu \
    skia_use_dng_sdk=false \
    skia_use_egl=false \
    skia_use_expat=false \
    skia_use_fontconfig=false \
    skia_use_freetype=false \
    skia_use_icu=false \
    skia_use_libheif=false \
    skia_use_system_libpng=false \
    skia_use_system_libjpeg_turbo=false \
    skia_use_libjpeg_turbo_encode=false \
    skia_use_libjpeg_turbo_decode=false \
    skia_use_libwebp_encode=false \
    skia_use_libwebp_decode=false \
    skia_use_system_libwebp=false \
    skia_use_lua=false \
    skia_use_piex=false \
    skia_use_vulkan=false \
    skia_use_metal=false \
    skia_use_angle=false \
    skia_use_system_zlib=false \
    skia_enable_spirv_validation=false \
    skia_enable_pdf=false \
    skia_enable_skottie=false \
    skia_enable_tools=false \
    skia_enable_skgpu_v2=false \
    "
ninja -C out/Shared
du -hs out/Shared/libskia.so

cd ..