project('cprfun', ['cpp', 'c'], version: '1.0.0',
	default_options: [
		'cpp_std=c++20',
		'cpp_rtti=false',
		'warning_level=3',
		'default_library=static'
	],
	meson_version: '>=1.1.0'
)

if get_option('feat_libsodium').enabled()
	add_project_arguments('-DFEAT_LIBSODIUM', language: ['c', 'cpp'])
endif

root_inc = include_directories('src')

subdir('src/core')
subdir('src/calchash')
subdir('src/bruteforce')
subdir('src/multibrute')
subdir('src/dbtool')
