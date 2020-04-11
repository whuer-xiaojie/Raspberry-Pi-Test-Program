CROSS_COMPILE =arm-linux-gnueabihf-
CC            = $(CROSS_COMPILE)gcc
RM            = rm -f

CFLAGS  = -O2 -w -Wl,-rpath,/usr/local/pc/lib  
LIBS    =  -lpng -lm -lrt -lz\

OBJECTS =main.o mypng.o resize.o ../common/backLayer.o ../common/imageLayer.o ../common/dispRender.o \
         ../common/clock.o ../dispResizer/dispResizer.o 

CFLAGS  += -DRASPBERRY_PI -DOMX_SKIP64BIT -DUSE_VCHIQ_ARM -D_REENTRANT -D__x9__ -D__lcd__\
        -I/opt/vc/include -I/usr/local/x9/include \
        -I/opt/vc/include/interface/vmcs_host/ -I/opt/vc/include/interface/vcos/pthreads \
        -I/opt/vc/include/interface/vmcs_host/linux \
        -mcpu=cortex-a7 -mfpu=neon-vfpv4 \
	    -mfloat-abi=hard
           
LIBS    += -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib -L/usr/local/x9/lib -L/opt/vc/src/hello_pi/libs/ilclient \
        -L/opt/vc/src/hello_pi/libs/vgfont -ldl -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm 
     
PROGRAM = imageRender

all: $(PROGRAM)


%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRA)
