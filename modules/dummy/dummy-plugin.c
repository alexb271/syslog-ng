#include "cfg-parser.h"
#include "plugin.h"
#include "plugin-types.h"

extern CfgParser dummy_parser;

static Plugin dummy_plugin[] =
{
  {
    .type = LL_CONTEXT_DESTINATION,
    .name = "dummy",
    .parser = &dummy_parser
  }
};

gboolean
dummy_module_init(PluginContext *context, CfgArgs *args)
{
  plugin_register(context, dummy_plugin, G_N_ELEMENTS(dummy_plugin));
  return TRUE;
}

const ModuleInfo module_info =
{
  .canonical_name = "dummy",
  .version = SYSLOG_NG_VERSION,
  .description = "Dummy module",
  .core_revision = SYSLOG_NG_SOURCE_REVISION,
  .plugins = dummy_plugin,
  .plugins_len = G_N_ELEMENTS(dummy_plugin),
};
