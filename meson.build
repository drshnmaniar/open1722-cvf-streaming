project(
    'open1722-cvf-streaming',
    'c',
    default_options: ['c_std=c11', 'warning_level=3'],
)

add_project_arguments('-Wpedantic', language: 'c')

gstreamer = dependency('gstreamer-1.0', version: '>=1.14')

sender = executable(
    'open1722-sender',
    'example/sender/h264-rtp-udp-sender.c',
    dependencies: gstreamer,
)

receiver = executable(
    'open1722-receiver',
    'src/main.c',
    dependencies: gstreamer,
)

test(
    'sender rejects invalid port',
    sender,
    args: ['127.0.0.1', '0'],
    should_fail: true,
)

test(
    'receiver rejects invalid port',
    receiver,
    args: ['65536'],
    should_fail: true,
)
