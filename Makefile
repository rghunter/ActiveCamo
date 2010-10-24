# protocol01
# Dhananjay Raghunathan
#

#The arm-linux compiler
#CC=arm-linux-gcc
#The arm linux linker
#LD=arm-linux-ld
#The arm linux archiver
#AR=arm-linux-ar
CC=arm-angstrom-linux-gnueabi-gcc
LD=arm-angstrom-linux-gnueabi-ld
AR=arm-angstrom-linux-gnueabi-ar

#Your program name
PROGNAME=activeCamo

#CUSTOM_INCLUDES_FOLDER=/home/rdjay/work/helper_includes
#The list of objects to link
SRC= tracker3Revised.c camoControlInterface.c trackerLib.c

#The path to the libkorebot.so file
LIBPATH=/home/rghunter/development_lightV0.1_kernel2.6/libkorebot-1.11-kb1/build-korebot-2.6/lib
 
#The path to the libkorebot.h file
INCPATH+= -I /home/rghunter/development_lightV0.1_kernel2.6/libkorebot-1.11-kb1/build-korebot-2.6/include\
	  -I /home/rghunter/development_lightV0.1_kernel2.6/linux-2.6.23.1-kb1/include \
	-I /usr/local/khepIIItoolbox/Modules/commandline \
	-I /usr/local/khepIIItoolbox/Modules/khepera3 \
	-I /usr/local/khepIIItoolbox/Modules/i2cal \
	-I /usr/local/khepIIItoolbox/Modules/odometry_track \
	-I /usr/local/khepIIItoolbox/Modules/sensorGrab

MODULES+=/usr/local/khepIIItoolbox/Modules/commandline/commandline.o \
	/usr/local/khepIIItoolbox/Modules/khepera3/*.o \
	/usr/local/khepIIItoolbox/Modules/i2cal/i2cal.a \
	/usr/local/khepIIItoolbox/Modules/odometry_track/odometry_track.a \
	/usr/local/khepIIItoolbox/Modules/nmea/nmea.a \
	/usr/local/khepIIItoolbox/Modules/sensorGrab/sensorGrab.o

OBJ  += $(MODULES)
CFLAGS= $(INCPATH) 
LDFLAGS+= -lkorebot -L$(LIBPATH) -lm 

# Default rules
.PHONY: clean all 

all : $(PROGNAME) 

$(PROGNAME) : $(SRC) $(HDR) Makefile 
	$(CC) -o $(PROGNAME) -lkorebot $(SRC) $(OBJ) $(CFLAGS) $(LDFLAGS) -Wall

clean : 
	@echo removing object files
	@rm -f *.o
	@rm -f $(PROGNAME)

#include Makefile.include
