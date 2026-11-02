# Building Open1722

Do this on both machines.

## Install the tools

    sudo apt update
    sudo apt install -y build-essential cmake git pkg-config \
      gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good \
      gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav

## Build

From the `Open1722` folder:

    cmake -B build -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_C_FLAGS="-Wall -Wextra -Wconversion -Wsign-conversion"
    cmake --build build -j4

This builds the libraries only. The CVF example programs are separate
targets and have to be built by name:

    cmake --build build --target cvf-listener cvf-talker -j4
    ls build/examples/cvf/

The folder should now contain `cvf-talker` and `cvf-listener`.

## Command line options

    cvf-talker   -i IFNAME  -d MACADDR  [-m MSEC]  [-p NUM]
    cvf-listener -i IFNAME  -d MACADDR

| Option | Meaning |
|---|---|
| `-i`, `--ifname` | network interface (`eth0`) |
| `-d`, `--dst-addr` | stream destination MAC address |
| `-m`, `--max-transit-time` | talker only: maximum transit time in ms |
| `-p`, `--prio` | talker only: `SO_PRIORITY` set on the socket |

There is no stream ID option. Both programs need `sudo` because they use
raw Ethernet sockets.
