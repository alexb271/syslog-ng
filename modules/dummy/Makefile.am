module_LTLIBRARIES      += modules/dummy/libdummy.la
modules_dummy_libdummy_la_SOURCES = \
  modules/dummy/dummy-grammar.y     \
  modules/dummy/dummy-parser.h      \
  modules/dummy/dummy-parser.c      \
  modules/dummy/dummy-worker.h      \
  modules/dummy/dummy-worker.c      \
  modules/dummy/dummy.h      \
  modules/dummy/dummy.c      \
  modules/dummy/dummy-plugin.c

BUILT_SOURCES       +=      \
  modules/dummy/dummy-grammar.y       \
  modules/dummy/dummy-grammar.c       \
  modules/dummy/dummy-grammar.h

EXTRA_DIST        +=      \
  modules/dummy-grammar.ym

modules_dummy_libdummy_la_CPPFLAGS  =     \
  $(AM_CPPFLAGS)            \
  -I$(top_srcdir)/modules/dummy        \
  -I$(top_builddir)/modules/dummy

modules_dummy_libdummy_la_LIBADD  = $(MODULE_DEPS_LIBS)
modules_dummy_libdummy_la_LDFLAGS = $(MODULE_LDFLAGS)
modules_dummy_libdummy_la_DEPENDENCIES= $(MODULE_DEPS_LIBS)
