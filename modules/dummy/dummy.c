#include "dummy.h"
#include "dummy-worker.h"
#include "dummy-parser.h"

#include "plugin.h"
#include "messages.h"
#include "misc.h"
#include "stats/stats-registry.h"
#include "logqueue.h"
#include "driver.h"
#include "plugin-types.h"
#include "logthrdest/logthrdestdrv.h"
#include "str-utils.h"

// Configuration

void
dummy_dd_set_filename(LogDriver *driver, const gchar *filename)
{
    DummyDestinationDriver *self = (DummyDestinationDriver *)driver;

    g_string_assign(self->filename, filename);
}

// Utilities

static gboolean
_dd_init(LogPipe *pipe)
{
    DummyDestinationDriver *self = (DummyDestinationDriver *)pipe;

    if (self->filename->len == 0)
    {
        g_string_assign(self->filename, "/tmp/dummy-output.txt");
    }

    if (!log_threaded_dest_driver_init_method(pipe))
    {
        return FALSE;
    }

    return TRUE;
}

gboolean
_dd_deinit(LogPipe *pipe)
{
    // free resources created during init

    return log_threaded_dest_driver_deinit_method(pipe);
}

static void
_dd_free(LogPipe *pipe)
{
    DummyDestinationDriver *self = (DummyDestinationDriver *)pipe;

    g_string_free(self->filename, TRUE);

    log_threaded_dest_driver_free(pipe);
}

static const gchar *
_format_stats_instance(LogThreadedDestDriver *driver)
{
    DummyDestinationDriver *self = (DummyDestinationDriver *)driver;
    static gchar stats_instance[1024];

    g_snprintf(stats_instance, sizeof(stats_instance),
               "dummy-destination,%s", self->filename->str);

    return stats_instance;
}

static const gchar *
_format_persist_name(const LogPipe *pipe)
{
    DummyDestinationDriver *self = (DummyDestinationDriver *)pipe;
    static gchar persist_name[1024];

    if (pipe->persist_name)
        g_snprintf(persist_name, sizeof(persist_name), "dummy-destination.%s", pipe->persist_name);
    else
        g_snprintf(persist_name, sizeof(persist_name), "dummy-destination.%s", self->filename->str);

    return persist_name;
}

LogDriver *dummy_dd_new(GlobalConfig *cfg)
{
    DummyDestinationDriver *self = g_new0(DummyDestinationDriver, 1);
    self->filename = g_string_new("");

    log_threaded_dest_driver_init_instance(&self->super, cfg);
    self->super.super.super.super.init = _dd_init;
    self->super.super.super.super.deinit = _dd_deinit;
    self->super.super.super.super.free_fn = _dd_free;

    self->super.format_stats_instance = _format_stats_instance;
    self->super.super.super.super.generate_persist_name = _format_persist_name;
    self->super.stats_source = stats_register_type("dummy-destination");
    self->super.worker.construct = dummy_worker_new;

    return (LogDriver *)self;
}
