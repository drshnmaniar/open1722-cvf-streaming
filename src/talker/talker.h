#ifndef OPEN1722_TALKER_H
#define OPEN1722_TALKER_H

#include <glib.h>

/*
 * Requires gst_init() first. Runs until EOS or an error; on failure, sets
 * error, which must point to NULL. The caller owns and must clear the error.
 */
gboolean talker_run(const char *device, const char *listener_ip, int port, GError **error);

#endif
