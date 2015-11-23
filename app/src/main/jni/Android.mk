LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)
LOCAL_MODULE    := target
LOCAL_SRC_FILES := target.cpp
LOCAL_C_INCLUDES += $(local_c_includes)
LOCAL_LDLIBS += -L/home/razaina/android-ndk-r10d/platforms/android-19/arch-arm/usr/lib -llog
LOCAL_LDFLAGS := -L/home/razaina/android-ndk-r10d/platforms/android-19/arch-arm/usr/lib/

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := spy
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := spy.c asm.S
LOCAL_C_INCLUDES += $(local_c_includes)
LOCAL_LDLIBS += -L/home/razaina/android-ndk-r10d/platforms/android-21/arch-arm/usr/lib -llog -landroid
LOCAL_CFLAGS := -I/home/razaina/android-ndk-r10d/platforms/android-21/arch-arm/usr/include/
LOCAL_LDFLAGS := -L/home/razaina/android-ndk-r10d/platforms/android-21/arch-arm/usr/lib/

include $(BUILD_EXECUTABLE)