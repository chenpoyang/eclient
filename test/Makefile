####################################################################
# For building: ALL_T
# make version:	GNU Make 3.82
# g++ version:	gcc version 4.7.0
# gcc version:	g++ version 4.7.0
####################################################################

#### === CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT === ####

########## Compiler, tools and options

CC			= gcc
CFLAGS		= -O2 -Wall -g -fPIC $(DEFINES)
CXXFLAGS	= -O2 -Wall -g -fPIC $(DEFINES)

# all target
ALL_T		= test_trigger eserver

# all object
CORE_OBJ    = 
CLIENT_OBJ	= test_trigger.o
SERVER_OBJ	= eserver.o
ALL_OBJ     = $(CORE_OBJ) $(CLIENT_OBJ) $(SERVER_OBJ)

# .so .a
LIBCORE  	=
ALL_LIB     = $(LIBCORE)

# Targets start here
all:	$(ALL_OBJ) $(ALL_LIB) $(ALL_T)

test_trigger: $(CORE_OBJ) $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ -lpthread $^

eserver: $(CORE_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ -lpthread $^

#.c.o:
#	gcc -c -fPIC -o $@ $^

clean:
	$(RM) $(ALL_OBJ) $(ALL_T) $(ALL_LIB)

# list targets that do not create files
.PHONY: clean all

# DO NOT DELETE
global.o: global.c global.h
eserver.o: eserver.c global.h
test_trigger.o: test_trigger.c
