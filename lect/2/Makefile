DIR_SRC     	:= src
DIR_INCLUDE 	:= include
DIR_BUILD   	:= build
DIR_MISC    	:= misc
DIR_SYSTEMD		:= /etc/systemd/system
DIR_LIB			:= /usr/lib
DIR_HEADERS		:= /usr/include
DIR_SUDO		:= /usr/sbin
DIR_BIN			:= /usr/bin

PKG_NAME		:= mysyslog
PKG_VERSION		:= 1.0.0
DIR_DEBIAN_LIB	:= $(DIR_BUILD)/lib$(PKG_NAME)-$(PKG_VERSION)
DIR_DEBIAN_CL	:= $(DIR_BUILD)/$(PKG_NAME)-client-$(PKG_VERSION)
DIR_DEBIAN_D	:= $(DIR_BUILD)/$(PKG_NAME)d-$(PKG_VERSION)

CFLAGS          := -I$(DIR_INCLUDE) -DUSE_BACKEND_MYSYSLOG
CLIENT_SRC      := $(DIR_SRC)/mysyslog_client.c
CLIENT_OUT      := $(DIR_BUILD)/mysyslog-client
DAEMON_SRC      := $(DIR_SRC)/mysyslog_daemon.c
DAEMON_OUT      := $(DIR_BUILD)/mysyslogd
MYSYSLOG_SRC    := $(DIR_SRC)/mysyslog.c
MYSYSLOG_OUT    := $(DIR_BUILD)/drivers/libmysyslog.so

DRIVERS_CFLAGS  := -I$(DIR_INCLUDE) -fPIC -DUSE_BACKEND_MYSYSLOG -shared
DRIVERS_SRCS    := $(wildcard $(DIR_SRC)/drivers/*.c)
DRIVERS_SO		:= $(patsubst $(DIR_SRC)/drivers/%.c, $(DIR_BUILD)/drivers/lib%.so, $(DRIVERS_SRCS))


all: create_dirs drivers libmysyslog deb install_lib client daemon install

install: install_lib
	sudo dpkg -i $(DIR_DEBIAN_D).deb
	sudo dpkg -i $(DIR_DEBIAN_CL).deb

install_lib:
	sudo dpkg -i $(DIR_DEBIAN_LIB).deb

client: install_lib
	@cc $(CFLAGS) -o $(CLIENT_OUT) $(CLIENT_SRC) -lmysyslog

daemon: install_lib
	@cc $(CFLAGS) -o $(DAEMON_OUT) $(DAEMON_SRC) -lmysyslog

deb: libmysyslog_deb mysyslogd_deb mysyslog-client_deb

mysyslog-client_deb: libmysyslog_deb client
	@mkdir -p $(DIR_DEBIAN_CL)$(DIR_BIN)
	@cp -r $(DIR_MISC)/$(PKG_NAME)-client/* $(DIR_DEBIAN_CL)
	@cp $(DIR_BUILD)/$(PKG_NAME)-client $(DIR_DEBIAN_CL)$(DIR_BIN)
	dpkg-deb --root-owner-group --build $(DIR_DEBIAN_CL)

mysyslogd_deb: libmysyslog_deb daemon
	@mkdir -p $(DIR_DEBIAN_D)$(DIR_SUDO)
	@cp -r $(DIR_MISC)/$(PKG_NAME)d/* $(DIR_DEBIAN_D)
	@cp $(DIR_BUILD)/$(PKG_NAME)d $(DIR_DEBIAN_D)$(DIR_SUDO)
	dpkg-deb --root-owner-group --build $(DIR_DEBIAN_D)

libmysyslog_deb: create_dirs drivers libmysyslog
	@mkdir -p $(DIR_DEBIAN_LIB)/$(DIR_LIB)
	@mkdir -p $(DIR_DEBIAN_LIB)/$(DIR_HEADERS)
	@cp -r $(DIR_MISC)/lib$(PKG_NAME)/* $(DIR_DEBIAN_LIB)
	@cp $(DIR_BUILD)/drivers/*.so $(DIR_DEBIAN_LIB)/$(DIR_LIB)
	@cp $(DIR_INCLUDE)/mysyslog.h $(DIR_DEBIAN_LIB)/$(DIR_HEADERS)
	@cp $(DIR_INCLUDE)/mysyslog_back.h $(DIR_DEBIAN_LIB)/$(DIR_HEADERS)
	dpkg-deb --root-owner-group --build $(DIR_DEBIAN_LIB)

libmysyslog: create_dirs
	@cc $(DRIVERS_CFLAGS) -o $(MYSYSLOG_OUT) $(MYSYSLOG_SRC)

drivers: $(DRIVERS_SO)

$(DIR_BUILD)/drivers/lib%.so: $(DIR_SRC)/drivers/%.c
	@cc $(DRIVERS_CFLAGS) -o $@ $<

create_dirs:
	@mkdir -p $(DIR_BUILD)/drivers
	@mkdir -p $(DIR_DEBIAN_LIB)

clean:
	@rm -rf $(DIR_BUILD)
