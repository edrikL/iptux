# uninstall program
bin_PROGRAMS = ihateiptux
ihateiptux_SOURCES = ihateiptux.cpp utils.cpp

AM_CXXFLAGS = -Wall

# set the include path found by configure
ihateiptux_CPPFLAGS = $(all_includes) @PACKAGE_CFLAGS@

# the library search path.
ihateiptux_LDFLAGS = $(all_libraries) @PACKAGE_LIBS@

noinst_HEADERS = utils.h
