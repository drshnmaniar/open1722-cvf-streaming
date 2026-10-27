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

    /*
     * The receiver pipeline breaks down as follows:
     * - udpsrc: listens on UDP port 5000
     * - application/x-rtp caps: specifies payload type 96 and H.264 encoding to match the sender
     * - rtph264depay: extracts H.264 stream from RTP packets
     * - h264parse: parses H.264 elementary stream
     * - avdec_h264: decodes H.264 video frames
     * - videoconvert: converts video format for display
     * - autovideosink: renders the video output window (with low latency sync)
     */
    pipeline = gst_parse_launch(
        "udpsrc port=5000 caps=\"application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96\" ! "
        "rtph264depay ! "
        "h264parse ! "
        "avdec_h264 ! "
        "videoconvert ! "
        "autovideosink sync=false",
        &error);

    if (pipeline == NULL || error != NULL) {
        g_printerr("Could not create receiver pipeline: %s\n",
                   error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
        if (pipeline != NULL) {
            gst_object_unref(pipeline);
        }
        return EXIT_FAILURE;
    }

    /* Set state to PLAYING to start receiving the video stream */
    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) ==
        GST_STATE_CHANGE_FAILURE) {
        g_printerr("Could not start receiver pipeline\n");
        gst_object_unref(pipeline);
        return EXIT_FAILURE;
    }

    g_print("Listening for H.264 stream on port %d... (press Ctrl+C to stop)\n", port);

    /* Monitor pipeline bus for errors or EOS */
    bus = gst_element_get_bus(pipeline);
    message = gst_bus_timed_pop_filtered(
        bus, GST_CLOCK_TIME_NONE,
        GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (message != NULL && GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        GError *pipeline_error;
        gchar *debug_info = NULL;

        gst_message_parse_error(message, &pipeline_error, &debug_info);
        g_printerr("GStreamer receiver error: %s\n", pipeline_error->message);
        if (debug_info) {
            g_printerr("Debug info: %s\n", debug_info);
            g_free(debug_info);
        }
        g_error_free(pipeline_error);
        result = EXIT_FAILURE;
    }

    if (message != NULL) {
        gst_message_unref(message);
    }
    gst_object_unref(bus);

    /* Stop pipeline and free resources */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return result;
}