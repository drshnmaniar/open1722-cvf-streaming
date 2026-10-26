# Raspberry Pi sender setup and PTP preparation

This document records the setup confirmed during the project session. Related tracking issue: [#4](https://github.com/drshnmaniar/open1722-cvf-streaming/issues/4).

## Current status

The sender Pi is accessible remotely, GStreamer is installed, the USB webcam has been identified, and the PTP software-timestamp configuration has been saved. Local recording, Open1722 streaming, and two-node clock synchronisation have not yet been demonstrated in this record.

## Operating system and remote access

- Prepared the SD card using Raspberry Pi Imager and booted the Pi.
- Connected from macOS using `ssh sender@RTS-CVF.local`.
- Confirmed desktop session files, including `labwc.desktop` and `rpd-labwc.desktop`.
- Signed in to Raspberry Pi Connect and successfully accessed the desktop.
- An accommodation network requires an additional browser login. Connect initially failed its API/WebSocket checks; access worked through a phone hotspot. Accommodation-network access remains unconfirmed.

## GStreamer and webcam inspection

Confirmed installed version:

```text
gst-launch-1.0 version 1.26.2
```

Commands used to inspect the camera:

```sh
v4l2-ctl --list-devices
v4l2-ctl -d /dev/video0 --list-formats-ext
```

The USB device identifies itself as `4K AutoFocus Webcam`. It exposes `/dev/video0`, `/dev/video1`, and `/dev/media4` in the observed setup. Device numbering can change after reconnecting hardware.

Selected advertised modes:

| Format | Resolution | Frame rate |
| --- | --- | --- |
| MJPEG | 1280 x 720 | 30 fps |
| MJPEG | 1920 x 1080 | 30 fps |
| MJPEG | 3840 x 2160 | 30 fps |
| YUYV | 1280 x 720 | 10 fps |
| YUYV | 1920 x 1080 | 5 fps |
| YUYV | 640 x 480 | 30 fps |

These are advertised modes, not measured capture performance. Native H.264 was not listed. The planned H.264 path therefore decodes MJPEG and encodes H.264 on the Pi. Recording commands were proposed, but no successful recording output has yet been recorded here.

## PTP: completed preparation

Goal: synchronise two Linux nodes using linuxptp with software timestamps.

### Interface inspection

```sh
ip -br link
ethtool -T eth0
```

The Ethernet interface is `eth0`. At the last inspection it was administratively up but reported `NO-CARRIER`; Wi-Fi (`wlan0`) was active.

Confirmed timestamp capabilities:

```text
software-transmit
software-receive
software-system-clock
PTP Hardware Clock: none
```

Software timestamping is supported. No hardware timestamping or PTP hardware clock was reported.

### Configuration saved on the Pi

Executed successfully:

```sh
sudo mkdir -p /etc/linuxptp
sudo tee /etc/linuxptp/ptp4l-software.conf >/dev/null <<'EOF'
[global]
time_stamping software
network_transport L2
delay_mechanism E2E
domainNumber 0
logging_level 6
EOF
```

Returning to the shell prompt without an error indicates the file-writing command completed. It does not start `ptp4l`.

| Setting | Purpose |
| --- | --- |
| `time_stamping software` | Use software timestamps and the system clock |
| `network_transport L2` | Carry PTP directly over Ethernet |
| `delay_mechanism E2E` | Use end-to-end delay measurement |
| `domainNumber 0` | Select PTP domain 0; the peer must match |
| `logging_level 6` | Enable informational logging |

## PTP: next steps, not yet completed

1. Confirm linuxptp is installed (`ptp4l -v`). Installation was instructed, but its output has not been recorded. If needed, install with `sudo apt install linuxptp`.
2. Connect `eth0` to an active Ethernet link.
3. Start the process in the foreground:

```sh
sudo ptp4l -i eth0 -f /etc/linuxptp/ptp4l-software.conf -m
```

4. Save startup logs and check for errors; stop with Ctrl+C.
5. When the second Linux node is available, configure matching transport, delay mechanism, and domain, using its own Ethernet interface name.
6. Check master/client role selection and clock convergence. Before the controlled test, identify other services adjusting the system clock so they do not compete with PTP on the synchronising node.
7. Record clock offsets and path delays over a defined measurement period, together with topology, software versions, and test conditions.

Only one Pi is currently available. A Pi selecting itself as grandmaster is a startup result, not evidence of synchronisation between nodes. No offset, accuracy, or convergence results are claimed. Software timestamp mode does not require a PHC-to-system-clock `phc2sys` setup.

## Open1722 inspection so far

The pasted CVF talker/listener source was inspected, but a repository checkout, build, or network test has not been confirmed. The examples accept H.264 byte-stream input/output and impose a 1,400-byte per-NAL limit. Fragmentation, payload mapping, frame markers, timing, packet-loss handling, and JPEG support remain audit/integration topics rather than completed changes.

## Progress checklist

- [x] Boot Pi and establish SSH access.
- [x] Access the desktop through Raspberry Pi Connect.
- [x] Confirm GStreamer version and inspect webcam formats.
- [x] Identify Ethernet interface and software timestamp support.
- [x] Save PTP configuration.
- [ ] Confirm linuxptp version and successful startup.
- [ ] Complete two-node PTP synchronisation and measurements.
- [ ] Demonstrate local webcam recording and H.264 encoding.
- [ ] Build and test the existing Open1722 examples.
