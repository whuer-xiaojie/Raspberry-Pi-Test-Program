CROSS_COMPILE   =arm-linux-gnueabihf-
CC              = $(CROSS_COMPILE)gcc
RM              = rm -f

CFLAGS  = -O2 -Wall
LDFLAGS = -s
LIBS    = -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt

OBJECTS =ilclient.o ilcore.o omx_image.o omx_typedef.o omx_resize.o omx_render.o

CFLAGS  +=  -DRASPBERRY_PI -DOMX_SKIP64BIT -DUSE_VCHIQ_ARM -I/opt/vc/include/ \
-I /opt/vc/include/interface/vmcs_host/ -I/opt/vc/include/interface/vcos/pthreads \
-I/opt/vc/include/interface/vmcs_host/linux 

LIBS    += -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -lEGL -lGLESv2 

#OBJECTS += main.o
OBJECTS +=omx_video.o

#PROGRAM  = omx_decode_image-x9
PROGRAM  = omx_video-x9

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRA)