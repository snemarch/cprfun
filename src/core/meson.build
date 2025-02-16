core_lib = library('core',
  sources: ['stdafx.cpp', 'core.cpp', 'SHA256.cpp', 'pycrypto_SHA256.cpp'],
  dependencies: [
	  dependency('libsodium', method: 'pkg-config', required: get_option('feat_libsodium'))
  ],
  version: '1.0.0'
)

# Expose this library for use by other subprojects
core_dep = declare_dependency(link_with: core_lib,
	include_directories: ['.', root_inc],
)
