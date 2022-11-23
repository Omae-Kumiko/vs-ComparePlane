project('VS-ComparePlane', 'cpp',
  default_options: ['buildtype=release', 'b_lto=true', 'b_ndebug=if-release', 'cpp_std=c++17'],
  license: 'GPL-3.0-or-later',
  meson_version: '>=0.51.0',
  version: '1.1.1'
)

cxx = meson.get_compiler('cpp')

gcc_syntax = cxx.get_argument_syntax() == 'gcc'

if gcc_syntax
  if get_option('buildtype') == 'release'
    add_project_arguments('-fno-exceptions', language: 'cpp')
  endif
  if cxx.get_id() == 'clang' and build_machine.system() == 'windows'
    add_project_link_arguments('-lclang_rt.builtins-x86_64', language: 'cpp') # for __cpu_model
  endif
endif

if gcc_syntax and build_machine.system() != 'windows'
  vapoursynth_dep = dependency('vapoursynth', version: '>=54').partial_dependency(compile_args: true, includes: true)
  install_dir = vapoursynth_dep.get_variable(pkgconfig: 'libdir') / 'vapoursynth'
else
  vapoursynth_dep = []
  install_dir = get_option('libdir') / 'vapoursynth'
endif

sources = [
  'Complane/complane.cpp',
  'Complane/complane.h',
]

libs = []

if host_machine.cpu_family().startswith('x86')
  libs += static_library('avx', 'Complane/complane_avx.cpp',
    cpp_args: gcc_syntax ? ['-mavx', '-mfma'] : '/arch:AVX',
    dependencies: vapoursynth_dep,
    gnu_symbol_visibility: 'hidden'
  )
  libs += static_library('avx2', 'Complane/complane_avx2.cpp',
    cpp_args: gcc_syntax ? ['-mavx2', '-mfma'] : '/arch:AVX2',
    dependencies: vapoursynth_dep,
    gnu_symbol_visibility: 'hidden'
  )
endif

shared_module('complane', sources,
  dependencies: vapoursynth_dep,
  link_with: libs,
  install: true,
  install_dir: install_dir,
  gnu_symbol_visibility: 'hidden'
)
