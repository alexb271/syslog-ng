#include "dbparser.h"
#include "logpatterns.h"
#include "radix.h"
#include "apphook.h"

#include <sys/stat.h>

struct _LogDBParser
{
  LogParser super;
  LogPatternDatabase db;
  gchar *db_file;
  time_t db_file_last_check;
  ino_t db_file_inode;
  time_t db_file_mtime;
};

static void
log_db_parser_reload_database(LogDBParser *self)
{
  struct stat st;
  LogPatternDatabase db_old;
  gchar *db_file_old;

  if (stat(self->db_file, &st) < 0)
    {
      msg_error("Error stating pattern database file, no automatic reload will be performed",
                evt_tag_str("error", g_strerror(errno)),
                NULL);
      return;
    }
  if ((self->db_file_inode == st.st_ino && self->db_file_mtime == st.st_mtime))
    {
      return;
    }
  self->db_file_inode = st.st_ino;
  self->db_file_mtime = st.st_mtime;
  db_old = self->db;
  db_file_old = self->db_file;

  if (!log_pattern_database_load(&self->db, self->db_file))
    {
      msg_error("Error reloading pattern database, no automatic reload will be performed", NULL);
      /* restore the old object pointers */
      self->db = db_old;
      self->db_file = db_file_old;
    }
  else
    {
      msg_notice("Log pattern database reloaded",
                 evt_tag_str("file", self->db_file),
                 evt_tag_str("version", self->db.version),
                 evt_tag_str("pub_date", self->db.pub_date),
                 NULL);
      /* free the old database if the new was loaded successfully */
      log_pattern_database_free(&db_old);
    }

}

NVHandle class_handle = 0;
NVHandle rule_id_handle = 0;

static inline void
log_db_parser_process_real(LogPatternDatabase *db, LogMessage *msg, GSList **dbg_list)
{
  LogDBResult *verdict;
  gint i;

  verdict = log_pattern_database_lookup(db, msg, dbg_list);
  if (verdict)
    {
      log_msg_set_value(msg, class_handle, verdict->class, -1);
      log_msg_set_value(msg, rule_id_handle, verdict->rule_id, -1);

      if (verdict->tags)
        {
          for (i = 0; i < verdict->tags->len; i++)
            log_msg_set_tag_by_id(msg, g_array_index(verdict->tags, guint, i));
        }

      if (verdict->values)
        {
          GString *result = g_string_sized_new(32);

          for (i = 0; i < verdict->values->len; i++)
            {
              log_template_format(g_ptr_array_index(verdict->values, i), msg, 0, TS_FMT_ISO, NULL, 0, 0, result);
              log_msg_set_value(msg, log_msg_get_value_handle(((LogTemplate *)g_ptr_array_index(verdict->values, i))->name), result->str, result->len);
            }
          g_string_free(result, TRUE);
        }
    }
  else
    {
      log_msg_set_value(msg, class_handle, "unknown", 7);
    }
}

static gboolean
log_db_parser_process(LogParser *s, LogMessage *msg, const char *input)
{
  LogDBParser *self = (LogDBParser *) s;

  if (G_UNLIKELY(self->db_file_last_check == 0 || self->db_file_last_check < msg->timestamps[LM_TS_RECVD].time.tv_sec - 5))
    {
      self->db_file_last_check = msg->timestamps[LM_TS_RECVD].time.tv_sec;
      log_db_parser_reload_database(self);
    }

  log_db_parser_process_real(&self->db, msg, NULL);
  return TRUE;
}

void
log_db_parser_process_lookup(LogPatternDatabase *db, LogMessage *msg, GSList **dbg_list)
{
  log_db_parser_process_real(db, msg, dbg_list);
}

void
log_db_parser_set_db_file(LogDBParser *self, const gchar *db_file)
{
  if (self->db_file)
    g_free(self->db_file);
  self->db_file = g_strdup(db_file);
}

static void
log_db_parser_post_config_hook(gint type, gpointer user_data)
{
  LogDBParser *self = (LogDBParser *) user_data;

  log_db_parser_reload_database(self);
}

static void
log_db_parser_free(LogParser *s)
{
  LogDBParser *self = (LogDBParser *) s;

  log_pattern_database_free(&self->db);

  if (self->db_file)
    g_free(self->db_file);
}

LogParser *
log_db_parser_new(void)
{
  LogDBParser *self = g_new0(LogDBParser, 1);

  self->super.process = log_db_parser_process;
  self->super.free_fn = log_db_parser_free;
  self->db_file = g_strdup(PATH_PATTERNDB_FILE);

  register_application_hook(AH_POST_CONFIG_LOADED, log_db_parser_post_config_hook, self);
  return &self->super;
}

void
log_db_parser_global_init(void)
{
  class_handle = log_msg_get_value_handle(".classifier.class");
  rule_id_handle = log_msg_get_value_handle(".classifier.rule_id");
}
