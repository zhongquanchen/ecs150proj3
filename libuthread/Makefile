# Target library
lib := libuthread.a
objs := queue.o thread.o sem.o tps.o
REMOVE := libuthread.a sem.o tps.o

CC := gcc
CFLAGS := -Wall -Werror -g
CFLAGS += -g

ifneq ($(V),1)
Q = @
endif

$(lib): $(objs)
	@echo "ar libuthread.a"
	$(Q)ar rcs $@ $^

sem.o: sem.c sem.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

tps.o: tps.c tps.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	$(Q)rm -f $(REMOVE)
