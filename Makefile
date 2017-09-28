CFLAGS =-DWITH_OPENSSL -DWITH_NONAMESPACES -DWITH_DOM -I./ -I./import -g -fPIC
# -DDEBUG -g -O2
CC:=$(CROSS)gcc 

NVC_LIB = libsoap.so

BUILD_SRC = dom.c duration.c soapC.c stdsoap2.c \
	    	plugin/wsseapi.c \
		plugin/smdevp.c \
		plugin/mecevp.c \
		plugin/threads.c \
		plugin/wsaapi.c \

NVC_LIB_OBJS = soapC.o stdsoap2.o duration.o dom.o soapClient.o \
plugin/wsseapi.o plugin/smdevp.o plugin/mecevp.o plugin/threads.o plugin/wsaapi.o

all:$(NVC_LIB)

#build libraray
$(NVC_LIB):$(NVC_LIB_OBJS)
	@echo Creating $(NVC_LIB) library... 
	@gcc -fPIC -shared -o $(NVC_LIB) $(NVC_LIB_OBJS) 
clean:
	rm -f *.o *.a *.log plugin/*.o $(NVC_LIB)

