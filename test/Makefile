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
ALL_T		= test_trigger eserver test_ejson test_mjson

# all object
CORE_OBJ    = 
CLIENT_OBJ	= test_trigger.o
JSON_OBJ    = test_ejson.o 
MJSON_OBJ   = test_mjson.o
SERVER_OBJ	= eserver.o
ALL_OBJ     = $(CORE_OBJ) $(CLIENT_OBJ) $(SERVER_OBJ) $(JSON_OBJ)

# .so .a
LIBCORE  	=
ALL_LIB     = $(LIBCORE)

# Targets start here
all: $(ALL_OBJ) $(ALL_LIB) $(ALL_T)

test_trigger: $(CORE_OBJ) $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ -lpthread $^

eserver: $(CORE_OBJ) $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ -lpthread $^

test_ejson: $(CORE_OBJ) $(JSON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lejson

test_mjson: $(CORE_OBJ) $(MJSON_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lejson

.c.o:
	gcc -c -fPIC -o $@ $^

clean:
	$(RM) $(ALL_OBJ) $(ALL_T) $(ALL_LIB)

# list targets that do not create files
.PHONY: clean all

# DO NOT DELETE
