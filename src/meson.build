compiler_flags = ['-DRIVE_BUILD']

rive_tizen_src = [
	'skia_factory.cpp',
]

if get_option('use_gl') == true
	add_dep = [dependency('threads', required: true), dependency('gl', required: true)]
else
	add_dep = [dependency('threads', required: true)]
endif

rive_tizen_dep = declare_dependency(
	dependencies        : add_dep,
	include_directories : include_directories('.'),
	sources : rive_tizen_src
)

rive_tizen_lib = library(
	'rive-tizen',
	include_directories : headers,
	version             : meson.project_version(),
	dependencies        : [rive_tizen_dep, rive_cpp_dep, skia_dep],
	install             : true,
	cpp_args            : compiler_flags
)

rive_tizen_lib_dep = declare_dependency(
	include_directories : headers,
	link_with : rive_tizen_lib
)

pkg_mod = import('pkgconfig')

pkg_mod.generate(
        libraries    : rive_tizen_lib,
        version      : meson.project_version(),
        name         : 'librive-tizen',
        filebase     : 'rive-tizen',
        description  : 'A Rive Animation Tizen Runtime Engine using SKIA renderer'
)