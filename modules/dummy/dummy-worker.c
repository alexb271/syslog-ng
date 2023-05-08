#include "dummy.h"
#include "dummy-worker.h"

#include "thread-utils.h"

#include <stdio.h>

static LogThreadedResult
_dw_insert(LogThreadedDestWorker *worker, LogMessage *msg)
{
    DummyDestinationWorker *self = (DummyDestinationWorker *)worker;
    GString *string_to_write = g_string_new("");

    g_string_printf(string_to_write, "thread_id=%lu message=%s\n",
                    self->thread_id, log_msg_get_value(msg, LM_V_MESSAGE, NULL));

    size_t return_value = fwrite(string_to_write->str, 1, string_to_write->len, self->file);
    gsize string_to_write_len = string_to_write->len;
    g_string_free(string_to_write, TRUE);

    if (return_value != string_to_write_len)
    {
        msg_error("Error while reading file");
        return LTR_NOT_CONNECTED;
    }
    if (fflush(self->file) != 0)
    {
        msg_error("Error while flushing file");
        return LTR_NOT_CONNECTED;
    }

    return LTR_SUCCESS;
}

static gboolean
_dw_connect(LogThreadedDestWorker *worker)
{
    DummyDestinationWorker *self = (DummyDestinationWorker *)worker;
    DummyDestinationDriver *owner = (DummyDestinationDriver *)worker->owner;

    self->file = fopen(owner->filename->str, "a");
    if (!self->file)
    {
        msg_error("Could not open file", evt_tag_error("error"));
        return FALSE;
    }
    return TRUE;
}

static void
_dw_disconnect(LogThreadedDestWorker *worker)
{
    DummyDestinationWorker *self = (DummyDestinationWorker *)worker;

    fclose(self->file);
}

static gboolean
_dw_init(LogThreadedDestWorker *worker)
{
    DummyDestinationWorker *self = (DummyDestinationWorker *)worker;

    self->thread_id = get_thread_id();

    return log_threaded_dest_worker_init_method(worker);
}

static void
_dw_deinit(LogThreadedDestWorker *worker)
{
    // free resources allocated during _thread_init

    log_threaded_dest_worker_deinit_method(worker);
}

static void
_dw_free(LogThreadedDestWorker *worker)
{
    // free resources allocated during new

    log_threaded_dest_worker_free_method(worker);
}

LogThreadedDestWorker *
dummy_worker_new(LogThreadedDestDriver *driver, gint worker_index)
{
    DummyDestinationWorker *self = g_new0(DummyDestinationWorker, 1);

    log_threaded_dest_worker_init_instance(&self->super, driver, worker_index);

    self->super.init = _dw_init;
    self->super.deinit= _dw_deinit;
    self->super.insert = _dw_insert;
    self->super.free_fn = _dw_free;
    self->super.connect = _dw_connect;
    self->super.disconnect = _dw_disconnect;

    return &self->super;
}
