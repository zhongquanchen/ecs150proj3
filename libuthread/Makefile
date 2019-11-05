# Target library
lib := libuthread.a
objs := queue.o thread.o sem.o  
REMOVE := libuthread.a sem.o  

CC := gcc
CFLAGS := -Wall -Werror -g
CFLAGS += -g

ifneq ($(V),1)
Q = @
endif

all: $(lib)
	@echo "ar libuthread.a"
	$(Q)ar ras $@ $^

sem.o: sem.o sem.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	$(Q)rm -f $(REMOVE)