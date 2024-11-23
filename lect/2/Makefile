DIR_SRC     := src
DIR_INCLUDE := include
DIR_BUILD   := build

CFLAGS          := -I$(DIR_INCLUDE) -DUSE_BACKEND_MYSYSLOG
CLIENT_SRC      := $(DIR_SRC)/mysyslog_client.c
CLIENT_OUT      := $(DIR_BUILD)/mysyslog-client
MYSYSLOG_SRC    := $(DIR_SRC)/mysyslog.c
MYSYSLOG_OUT    := $(DIR_BUILD)/drivers/libmysyslog.so


DRIVERS_CFLAGS  := -I$(DIR_INCLUDE) -fPIC -DUSE_BACKEND_MYSYSLOG -shared
DRIVERS_SRCS    := $(wildcard $(DIR_SRC)/drivers/*.c)
DRIVERS_SO		:= $(patsubst $(DIR_SRC)/drivers/%.c, $(DIR_BUILD)/drivers/lib%.so, $(DRIVERS_SRCS))

all: create_dirs drivers libmysyslog client

client: create_dirs
	@cc $(CFLAGS) -o $(CLIENT_OUT) $(CLIENT_SRC)

libmysyslog: create_dirs
	@cc $(DRIVERS_CFLAGS) -o $(MYSYSLOG_OUT) $(MYSYSLOG_SRC)

drivers: $(DRIVERS_SO)

$(DIR_BUILD)/drivers/lib%.so: $(DIR_SRC)/drivers/%.c
	@cc $(DRIVERS_CFLAGS) -o $@ $<

create_dirs:
	@mkdir -p $(DIR_BUILD)/drivers

clean:
	@rm -rf $(DIR_BUILD)
