/*
 * Copyright (c) 2017 Balabit
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */

#include <collection-comparator.h>
#include "directory-monitor-poll.h"
#include "messages.h"
#include "timeutils/misc.h"
#include "glib.h"
#include "glib/gstdio.h"

#include <iv.h>

typedef struct _DirectoryMonitorPoll
{
  DirectoryMonitor super;
  CollectionComparator *comparator;
  struct iv_timer rescan_timer;
} DirectoryMonitorPoll;

static void
_handle_new_entry(CollectionComparatorEntry *entry, gpointer user_data)
{
  DirectoryMonitorPoll *self = (DirectoryMonitorPoll *)user_data;
  if (self->super.callback)
    {
      DirectoryMonitorEvent event;

      event.name = entry->value;
      event.full_path = build_filename(self->super.real_path, event.name);
      event.event_type = g_file_test(event.full_path, G_FILE_TEST_IS_DIR) ? DIRECTORY_CREATED : FILE_CREATED;

      self->super.callback(&event, self->super.callback_data);

      g_free(event.full_path);
    }
}

static void
_handle_deleted_entry(CollectionComparatorEntry *entry, gpointer user_data)
{
  DirectoryMonitorPoll *self = (DirectoryMonitorPoll *)user_data;
  if (self->super.callback)
    {
      DirectoryMonitorEvent event;

      event.name = entry->value;
      event.event_type = FILE_DELETED;
      event.full_path = build_filename(self->super.real_path, event.name);

      self->super.callback(&event, self->super.callback_data);

      g_free(event.full_path);
    }
}

static void
_handle_deleted_self(DirectoryMonitorPoll *self)
{
  if (self->super.callback)
    {
      DirectoryMonitorEvent event;

      event.name = self->super.real_path;
      event.event_type = DIRECTORY_DELETED;
      event.full_path = self->super.real_path;

      self->super.callback(&event, self->super.callback_data);
    }
}

static void
_rescan_directory(DirectoryMonitorPoll *self, gboolean initial_scan)
{
  GError *error = NULL;
  GDir *directory = g_dir_open(self->super.real_path, 0, &error);

  if (FALSE == initial_scan)
    collection_comparator_start(self->comparator);

  if (directory)
    {
      const gchar *filename;
      while((filename = g_dir_read_name(directory)))
        {
          gchar *full_filename = build_filename(self->super.real_path, filename);
          GStatBuf file_stat;

          if (g_stat(full_filename, &file_stat) == 0)
            {
              g_free(full_filename);
              gint64 fids[2] = { file_stat.st_dev, file_stat.st_ino };
              if (initial_scan)
                collection_comparator_add_initial_value(self->comparator, fids, filename);
              else
                collection_comparator_add_value(self->comparator, fids, filename);
            }
          else
            {
              g_free(full_filename);
              msg_error("Error invoking g_stat() on file", evt_tag_str("filename", filename));
            }
        }
      g_dir_close(directory);

      if (FALSE == initial_scan)
        collection_comparator_stop(self->comparator);
    }
  else
    {
      if (FALSE == initial_scan)
        collection_comparator_stop(self->comparator);

      _handle_deleted_self(self);
      msg_debug("Error while opening directory",
                evt_tag_str("dirname", self->super.real_path),
                evt_tag_str("error", error->message));
      g_clear_error(&error);
    }
}

void
_rearm_rescan_timer(DirectoryMonitorPoll *self)
{
  iv_validate_now();
  self->rescan_timer.expires = iv_now;
  timespec_add_msec(&self->rescan_timer.expires, self->super.recheck_time);
  iv_timer_register(&self->rescan_timer);
}

static void
_start_watches(DirectoryMonitor *s)
{
  DirectoryMonitorPoll *self = (DirectoryMonitorPoll *)s;
  msg_trace("Starting to poll directory changes",
            evt_tag_str("dir", self->super.dir),
            evt_tag_int("freq", self->super.recheck_time));
  _rescan_directory(self, TRUE);
  _rearm_rescan_timer(self);
}


static void
_stop_watches(DirectoryMonitor *s)
{
  DirectoryMonitorPoll *self = (DirectoryMonitorPoll *)s;
  if (iv_timer_registered(&self->rescan_timer))
    iv_timer_unregister(&self->rescan_timer);
}

static void
_free_fn(DirectoryMonitor *s)
{
  DirectoryMonitorPoll *self = (DirectoryMonitorPoll *)s;
  collection_comparator_free(self->comparator);
}

static void
_triggered_timer(gpointer data)
{
  DirectoryMonitorPoll *self = (DirectoryMonitorPoll *)data;
  _rescan_directory(self, FALSE);
  _rearm_rescan_timer(self);
}

DirectoryMonitor *
directory_monitor_poll_new(const gchar *dir, guint recheck_time)
{
  DirectoryMonitorPoll *self = g_new0(DirectoryMonitorPoll, 1);
  directory_monitor_init_instance(&self->super, dir, recheck_time, "poll");

  IV_TIMER_INIT(&self->rescan_timer);
  self->rescan_timer.cookie = self;
  self->rescan_timer.handler = _triggered_timer;
  self->super.start_watches = _start_watches;
  self->super.stop_watches = _stop_watches;
  self->super.free_fn = _free_fn;
  self->comparator = collection_comparator_new();
  collection_comporator_set_callbacks(self->comparator,
                                      _handle_new_entry,
                                      _handle_deleted_entry,
                                      self);
  return &self->super;
}
