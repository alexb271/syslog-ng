#ifndef DUMMY_H_INCLUDED
#define DUMMY_H_INCLUDED

#include "driver.h"
#include "logthrdest/logthrdestdrv.h"

typedef struct {
    LogThreadedDestDriver super;
    GString *filename;
} DummyDestinationDriver;

LogDriver *dummy_dd_new(GlobalConfig *cfg);

void dummy_dd_set_filename(LogDriver *self, const gchar *filename);

#endif
