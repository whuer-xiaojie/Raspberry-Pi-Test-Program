ARCH          = x9
CROSS_COMPILE =arm-linux-gnueabihf-
CC            = $(CROSS_COMPILE)gcc
RM            = rm -f

CFLAGS  = -O2 -Wall
LDFLAGS = -s
LIBS    = -lgif -ljpeg -lmpeg2 -lmpeg2convert -lpthread

OBJECTS = box.o checksum.o clock.o config.o debug.o display.o frame.o gif.o graphics.o hiaboxc.o jpg.o keyboard.o lc100.o main.o mpg.o packet.o protocol.o region.o sensor-and-control-panel.o serial.o serial-udp.o socket-udp.o text.o udp.o v3-frame.o v3-packet.o v3scan.o wait-queue.o highway/bmp.o highway/display.o highway/protocol.o highway/tcp-client.o highway/tcp-server.o highway/text.o highway/udp-server.o svp/bmp.o svp/display.o svp/gif.o svp/jpg.o svp/mpg.o svp/protocol.o svp/tcp-client.o svp/tcp-server.o svp/text.o svp/udp-server.o

ifeq ($(ARCH),pc)
CFLAGS  += -I/usr/local/include -I/usr/local/include/SDL -D_REENTRANT
LIBS    += -L/usr/local/lib -lrt -lSDL2 libunix.a
OBJECTS += $(ARCH)/clock.o $(ARCH)/graphics.o $(ARCH)/v3check.o $(ARCH)/v3ss.o $(ARCH)/vram.o
PROGRAM = beacon
else
CFLAGS  += -I/usr/local/$(ARCH)/include -D__EMBEDDED__
LIBS    += -L/usr/local/$(ARCH)/lib libunix-$(ARCH).a
PROGRAM = beacon-$(ARCH)
endif

ifeq ($(ARCH),x6b)
CFLAGS  += -D__X6B__
OBJECTS += $(ARCH)/clock.o $(ARCH)/fpga.o $(ARCH)/graphics.o $(ARCH)/mon.o $(ARCH)/vram.o
endif

ifeq ($(ARCH),x9)
CFLAGS  += -D__X9__ -I/opt/vc/include -I/usr/local/$(ARCH)/include -D_REENTRANT -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
LIBS    += -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib  -L/opt/vc/src/hello_pi/libs/ilclient -L/opt/vc/src/hello_pi/libs/vgfont -L/usr/local/$(ARCH)/lib -lrt -lm -ldl -lGLESv2 -lSDL2 -lEGL libunix-$(ARCH).a -lbcm_host -lvcos -lvchiq_arm
OBJECTS += pc/clock.o pc/vram.o x9/graphics.o x9/firmware.o x9/fpga.o x9/lcd.o
PROGRAM = beacon-$(ARCH)
endif

all: $(PROGRAM)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRAM)
