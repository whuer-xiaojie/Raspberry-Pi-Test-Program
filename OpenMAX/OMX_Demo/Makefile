#CC             = gcc
CROSS_COMPILE   =arm-linux-gnueabihf-
CC              = $(CROSS_COMPILE)gcc
RM              = rm -f

CFLAGS  = -O2 -Wall
LDFLAGS = -s
LIBS    = -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread  
OBJECTS =ilclient.o ilcore.o OMX_Common.o
#OBJECTS +=  get_a_list_of_components.o
#OBJECTS += get_component_port_info.o
#OBJECTS += get_all_port_information.o
#OBJECTS += change_componet_state.o
#OBJECTS += ilclient_change_component_states.o
#OBJECTS +=ilclient_alloc_component_buffer.o
OBJECTS +=ilclient_decode_image.o


CFLAGS  +=  -DRASPBERRY_PI -DOMX_SKIP64BIT -DUSE_VCHIQ_ARM -I/opt/vc/include/ -I /opt/vc/include/interface/vmcs_host/ -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/src/hello_pi/libs/ilclient
LIBS    += -L/opt/vc/lib -Wl,-rpath,/opt/vc/lib  -L/opt/vc/src/hello_pi/libs/ilclient -lilclient -lEGL -lGLESv2 

#OBJECTS = il_decode_image.o

#PROGRAM = get_a_list_of_components-x9
#PROGRAM = get_component_port_info-x9
#PROGRAM = il_decode_image-x9
#PROGRAM = get_all_port_info-x9
#PROGRAM = change_component_state-x9
#PROGRAM = ilclient_change_component_states-x9
#PROGRAM = ilclient_alloc_component_buffer-x9
PROGRAM = ilclient_decode_image-x9

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(PROGRAM): $(OBJECTS)
	-$(RM) $@
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	-$(RM) $(OBJECTS) $(PROGRA)
