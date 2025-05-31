project('downscaler', 'c', default_options: ['default_library=static'])

executable('downscaler',
'src/main.c',
dependencies: meson.get_compiler('c').find_library('m'), 
install: true)

if host_machine.system() == 'darwin'
  install_data('platforms/macos/info.plist',
  install_dir : 'Contents')
endif
