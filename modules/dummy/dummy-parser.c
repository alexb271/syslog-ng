#include "driver.h"
#include "cfg-parser.h"
#include "dummy-grammar.h"

extern int dummy_debug;

int dummy_parse(CfgLexer *lexer, LogDriver **instance, gpointer arg);

static CfgLexerKeyword dummy_keywords[] =
{
    {"dummy", KW_DUMMY},
    {"filename", KW_FILENAME},
    {NULL}
};

CfgParser dummy_parser =
{
#if SYSLOG_NG_ENABLE_DEBUG
  .debug_flag = &dummy_debug,
#endif
  .name = "dummy",
  .keywords = dummy_keywords,
  .parse = (gint (*)(CfgLexer *, gpointer *, gpointer)) dummy_parse,
  .cleanup = (void (*)(gpointer)) log_pipe_unref,
};

CFG_PARSER_IMPLEMENT_LEXER_BINDING(dummy_, DUMMY_, LogDriver **)
