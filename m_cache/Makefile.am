SUBDIRS=wcrt

noinst_PROGRAMS=opt optdebug

# First program to build: The analyzer/optimizer for the user
opt_SOURCES=main.c

opt_CPPFLAGS=-D_PRINT

# Second program to build: The analyzer/optimizer for the developer to play around
optdebug_SOURCES=main_debug.c

optdebug_CPPFLAGS=-D_DEBUG -D_PRINT

# Additionally remove the core dumps on 'make clean' if there are any
CLEANFILES=core