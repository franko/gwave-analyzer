project('gwave-analyzer', ['c', 'cpp'], version: '0.1.0', default_options : ['c_std=gnu99'])

if host_machine.system() == 'windows'
    windows = import('windows')
    add_global_arguments('-DWIN32', language : ['c', 'cpp'])
    gwave_res = windows.compile_resources('sample.rc')
endif

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

gwave_src = ['debug_log.c', 'main.c', 'fourier.c', 'wav_reader.c', 'wtm.c', 'wav2midi.c']

libui = dependency('libui')

executable('gwave', gwave_src,
           include_directories: [], dependencies: m_dep,
           gui_app: false)

# The dummy.cpp file is there to force the linker to use g++ because
# libui needs C++ linking.
gwave_ui_src = ['ui-main.c', 'dummy.cpp']

if host_machine.system() == 'windows'
    gwave_ui_src += gwave_res
endif

executable('gwave-ui', gwave_ui_src,
           include_directories: [], dependencies: [libui, m_dep],
           gui_app: false)
