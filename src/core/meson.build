sources = ['stdafx.cpp', 'core.cpp', 'SHA256PyCrypto.cpp']
if get_option('feat_libsodium').enabled()
	sources += 'SHA256Sodium.cpp'
endif

core_lib = library('core',
  sources: sources,
  dependencies: [
	  dependency('libsodium', method: 'pkg-config', required: get_option('feat_libsodium'))
  ],
  version: '1.0.0'
)

# Expose this library for use by other subprojects
core_dep = declare_dependency(link_with: core_lib,
	include_directories: ['.', root_inc],
)
