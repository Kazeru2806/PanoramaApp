#include "com_prasoon_panoramastitching_NativePanorama.h"
#include "opencv2/opencv.hpp"
#include "opencv2/stitching.hpp"
#include <android/log.h>

#define LOG_TAG "PanoramaNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using namespace std;
using namespace cv;

JNIEXPORT jint JNICALL Java_com_prasoon_panoramastitching_NativePanorama_processPanorama
  (JNIEnv *env, jclass clazz, jlongArray imageAddressArray, jlong outputAddress){
  jint ret=1;
  // Get the length of the long array
  jsize a_len = env->GetArrayLength(imageAddressArray);
  LOGI("Processing panorama with %d images", (int)a_len);
  
  if (a_len < 2) {
      LOGE("Need at least 2 images for stitching, got %d", (int)a_len);
      return 0;
  }
  
  // Convert the jlongArray to an array of jlong
  jlong *imgAddressArr = env->GetLongArrayElements(imageAddressArray, 0);
  // Create a vector to store all the image
  vector<Mat> imgVec;

    for(int k=0;k<a_len;k++){
        // Get the image
        Mat & curimage=*(Mat*)imgAddressArr[k];
        if (curimage.empty()) {
            LOGE("Image %d is empty!", k);
            env->ReleaseLongArrayElements(imageAddressArray, imgAddressArr ,0);
            return 0;
        }
        
        Mat newimage;

        // Convert to a 3 channel Mat to use with Stitcher module
        cvtColor(curimage, newimage, COLOR_RGBA2RGB);

        // Reduce the resolution for fast computation (max dimension 1000px)
        float scale = 1.0f;
        int maxDim = std::max(curimage.rows, curimage.cols);
        if (maxDim > 1000) {
            scale = 1000.0f / maxDim;
        }
        int newWidth = (int)(curimage.cols * scale);
        int newHeight = (int)(curimage.rows * scale);
        resize(newimage, newimage, Size(newWidth, newHeight));
        
        LOGI("Image %d: original %dx%d, resized to %dx%d", k, curimage.cols, curimage.rows, newWidth, newHeight);

        imgVec.push_back(newimage);
      }

  Mat & result  = *(Mat*) outputAddress;

  // OpenCV 4.x API: Use Stitcher::create() instead of createDefault()
  Ptr<Stitcher> stitcher = Stitcher::create(Stitcher::PANORAMA);
  
  // Configure stitcher settings (OpenCV 4.x API)
  stitcher->setRegistrationResol(-1); /// 0.6
  stitcher->setSeamEstimationResol(-1);   /// 0.1
  stitcher->setCompositingResol(-1);   //1
  stitcher->setPanoConfidenceThresh(-1);   //1
  stitcher->setWaveCorrection(true);
  stitcher->setWaveCorrectKind(detail::WAVE_CORRECT_HORIZ);
  
  LOGI("Starting stitching process...");
  Stitcher::Status status = stitcher->stitch(imgVec, result);
  
  const char* statusStr = "UNKNOWN";
  switch(status) {
      case Stitcher::OK: statusStr = "OK"; break;
      case Stitcher::ERR_NEED_MORE_IMGS: statusStr = "ERR_NEED_MORE_IMGS"; break;
      case Stitcher::ERR_HOMOGRAPHY_EST_FAIL: statusStr = "ERR_HOMOGRAPHY_EST_FAIL"; break;
      case Stitcher::ERR_CAMERA_PARAMS_ADJUST_FAIL: statusStr = "ERR_CAMERA_PARAMS_ADJUST_FAIL"; break;
  }
  LOGI("Stitching status: %s", statusStr);

   if (status != Stitcher::OK){
                  LOGE("Stitching failed with status: %s", statusStr);
                  ret=0;
   }else{
         if (result.empty()) {
             LOGE("Stitching returned OK but result is empty!");
             ret = 0;
         } else {
             LOGI("Stitching successful! Result size: %dx%d", result.cols, result.rows);
             cv::cvtColor(result, result, COLOR_BGR2RGBA, 4);
         }
   }

  // Release the jlong array
  env->ReleaseLongArrayElements(imageAddressArray, imgAddressArr ,0);
  return ret;

}


JNIEXPORT jstring JNICALL Java_com_prasoon_panoramastitching_NativePanorama_getMessageFromJni
  (JNIEnv *env, jclass obj){
  return env->NewStringUTF("This is a message from JNI");
  }