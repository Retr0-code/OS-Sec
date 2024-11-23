DIR_SRC     	:= src
DIR_INCLUDE 	:= include
DIR_BUILD   	:= build
DIR_MISC    	:= misc
DIR_SYSTEMD		:= /etc/systemd/system
DIR_LIB			:= /usr/lib
DIR_HEADERS		:= /usr/include

PKG_NAME		:= mysyslog
PKG_VERSION		:= 1.0.0
DIR_DEBIAN		:= $(DIR_BUILD)/$(PKG_NAME)-$(PKG_VERSION)

CFLAGS          := -I$(DIR_INCLUDE) -DUSE_BACKEND_MYSYSLOG
CLIENT_SRC      := $(DIR_SRC)/mysyslog_client.c
CLIENT_OUT      := $(DIR_BUILD)/mysyslog-client
MYSYSLOG_SRC    := $(DIR_SRC)/mysyslog.c
MYSYSLOG_OUT    := $(DIR_BUILD)/drivers/libmysyslog.so

DRIVERS_CFLAGS  := -I$(DIR_INCLUDE) -fPIC -DUSE_BACKEND_MYSYSLOG -shared
DRIVERS_SRCS    := $(wildcard $(DIR_SRC)/drivers/*.c)
DRIVERS_SO		:= $(patsubst $(DIR_SRC)/drivers/%.c, $(DIR_BUILD)/drivers/lib%.so, $(DRIVERS_SRCS))


all: create_dirs drivers libmysyslog deb

install:
	sudo dpkg -i $(DIR_BUILD)/$(PKG_NAME)-$(PKG_VERSION).deb

client: install
	@cc $(CFLAGS) -o $(CLIENT_OUT) $(CLIENT_SRC) -lmysyslog

deb: create_dirs drivers libmysyslog
	@mkdir -p $(DIR_DEBIAN)/DEBIAN
	@cp $(DIR_MISC)/control $(DIR_DEBIAN)/DEBIAN/control
	@mkdir -p $(DIR_DEBIAN)/$(DIR_SYSTEMD)
	@cp $(DIR_MISC)/mysyslog.service $(DIR_DEBIAN)/$(DIR_SYSTEMD)
	@mkdir -p $(DIR_DEBIAN)/$(DIR_LIB)
	@cp $(DIR_BUILD)/drivers/*.so $(DIR_DEBIAN)/$(DIR_LIB)
	@mkdir -p $(DIR_DEBIAN)/$(DIR_HEADERS)
	@cp $(DIR_INCLUDE)/mysyslog.h $(DIR_DEBIAN)/$(DIR_HEADERS)
	dpkg-deb --root-owner-group --build $(DIR_BUILD)/$(PKG_NAME)-$(PKG_VERSION)

libmysyslog: create_dirs
	@cc $(DRIVERS_CFLAGS) -o $(MYSYSLOG_OUT) $(MYSYSLOG_SRC)

drivers: $(DRIVERS_SO)

$(DIR_BUILD)/drivers/lib%.so: $(DIR_SRC)/drivers/%.c
	@cc $(DRIVERS_CFLAGS) -o $@ $<

create_dirs:
	@mkdir -p $(DIR_BUILD)/drivers
	@mkdir -p $(DIR_DEBIAN)

clean:
	@rm -rf $(DIR_BUILD)
