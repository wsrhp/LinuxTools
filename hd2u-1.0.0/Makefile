# makefile for 'hd2u'

### CONFIGURATION ###############################
# feel free to set following values as you want #
#################################################

prefix=/usr/local
exec_prefix=${prefix}
BIN_DIR=${exec_prefix}/bin

PACKAGE_NAME=hd2u
PACKAGE_VERSION=1.0.0

#################################################
# end of configuration                          #
#################################################

CC = gcc
CFLAGS = -g -O2 -Wall -pedantic
TARGET = dos2unix
SCRIPTS = \
	configure \
	install-sh
DOC = \
	AUTHORS \
	COPYING \
	CREDITS \
	ChangeLog \
	INSTALL \
	NEWS \
	README \
	TODO
INSTALL = /usr/bin/install -c
LIBS= -lpopt 

all:	$(TARGET)

$(TARGET):	$(TARGET).c config.h
	$(CC) $(CFLAGS) -o $@ $(@:%=%.c) $(LIBS)

rebuild:	cleanall $(TARGET)

clean:
	rm -f *.o
	rm -f core
	rm -f *~
	rm -f *.bak

cleanall:	clean
	rm -f *.out
	rm -f $(TARGET)
	rm -fr autom4te.cache
	rm -f config.log
	rm -f config.cache
	rm -f config.status
	rm -f Makefile
	rm -f configure

mrproper:	cleanall
distclean:	cleanall

install:
	$(INSTALL) -d -m 755 $(BUILD_ROOT)$(BIN_DIR)
	$(INSTALL) -m 755 -s $(TARGET) $(BUILD_ROOT)$(BIN_DIR)

tarball:
	$(INSTALL) -d -m 755 ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)
	$(INSTALL) -m 644 $(TARGET).c ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)
	$(INSTALL) -m 755 $(SCRIPTS) ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)
	$(INSTALL) -m 644 $(DOC) ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)
	$(INSTALL) -m 644 Makefile.in configure.in config.h.in ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)

	$(INSTALL) -d -m 755 ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)/packaging
	$(INSTALL) -m 644 packaging/hd2u.spec ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)/packaging

	tar cfz $(PACKAGE_NAME)-$(PACKAGE_VERSION).tgz $(PACKAGE_NAME)-$(PACKAGE_VERSION)
	rm -rf ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)/*
	rmdir ./$(PACKAGE_NAME)-$(PACKAGE_VERSION)
