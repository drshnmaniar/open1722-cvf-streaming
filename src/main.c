#include <errno.h>
#include <gst/gst.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    unsigned long port = 5000;
    char *end = NULL;
    GError *error = NULL;
    GstElement *pipeline = NULL;
    GstElement *source;
    GstBus *bus = NULL;
    GstMessage *message = NULL;
    int result = EXIT_FAILURE;

    if (argc > 2) {
        g_printerr("Usage: %s [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        errno = 0;
        port = strtoul(argv[1], &end, 10);
        if (errno == ERANGE || end == argv[1] || *end != '\0' || port == 0 ||
            port > 65535) {
            g_printerr("Invalid port '%s'. Choose a port from 1 to 65535.\n",
                       argv[1]);
            return EXIT_FAILURE;
        }
    }

    gst_init(&argc, &argv);
    pipeline = gst_parse_launch(
        "udpsrc name=udp-source caps=\"application/x-rtp,media=video,"
        "encoding-name=H264,payload=96\" ! rtph264depay ! avdec_h264 ! "
        "videoconvert ! autovideosink sync=false",
        &error);
    if (pipeline == NULL || error != NULL) {
        g_printerr("Could not create pipeline: %s\n",
                   error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
        goto cleanup;
    }

    source = gst_bin_get_by_name(GST_BIN(pipeline), "udp-source");
    if (source == NULL) {
        g_printerr("Could not find the UDP input element\n");
        goto cleanup;
    }
    g_object_set(source, "port", (gint)port, NULL);
    gst_object_unref(source);

    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not start the pipeline\n");
        goto cleanup;
    }

    g_print("Listening for H.264 video on UDP port %lu "
            "(press Ctrl+C to stop)\n",
            port);

    bus = gst_element_get_bus(pipeline);
    if (bus == NULL) {
        g_printerr("Could not get the GStreamer message bus\n");
        goto cleanup;
    }

    message = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    if (message == NULL) {
        g_printerr("Stopped waiting for GStreamer messages unexpectedly\n");
        goto cleanup;
    }

    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        GError *pipeline_error = NULL;
        gchar *debug_info = NULL;

        gst_message_parse_error(message, &pipeline_error, &debug_info);
        g_printerr("GStreamer error: %s\n",
                   pipeline_error != NULL ? pipeline_error->message
                                          : "unknown error");
        g_clear_error(&pipeline_error);
        g_free(debug_info);
    } else {
        result = EXIT_SUCCESS;
    }

cleanup:
    if (message != NULL) {
        gst_message_unref(message);
    }
    if (bus != NULL) {
        gst_object_unref(bus);
    }
    if (pipeline != NULL) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }

    return result;
}
