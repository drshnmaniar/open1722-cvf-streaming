project(
    'open1722-cvf-streaming',
    'c',
    default_options: ['c_std=c11', 'warning_level=3'],
)

add_project_arguments('-Wpedantic', language: 'c')

gstreamer = dependency('gstreamer-1.0', version: '>=1.14')

talker = executable(
    'open1722-talker',
    'src/talker/main.c',
    dependencies: gstreamer,
)

listener = executable(
    'open1722-listener',
    'src/listener/main.c',
    dependencies: gstreamer,
)

test(
    'talker rejects invalid port',
    talker,
    args: ['127.0.0.1', '0'],
    should_fail: true,
)

test(
    'listener rejects invalid port',
    listener,
    args: ['65536'],
    should_fail: true,
)





