LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := trigger
LOCAL_SRC_FILES := trigger.c elog.c memdef.c list.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -lc -ldl -lm -lz -llog
LOCAL_SHARED_LIBRARIES :=
LOCAL_CFLAGS := -Wno-write-strings -g -DRCS
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := ejson
LOCAL_SRC_FILES := jsonpro.c json.c json_helper.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -lc -ldl -lm -lz -llog
LOCAL_SHARED_LIBRARIES :=
LOCAL_CFLAGS := -Wno-write-strings -g -DRCS
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := eclient
LOCAL_SRC_FILES := recver.c conn.c ctrlagent.c eevent.c elistener.c \
	eparser.c erequest.c netagent.c netreq.c sender.c jsonpro.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -lc -ldl -lm -lz -llog
LOCAL_SHARED_LIBRARIES := trigger ejson
LOCAL_CFLAGS := -Wno-write-strings -g -DRCS
include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))
