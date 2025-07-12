project('downscaler', 'c', default_options: ['default_library=static'])

inc = include_directories('include')

inih_dep = dependency('libinih', fallback: ['inih', 'inih_dep'])
libheif_dep = dependency('libheif', fallback: ['libheif', 'libheif_dep'])
libmagic_dep = dependency('libmagic', fallback: ['libmagic', 'libmagic_dep'])

install_data('src/config.ini')

executable('downscaler',
'src/main.c',
dependencies: [meson.get_compiler('c').find_library('m'), inih_dep, libheif_dep, libmagic_dep], 
include_directories: inc,
install: true)
