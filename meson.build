project('downscaler', 'c', default_options: ['default_library=static'])

if host_machine.system() == 'darwin'
  install_data('info.plist',
  install_dir : 'Contents')
endif

executable('downscaler', 'src/main.c', dependencies: meson.get_compiler('c').find_library('m'))
