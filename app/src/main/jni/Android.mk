LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# OpenCV configuration
# OPENCVROOT can be set via environment variable or will try common locations
ifndef OPENCVROOT
    # Try to read from a config file (created by Gradle if needed)
    OPENCV_CONFIG := $(LOCAL_PATH)/opencv_path.txt
    ifneq ($(wildcard $(OPENCV_CONFIG)),)
        OPENCVROOT := $(shell cat $(OPENCV_CONFIG))
    else
        # Try common OpenCV SDK locations
        OPENCVROOT := /Users/kazeru2806/Documents/OpenCV-android-sdk/sdk/native
        ifeq ($(wildcard $(OPENCVROOT)/jni/OpenCV.mk),)
            # Try alternative location
            OPENCVROOT := $(HOME)/Documents/OpenCV-android-sdk/sdk/native
        endif
    endif
endif

# Check for OpenCV.mk in different possible locations (OpenCV 2.4.x vs 4.x structure)
OPENCV_MK_PATH :=
# Try OpenCV 4.x structure: OPENCVROOT/jni/OpenCV.mk
ifneq ($(wildcard $(OPENCVROOT)/jni/OpenCV.mk),)
    OPENCV_MK_PATH := $(OPENCVROOT)/jni/OpenCV.mk
else
    # Try OpenCV 2.4.x structure: OPENCVROOT/sdk/native/jni/OpenCV.mk
    ifneq ($(wildcard $(OPENCVROOT)/sdk/native/jni/OpenCV.mk),)
        OPENCV_MK_PATH := $(OPENCVROOT)/sdk/native/jni/OpenCV.mk
    endif
endif

# If still not found, try parent directory (in case path points to sdk/native/jni)
ifeq ($(OPENCV_MK_PATH),)
    ifneq ($(wildcard $(OPENCVROOT)/OpenCV.mk),)
        OPENCV_MK_PATH := $(OPENCVROOT)/OpenCV.mk
    endif
endif

ifeq ($(OPENCV_MK_PATH),)
    $(warning OpenCV.mk not found. Tried:)
    $(warning   $(OPENCVROOT)/jni/OpenCV.mk)
    $(warning   $(OPENCVROOT)/sdk/native/jni/OpenCV.mk)
    $(warning   $(OPENCVROOT)/OpenCV.mk)
    $(warning Current OPENCVROOT: $(OPENCVROOT))
    $(warning Please set OPENCVROOT environment variable or ensure OpenCV SDK is at the expected location)
    $(error OpenCV SDK not found. Please download OpenCV Android SDK from https://opencv.org/releases/)
endif

OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC
include $(OPENCV_MK_PATH)

LOCAL_SRC_FILES := com_prasoon_panoramastitching_NativePanorama.cpp

LOCAL_LDLIBS += -llog
LOCAL_MODULE := MyLibs

include $(BUILD_SHARED_LIBRARY)