# Hardware and network

## Machines

| Role | Hostname | OS | Runs |
|---|---|---|---|
| Sender (webcam) | CVF-Sender | Debian on Raspberry Pi | `cvf-talker` |
| Receiver (display) | RTS-CVF | Debian on Raspberry Pi | `cvf-listener` |

## Cabling

The two Pis are connected by one Ethernet cable, with no switch in between.

## MAC addresses (`eth0`)

| Machine | MAC |
|---|---|
| Sender | `d8:3a:dd:af:2b:be` |
| Receiver | `d8:3a:dd:70:0a:4d` |

CVF is sent as raw Ethernet, so the talker needs the receiver's MAC as
the destination (`-d`). `wlan0` is not used.

## Checking the link

On each machine:

    ethtool eth0 | grep -E "Speed|Duplex|Link detected"

Result on both: `Speed: 1000Mb/s`, `Duplex: Full`, `Link detected: yes`.

## IP addresses (for ping and ssh only)

CVF itself does not use IP. We added temporary addresses to check the
link. They disappear after a reboot.

    # receiver
    sudo ip addr add 192.168.10.2/24 dev eth0
    # sender
    sudo ip addr add 192.168.10.1/24 dev eth0
    ping -c 3 192.168.10.2      # from the sender

Result: 3 of 3 replies, 0% loss, round trip about 0.2 to 0.5 ms.

## Clocks

The clocks are not synchronized with PTP. We agreed to leave that for
later.
