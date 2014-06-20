

MAKE = make
CC = gcc
CFLAGS = -Wall -pipe -g
LIBS =
LDFLAGS = -lpthread -lrt
INCLUDE = -I.
RM = rm -f
TARGETS = server
OBJS = db_controller.o my_pthread.o socket.o wrap.o  messageHandler.o  mytools.o timeo.c	logc.o circleQueue.o 	prot.o menu.o tcp_server_main.o my_io.o
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
