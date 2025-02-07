project('cprfun', ['cpp', 'c'],
	default_options: [
		'cpp_std=c++20',
		'default_library=static'
	],
)

root_inc = include_directories('src')

bindir = 'bin'
objdir = 'obj'

subdir('src/core')
subdir('src/calchash')
subdir('src/bruteforce')
subdir('src/multibrute')
subdir('src/dbtool')
