#ifndef DUMMY_PARSER_H_INCLUDED
#define DUMMY_PARSER_H_INCLUDED

#include "cfg-parser.h"
#include "driver.h"

extern CfgParser dummy_parser;

CFG_PARSER_DECLARE_LEXER_BINDING(dummy_, DUMMY_, LogDriver **)

#endif
