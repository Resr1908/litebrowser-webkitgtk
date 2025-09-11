project('litebrowser', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=2', 'cpp_std=c++17'])

gtk_dep = dependency('gtk+-3.0', required : true)
wk_dep  = dependency('webkit2gtk-4.0', required : true)

subdir('src')

# Simple test (smoke script)
test('smoke', find_program('tests/smoke.sh'), env: ['LITEBROWSER_TEST=1'])
