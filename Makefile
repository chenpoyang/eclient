###############################################################################
# For building: ALL_T
# make version:	GNU Make 3.82
# g++ version:	gcc version 4.7.0
# gcc version:	g++ version 4.7.0
###############################################################################

####### ===== CHANGE THE SETTINGS BELOW TO SUIT YOUR ENVIRONMENT ===== #######

######################### Compiler, tools and options #########################

CC			= gcc
CXX			= g++
# debug MACRO
##CFLAGS		= -O2 -Wall -g -g3 $(LDFLAGS)
##CXXFLAGS	= -O2 -Wall -g -g3 $(LDFLAGS)
# for Memcheck and debug MACRO
CFLAGS		= -O0 -Wall -g -g3 -fPIC $(LDFLAGS)
CXXFLAGS	= -O0 -Wall -g -g3 -fPIC $(LDFLAGS)
#CFLAGS		= -O2 -Wall -g $(LDFLAGS)
#CXXFLAGS	= -O2 -Wall -g $(LDFLAGS)
LDFLAGS		= -D_REENTRANT -DD_EME_SOCKET

# all target
ALL_T		= emain

# all object
CORE_OBJ    = trigger.o elog.o memdef.o list.o

CLIENT_OBJ	= recver.o conn.o ctrlagent.o ctrlhandler.o eevent.o elistener.o \
			  eparser.o erequest.o netagent.o netreq.o sender.o jsonpro.o
JSONP_OBJ   = jsonpro.o json.o json_helper.o
TEST_OBJ    = emain.o

ALL_OBJ     = $(CORE_OBJ) $(CLIENT_OBJ) $(TEST_OBJ) $(JSONP_OBJ)

# .so .a
LIBCORE  	= libtrigger.so
LIBECLIENT	= libeclient.so
LIBJSON	    = libejson.so
ALL_LIB     = $(LIBCORE) $(LIBECLIENT) $(LIBJSON)

# Targets start here
all:	$(ALL_OBJ) $(ALL_LIB) $(ALL_T)

emain: $(CORE_OBJ) $(CLIENT_OBJ) $(TEST_OBJ) $(JSONP_OBJ)
	$(CC) $(CFLAGS) -o $@ -lpthread $^

$(LIBCORE) : $(CORE_OBJ)
	$(CC) -fPIC -shared -o $@ $^

$(LIBECLIENT) : $(CLIENT_OBJ)
	$(CC) -fPIC -shared -o $@ $^

$(LIBJSON) : $(JSONP_OBJ)
	$(CC) -fPIC -shared -o $@ $^

clean:
	$(RM) $(ALL_OBJ) $(ALL_T) $(ALL_LIB)

# list targets that do not create files
.PHONY: clean all

# DO NOT DELETE(depend)
conn.o: conn.c common.h elog.h recver.h conn.h
ctrlagent.o: ctrlagent.c common.h elog.h trigger.h memdef.h list.h \
 ctrlagent.h erequest.h netreq.h eevent.h
ctrlhandler.o: ctrlhandler.c ctrlhandler.h
eevent.o: eevent.c eevent.h elog.h common.h
elistener.o: elistener.c elistener.h netreq.h common.h elog.h
elog.o: elog.c elog.h common.h
emain.o: emain.c erequest.h common.h elog.h memdef.h ctrlagent.h netreq.h \
 netagent.h trigger.h list.h emain.h conn.h
eparser.o: eparser.c common.h elog.h netreq.h eparser.h jsonpro.h \
 trigger.h memdef.h list.h elistener.h sender.h conn.h json.h
erequest.o: erequest.c erequest.h common.h elog.h trigger.h memdef.h \
 list.h
json.o: json.c json.h
json_helper.o: json_helper.c json_helper.h json.h
jsonpro.o: jsonpro.c jsonpro.h
list.o: list.c list.h
memdef.o: memdef.c memdef.h trigger.h list.h
netagent.o: netagent.c netagent.h netreq.h common.h elog.h ctrlagent.h \
 erequest.h trigger.h memdef.h list.h sender.h conn.h
netreq.o: netreq.c trigger.h memdef.h list.h netreq.h common.h elog.h \
 eparser.h elistener.h
recver.o: recver.c erequest.h common.h elog.h trigger.h memdef.h list.h \
 eparser.h netreq.h recver.h conn.h
sender.o: sender.c sender.h conn.h elog.h common.h
trigger.o: trigger.c common.h elog.h trigger.h memdef.h list.h
