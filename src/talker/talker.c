#include "talker.h"

#include <gst/gst.h>

static void set_error_from_message(GstMessage *message, const char *prefix, GError **error) {
    GError *pipeline_error = NULL;
    gchar *debug_info = NULL;
    const gchar *source_name = GST_OBJECT_NAME(GST_MESSAGE_SRC(message));

    gst_message_parse_error(message, &pipeline_error, &debug_info);
    if (pipeline_error != NULL) {
        if (debug_info != NULL && debug_info[0] != '\0') {
            g_prefix_error(&pipeline_error, "%s from %s (debug: %s): ", prefix, source_name,
                           debug_info);
        } else {
            g_prefix_error(&pipeline_error, "%s from %s: ", prefix, source_name);
        }
        g_propagate_error(error, pipeline_error);
    } else {
        g_set_error(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                    "%s: GStreamer reported an unknown error from %s", prefix, source_name);
    }

    g_free(debug_info);
}

static GstElement *create_camera_pipeline(const char *device, GError **error) {
    GstElement *pipeline;
    GstElement *camera;

    /* Capture, encode H.264, packetize as RTP, then send over UDP. */
    pipeline = gst_parse_launch("v4l2src name=camera ! videoconvert ! "
                                "x264enc tune=zerolatency speed-preset=ultrafast bitrate=2000 ! "
                                "h264parse ! rtph264pay config-interval=1 pt=96 ! "
                                "udpsink name=udp-sink sync=false async=false",
                                error);
    /* Parsing may return a partial pipeline together with an error. */
    if (pipeline == NULL || *error != NULL) {
        if (pipeline != NULL) {
            gst_object_unref(pipeline);
        } else if (*error == NULL) {
            g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                                "Could not create the GStreamer pipeline");
        }
        return NULL;
    }

    /* Set the device after parsing so the camera path stays configurable. */
    camera = gst_bin_get_by_name(GST_BIN(pipeline), "camera");
    if (camera == NULL) {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Could not find the camera source");
        gst_object_unref(pipeline);
        return NULL;
    }

    g_object_set(camera, "device", device, NULL);
    gst_object_unref(camera);

    return pipeline;
}

gboolean talker_run(const char *device, const char *listener_ip, int port, GError **error) {
    GstElement *pipeline = NULL;
    GstElement *sink;
    GstBus *bus = NULL;
    GstMessage *message = NULL;
    gboolean success = FALSE;

    g_return_val_if_fail(error != NULL && *error == NULL, FALSE);

    if (device == NULL || device[0] == '\0') {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Camera device must not be empty");
        goto cleanup;
    }
    if (listener_ip == NULL || listener_ip[0] == '\0') {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Listener IP must not be empty");
        goto cleanup;
    }
    if (port < 1 || port > 65535) {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Listener port must be between 1 and 65535");
        goto cleanup;
    }

    pipeline = create_camera_pipeline(device, error);
    if (pipeline == NULL || *error != NULL) {
        goto cleanup;
    }

    sink = gst_bin_get_by_name(GST_BIN(pipeline), "udp-sink");
    if (sink == NULL) {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Could not find the UDP output element");
        goto cleanup;
    }
    g_object_set(sink, "host", listener_ip, "port", port, NULL);
    gst_object_unref(sink);

    /* Block until the pipeline ends or reports an error. */
    bus = gst_element_get_bus(pipeline);
    if (bus == NULL) {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Could not get the GStreamer message bus");
        goto cleanup;
    }

    if (gst_element_set_state(pipeline, GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE) {
        message = gst_bus_timed_pop_filtered(bus, GST_SECOND, GST_MESSAGE_ERROR);
        if (message != NULL) {
            set_error_from_message(message, "Could not start the pipeline", error);
        } else {
            g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                                "Could not start the pipeline; no GStreamer error was reported");
        }
        goto cleanup;
    }

    message =
        gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    if (message == NULL) {
        g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                            "Stopped waiting for GStreamer messages unexpectedly");
        goto cleanup;
    }

    if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
        set_error_from_message(message, "GStreamer pipeline error", error);
        goto cleanup;
    }

    success = TRUE;

cleanup:
    /* Release acquired references, then stop and release the owning pipeline. */
    if (message != NULL) {
        gst_message_unref(message);
    }
    if (bus != NULL) {
        gst_object_unref(bus);
    }
    if (pipeline != NULL) {
        if (gst_element_set_state(pipeline, GST_STATE_NULL) == GST_STATE_CHANGE_FAILURE) {
            if (*error == NULL) {
                g_set_error_literal(error, GST_CORE_ERROR, GST_CORE_ERROR_FAILED,
                                    "Could not stop the pipeline");
            } else {
                g_prefix_error(error, "Could not stop pipeline after an earlier failure: ");
            }
            success = FALSE;
        }
        gst_object_unref(pipeline);
    }

    return success;
}
