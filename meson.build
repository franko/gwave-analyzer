project('gwave-analyzer', 'c', version: '0.1.0', default_options : ['c_std=gnu99'])

if host_machine.system() == 'windows'
    add_global_arguments('-DWIN32', language : 'c')
endif

cc = meson.get_compiler('c')
m_dep = cc.find_library('m', required : false)

gwave_src = ['debug_log.c', 'main.c', 'comune.c', 'fourier.c', 'wav_reader.c', 'wtm.c']

executable('gwave', gwave_src,
           include_directories: [], dependencies: m_dep,
           gui_app: true)
