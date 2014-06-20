WORKDIR = ../..
BINDIR = $(WORKDIR)/bin
SRCDIR = $(WORKDIR)/src
LIBDIR = $(WORKDIR)/lib

MAKE = make
CC = arm-linux-gcc
CFLAGS = -Wall -pipe -g
LIBS =
LDFLAGS = -lpthread -lrt
INCLUDE = -I.
RM = rm -f
TARGETS = client
OBJS = socket.o wrap.o tcp_client_main.o mytools.o timeo.o logc.o alarm.o circleQueue.o messageHandler.o prot.o fileo.o
SUBDIRS =

all :  $(TARGETS)

$(TARGETS) : $(LIBS) $(OBJS)
	$(CC) $(CFLAGS)  $^ -o $@ $(LDFLAGS)
%.o : %.c
	$(CC) $(CFLAGS) -c $(INCLUDE) $< -o $@
clean :
	@for dir in $(SUBDIRS); \
		do $(MAKE) -C $$dir clean || exit 1; \
		done
	$(RM) $(OBJS) $(TARGETS)

.PHONY : all subdirs clean
