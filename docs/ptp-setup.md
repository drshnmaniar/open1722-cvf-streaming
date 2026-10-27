# GStreamer installation and PTP setup

Run on Raspberry Pi OS.

## Install GStreamer

```sh
sudo apt update
sudo apt install -y gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav v4l-utils
gst-launch-1.0 --version
```

Installs the tools, media plugins, and webcam inspection utilities. Confirmed sender version: 1.26.2.

## Install PTP tools

```sh
sudo apt install -y linuxptp ethtool
```

## Identify Ethernet and check timestamp support

```sh
ip -br link
ethtool -T eth0
```

Replace `eth0` if your Ethernet interface differs. Confirm `software-transmit`, `software-receive`, and `software-system-clock` are listed. All three were confirmed on the sender.

## Save the PTP configuration

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

Uses software timestamps, Ethernet transport, end-to-end delay measurement, domain 0, and informational logging. Saving the file does not start synchronisation.

## Run PTP

Connect Ethernet to the second Linux node directly or through a suitable switch, then run:

```sh
sudo ptp4l -i eth0 -f /etc/linuxptp/ptp4l-software.conf -m
```

The second node needs matching transport, delay mechanism, and domain settings, using its own Ethernet interface. Press Ctrl+C to stop.

## Progress

- [x] GStreamer installed and version checked.
- [x] Ethernet software timestamp support confirmed.
- [x] PTP configuration saved.
- [ ] PTP startup tested.
- [ ] Two-node synchronisation and offset measurements completed.

Related issue: [#4](https://github.com/drshnmaniar/open1722-cvf-streaming/issues/4).
