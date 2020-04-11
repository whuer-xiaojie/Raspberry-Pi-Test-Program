ARCH          = x9
CROSS_COMPILE =arm-linux-gnueabihf-
CC            = $(CROSS_COMPILE)gcc
RM            = rm -f

CFLAGS  = -O2 -w -Wl,-rpath,/usr/local/pc/lib  
LIBS    = -lgif -ljpeg -lpng -lpthread -lavcodec -lavutil -lswscale -lavformat -lfreetype -lxml2 -lm -lrt -lz\

OBJECTS =backgroundTest.o Common/backLayer.o Common/dispmanx_graphics.o
all: $(PROGRAM)

CFLAGS  += -DRASPBERRY_PI -DOMX_SKIP64BIT -DUSE_VCHIQ_ARM -D_REENTRANT -D__x9__ -D__lcd__\
        -I/opt/vc/include -I/usr/local/$(ARCH)/include -I/usr/local/$(ARCH)/include/libxml2/ \
        -I/opt/vc/include/interface/vmcs_host/ -I/opt/vc/include/interface/vcos/pthreads \
        -I/opt/vc/include/interface/vmcs_host/linux \
        -I/usr/local/$(ARCH)/include/freetype2  -mcpu=cortex-a7 -mfpu=neon-vfpv4 \
	    -mfloat-abi=hard

LIBS    += -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -L/usr/local/$(ARCH)/lib -L/opt/vc/src/hello_pi/libs/ilclient \
        -L/opt/vc/src/hello_pi/libs/vgfont -ldl -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm \
        -lswresample 

PROGRAM = Dispmanx-$(ARCH)

all: $(PROGRAM)
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)
	-$(RM) $@
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRA)