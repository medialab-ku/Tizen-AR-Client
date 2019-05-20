#ifndef __ZED_H__
#define __ZED_H__

#include <sl/Camera.hpp>
#include <opencv2/opencv.hpp>

class ZED
{
public:
    ZED();
    ZED(sl::InitParameters init);
    void GetImage(cv::Mat& left, cv::Mat& right);

private:
    void CameraInfoLog();
    cv::Mat slMat2cvMat(sl::Mat& input);
    sl::Camera zed;
    sl::InitParameters init_params;
    sl::Resolution image_size;
};

#endif