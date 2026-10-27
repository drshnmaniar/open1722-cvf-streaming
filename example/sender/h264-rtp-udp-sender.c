#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    const int port = 5000;
    const char *receiver_ip = "127.0.0.1";
    GError *error = NULL;
    GstElement *pipeline;
    GstElement *sink;
    GstBus *bus;
    GstMessage *message;
    int result = EXIT_SUCCESS;

    /* argc includes the program name: 1 means no IP, 2 means an IP was given. */
    if (argc > 2) {
        g_printerr("Usage: %s [receiver-ip]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Use the IP from argv[1], or keep 127.0.0.1 to send to this computer. */
    if (argc == 2) {
        receiver_ip = argv[1];
    }

    /* GStreamer must be initialized before its pipeline is created. */
    gst_init(&argc, &argv);

    /*
     * The ! symbols connect each step:
     * webcam -> format conversion -> H.264 encoding -> RTP packets -> UDP.
     * Payload type 96 must match the receiver's RTP caps.
     */
    pipeline = gst_parse_launch(
        "v4l2src device=/dev/video0 ! videoconvert ! "
        "x264enc tune=zerolatency speed-preset=ultrafast bitrate=2000 ! "
        "h264parse ! rtph264pay config-interval=1 pt=96 ! "
        "udpsink name=udp-sink sync=false async=false",
        &error);
    if (pipeline == NULL || error != NULL) {
        g_printerr("Could not create pipeline: %s\n",
                   error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
        if (pipeline != NULL) {
            gst_object_unref(pipeline);
        }
        return EXIT_FAILURE;
    }

    /* Tell the UDP output element where to send the video. */
    sink = gst_bin_get_by_name(GST_BIN(pipeline), "udp-sink");
    if (sink == NULL) {
        g_printerr("Could not find the UDP output element\n");
        gst_object_unref(pipeline);
        return EXIT_FAILURE;
    }
    g_object_set(sink, "host", receiver_ip, "port", port, NULL);
    gst_object_unref(sink);

    /* PLAYING starts the webcam capture and network transmission. */
    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not start the pipeline\n");
        gst_object_unref(pipeline);
        return EXIT_FAILURE;
    }

    g_print("Sending H.264 video to %s:%d (press Ctrl+C to stop)\n",
            receiver_ip, port);

    /* The bus carries messages from the pipeline, including errors. */
    bus = gst_element_get_bus(pipeline);
    message = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (message != NULL && GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        GError *pipeline_error;
        gchar *debug_info = NULL;

        gst_message_parse_error(message, &pipeline_error, &debug_info);
        g_printerr("GStreamer error: %s\n", pipeline_error->message);
        g_free(debug_info);
        g_error_free(pipeline_error);
        result = EXIT_FAILURE;
    }

    if (message != NULL) {
        gst_message_unref(message);
    }
    gst_object_unref(bus);

    /* Stop the pipeline and free its resources before exiting. */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return result;
}