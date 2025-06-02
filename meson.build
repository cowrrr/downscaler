project('downscaler', 'c', default_options: ['default_library=static'])

inc = include_directories('include')

install_data('src/config.ini')

executable('downscaler',
'src/main.c',
'include/ini.c',
dependencies: [meson.get_compiler('c').find_library('m')], 
include_directories: inc,
install: true)

if host_machine.system() == 'darwin'
  install_data('platforms/macos/info.plist',
  install_dir : 'Contents')
endif
