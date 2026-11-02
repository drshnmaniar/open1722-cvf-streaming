# Experiment 01: first live link

Goal: send a video from the sender to the receiver with the unmodified
Open1722 CVF examples.

## Commands

Receiver (started first):

    sudo ./build/examples/cvf/cvf-listener -i eth0 -d d8:3a:dd:70:0a:4d \
     | gst-launch-1.0 filesrc location=/dev/stdin ! h264parse ! avdec_h264 \
       ! videoconvert ! autovideosink

Sender:

    gst-launch-1.0 -e -q videotestsrc pattern=ball is-live=true \
      ! video/x-raw,width=192,height=144 ! x264enc key-int-max=30 \
      ! video/x-h264,stream-format=byte-stream ! filesink location=/dev/stdout \
      | sudo ./build/examples/cvf/cvf-talker -i eth0 -d d8:3a:dd:70:0a:4d -m 100

For a recording instead of a live display, the listener output was
redirected to a file (`> output.h264`) and the file was decoded with ffmpeg:

    ffmpeg -f h264 -framerate 30 -i output.h264 -f null -

## Results

- The stream works end to end: the moving ball was displayed live on
  the receiver.
- The listener prints many `Sequence number mismatch: expected X, got Y`
  messages, so some packets numbered by the talker never reach the
  listener's output. Gaps are mostly 2 to 8 packets, with a few larger.
- The recorded file (about 545 KB) decodes in ffmpeg with
  `Missing reference picture` errors, which fits the lost packets.
- GStreamer prints `A lot of buffers are being dropped` on the display
  sink. The listener writes a raw byte-stream without timestamps (as its
  README says), so the sink treats every frame as late.
- The receiver's network card shows 0 errors and 0 missed packets.

## Not yet determined

Whether the packets are lost on the wire or inside the listener. A
capture on both machines at the same time is needed for that:

    sudo tcpdump -i eth0 -nn -w capture.pcap ether proto 0x22f0

The first two attempts were not usable: in one the talker was started from
the wrong folder, and in the other the receiver capture ended before the
talker started.
