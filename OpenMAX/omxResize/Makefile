ROSS_COMPILE   =arm-linux-gnueabihf-
CC              = $(CROSS_COMPILE)gcc
RM              = rm -f

CFLAGS  = -O2 -Wall
LDFLAGS = -s
LIBS    = -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt \

OBJECTS = mmapHelper.o omxHelper.o omxResize.o

CFLAGS  +=  -DRASPBERRY_PI -DOMX_SKIP64BIT -DUSE_VCHIQ_ARM -I/opt/vc/include/ \
-I /opt/vc/include/interface/vmcs_host/ -I/opt/vc/include/interface/vcos/pthreads \
-I/opt/vc/include/interface/vmcs_host/linux -I/usr/local/x9/include

LIBS    += -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -L/usr/local/x9/lib

OBJECTS += main.o

PROGRAM  = omxResize-x9

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRA)