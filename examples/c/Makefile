# Build the example with pkg-config. If oct_parser is installed in a
# non-standard prefix, first do:
#   export PKG_CONFIG_PATH=<prefix>/lib/pkgconfig:$PKG_CONFIG_PATH
CC     ?= cc
CFLAGS ?= -O2 -Wall
OCTP_CFLAGS := $(shell pkg-config --cflags oct_parser)
OCTP_LIBS   := $(shell pkg-config --libs oct_parser)

example: example.c
	$(CC) $(CFLAGS) $(OCTP_CFLAGS) -o $@ $< $(OCTP_LIBS)

clean:
	rm -f example parser.log
