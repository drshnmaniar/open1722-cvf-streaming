#include "talker.h"

#include <errno.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    /* Defaults are for running the talker and listener on the same machine. */
    int port = 5000;
    const char *listener_ip = "127.0.0.1";
    GError *error = NULL;

    if (argc > 3) {
        g_printerr("Usage: %s [listener-ip] [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    /*
     * listener-ip is the IP assigned to the listener machine's reachable
     * network interface; the OS routing table chooses the talker's interface.
     * The port must match the listener's UDP port. For a remote listener,
     * 127.0.0.1 is not its address.
     */
    if (argc >= 2) {
        listener_ip = argv[1];
    }
    if (argc == 3) {
        char *end = NULL;
        unsigned long parsed_port;

        errno = 0;
        parsed_port = strtoul(argv[2], &end, 10);
        if (errno == ERANGE || end == argv[2] || *end != '\0' || parsed_port == 0 ||
            parsed_port > 65535) {
            g_printerr("Invalid port '%s'. Choose a port from 1 to 65535.\n", argv[2]);
            return EXIT_FAILURE;
        }
        port = (int)parsed_port;
    }

    /* GStreamer must be initialized before its pipeline is created. */
    gst_init(&argc, &argv);

    /*
     * The talker module manages the GStreamer pipeline lifecycle. This
     * baseline sends H.264 in RTP over UDP (not raw CVF); payload type 96
     * must match the listener's caps. The listener must be running, and the
     * network route and firewall must allow UDP traffic to its address and port.
     */
    g_print("Starting H.264 video to %s:%d (press Ctrl+C to stop)\n", listener_ip, port);
    if (!talker_run("/dev/video0", listener_ip, port, &error)) {
        g_printerr("Could not run talker: %s\n", error != NULL ? error->message : "unknown error");
        g_clear_error(&error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
