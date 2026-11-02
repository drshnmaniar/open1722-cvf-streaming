#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    const int port = 5000;
    GError *error = NULL;
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *message;
    int result = EXIT_SUCCESS;

    /* Initialize GStreamer before creating the pipeline */
    gst_init(&argc, &argv);

    pipeline = gst_parse_launch(
        "udpsrc port=5000 caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96\" ! "
        "rtph264depay ! "
        "h264parse ! "
        "avdec_h264 ! "
        "videoconvert ! "
        "autovideosink sync=false async=false",
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

    /* Start the pipeline to begin listening for incoming streams */
    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not start the receiver pipeline\n");
        gst_object_unref(pipeline);
        return EXIT_FAILURE;
    }

    g_print("Receiver listening for H.264 video on UDP port %d (press Ctrl+C to stop)\n", port);

    /* The bus listens for pipeline errors or EOS (End of Stream) */
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

    /* Clean up resources */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return result;
}