/*
 * Copyright (c) 2002-2013 Balabit
 * Copyright (c) 1998-2013 Balázs Scheidler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 *
 */
#include "poll-events.h"

void
poll_events_invoke_callback(PollEvents *self)
{
  self->callback(self->callback_data);
}

void
poll_events_set_callback(PollEvents *self, PollCallback callback, gpointer user_data)
{
  self->callback = callback;
  self->callback_data = user_data;
}

void
poll_events_set_checker(PollEvents *self, PollChecker check_watches, gpointer user_data)
{
  self->check_watches = check_watches;
  self->checker_data = user_data;
}

void
poll_events_init(PollEvents *self)
{
}

void
poll_events_free(PollEvents *self)
{
  if (self->free_fn)
    self->free_fn(self);
  g_free(self);
}
