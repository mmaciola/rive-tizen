project('rive-tizen-viewer',
        'cpp',
        default_options : ['cpp_std=c++17'],
        version : '0.2.0',
        license : 'MIT')

cc = meson.get_compiler('cpp')

headers = [include_directories('../inc'), include_directories('../src')]
rive_headers = [include_directories('../submodule/rive-cpp/include')]

skia_inc = include_directories('../submodule/skia')
skia_inc_core = include_directories('../submodule/skia/include/core')
skia_inc_effects = include_directories('../submodule/skia/include/effects')
skia_inc_dirs = [skia_inc, skia_inc_core, skia_inc_effects]

skia_inc_gpu = include_directories('../submodule/skia/include/gpu')

dir_base = meson.current_source_dir()
skia_dir = join_paths(dir_base, '../submodule/skia/out/Shared')
rivetizen_dir = join_paths(dir_base, '../build/src')

skia_lib = cc.find_library('libskia', dirs : skia_dir, static: false, header_include_directories: skia_inc_dirs)
rive_tizen_lib = cc.find_library('librive-tizen', dirs: rivetizen_dir, static: false)

skia_dep = declare_dependency(
	include_directories: [skia_inc, skia_inc_core, skia_inc_effects],
	dependencies: skia_lib,
)

rive_tizen_dep = declare_dependency(
    include_directories: headers,
    dependencies: rive_tizen_lib
)

if get_option('use_gl') == true
    message('Building GPU Viewer')

    glfw3_dep = dependency('glfw3', required : true)
    threads_dep = dependency('threads', required: true)
    glx_dep = dependency('glx', required: true)

    viewer_dep = [glfw3_dep, threads_dep, glx_dep, skia_dep]

    viewer_headers = [headers, skia_inc_gpu]

    executable('viewer_gpu', 'viewer_gpu.cpp',
        include_directories : viewer_headers,
        link_with : rive_tizen_lib,
        dependencies : viewer_dep)
else
    message('Building CPU Viewer')

    elementary_dep = dependency('elementary', required : true)
    viewer_dep = [elementary_dep, skia_dep, rive_tizen_dep]

    executable('viewer_cpu', 'viewer_cpu.cpp',
        include_directories : [headers, rive_headers],
        dependencies : viewer_dep)
endif