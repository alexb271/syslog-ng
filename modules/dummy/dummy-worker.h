#ifndef DUMMY_WORKER_H_INCLUDED
#define DUMMY_WORKER_H_INCLUDED

#include "logthrdest/logthrdestdrv.h"
#include "thread-utils.h"

typedef struct DummyDestinationWorker
{
    LogThreadedDestWorker super;
    FILE *file;
    ThreadId thread_id;
} DummyDestinationWorker;

LogThreadedDestWorker *dummy_worker_new(LogThreadedDestDriver *driver, gint worker_index);

#endif
