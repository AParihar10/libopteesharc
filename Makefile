SRCS := src/libopteesharc.c src/sharc-cli.c
OBJS := $(SRCS:.c=.o)
CFLAGS := $(CFLAGS) -Iinclude $(OPTEE_CLIENT_EXPORT)/include
LDFLAGS := $(LDFLAGS) -L$(OPTEE_CLIENT_EXPORT)/lib -lteec
DEBUG := -ggdb

all: libopteesharc.so sharc-cli

$(OBJS): %.o : %.c
	$(CC) $(DEBUG) -fPIC $(CFLAGS) -c $< -o $@

sharc-cli: src/sharc-cli.o libopteesharc.so
	$(CC) $(DEBUG) -o $@ $+ $(LDFLAGS) -L. -lopteesharc

libopteesharc.so: src/libopteesharc.o
	$(CC) $(DEBUG) -fPIC -shared -Wl,-soname,libopteesharc.so.1 -o $@ $+ $(LDFLAGS)

clean:
	rm -rf $(OBJS) libopteesharc.so sharc-cli
