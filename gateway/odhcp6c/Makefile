PREFIX   = /usr/local
BINDIR   = $(PREFIX)/sbin
BUILDDIR = $(SRC)
DESTDIR  = 
CFLAGS   = -g -std=c99 -D_GNU_SOURCE -Wall -Werror -Wextra -pedantic $(EXT_CFLAGS)
LDFLAGS  = $(EXT_LDFLAGS)
LIBS     = -lresolv

INSTALL  = install
SRC      = src

top ?= $(CURDIR)

CFLAGS += $(if $(EXT_PREFIX_CLASS),-DEXT_PREFIX_CLASS=$(EXT_PREFIX_CLASS),)
CFLAGS += $(if $(EXT_CER_ID),-DEXT_CER_ID=$(EXT_CER_ID),)

OBJS := odhcp6c.o dhcpv6.o ra.o script.o md5.o
HDRS := odhcp6c.h ra.h md5.h

all: $(BUILDDIR) dummy
	@cd $(BUILDDIR) && $(MAKE) top="$(top)" -f $(top)/Makefile odhcp6c

clean distclean:
	rm -f $(BUILDDIR)/*.o $(BUILDDIR)/odhcp6c

install: all install-common

install-common :
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 755 $(BUILDDIR)/odhcp6c $(DESTDIR)$(BINDIR)

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(OBJS:.o=.c) $(HDRS):
	ln -s $(SRC)/$@ .

.c.o:
	$(CC) $(CFLAGS) -c $<

odhcp6c: $(HDRS) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 

.PHONY : all clean install install-common dummy
