CC = arm-linux-gnueabihf-gcc
RM = rm -f
CFLAGS = -O2 -Wall

LIBS = -L /opt/vc/lib/ -lbcm_host -lvcos -lvchiq_arm -lpthread
CFLAGS += -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux

OBJECTS = Dispmanx.o

PROGRAM = Dispamnx-x9

all:$(PROGRAM)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
$(PROGRAM):$(OBJECTS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRA)
