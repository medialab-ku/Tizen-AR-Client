///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2018, STEREOLABS.
//
// All rights reserved.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


/********************************************************************************
 ** This sample demonstrates how to grab images and change the camera settings **
 ** with the ZED SDK                                                           **
 ********************************************************************************/

// #define ZED
#ifdef ZED
#include "ZED.h"
#include <string>

using namespace std;
using namespace sl;

ZED::ZED()
{
    InitParameters init_params;
    init_params.camera_resolution = RESOLUTION_VGA; /**< 672*376, available framerates: 15, 30, 60, 100 fps.*/
    init_params.camera_fps = 100;
    init_params.depth_mode = DEPTH_MODE_QUALITY;
    init_params.coordinate_units = UNIT_MILLIMETER;

    ERROR_CODE err = zed.open(init_params);
    if (err != SUCCESS)
    {
        cout << toString(err) << endl;
    }

    image_size = zed.getResolution();
    CameraInfoLog();
}

ZED::ZED(InitParameters init)
{
    ERROR_CODE err = zed.open(init);
    if (err != SUCCESS)
    {
        cout << toString(err) << endl;
    }

    image_size = zed.getResolution();
    CameraInfoLog();
}

void ZED::GetImage(cv::Mat& left, cv::Mat& right)
{
    Mat l(image_size.width, image_size.height, MAT_TYPE_8U_C4);
    Mat r(image_size.width, image_size.height, MAT_TYPE_8U_C4);

    RuntimeParameters runtime_parameters;
    rfind_package(CUDA 10.0 EXACT REQUIRED)

# include_directories(${CUDA_INCLUDE_DIRS})

# target_link_libraries(${PROJECT_NAME}
# ${CUDA_LIBRARY_DIRS}
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_core.so
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_input.so
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_zed.so
# )s.sensing_mode = SENSING_MODE_FILL;

    ifind_package(CUDA 10.0 EXACT REQUIRED)

# include_directories(${CUDA_INCLUDE_DIRS})

# target_link_libraries(${PROJECT_NAME}
# ${CUDA_LIBRARY_DIRS}
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_core.so
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_input.so
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_zed.so
# )ime_parameters) == SUCCESS) 
    {find_package(CUDA 10.0 EXACT REQUIRED)

# include_directories(${CUDA_INCLUDE_DIRS})

# target_link_libraries(${PROJECT_NAME}
# ${CUDA_LIBRARY_DIRS}
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_core.so
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_input.so
# ${PROJECT_SOURCE_DIR}/Thirdparty/lib/libsl_zed.so
# )
        // A new image and depth is available if grab() returns SUCCESS
        zed.retrieveImage(l, VIEW_LEFT); // Retrieve left image
        zed.retrieveImage(r, VIEW_RIGHT); // Retrieve left image
        //zed.retrieveMeasure(r, MEASURE_DEPTH); // Retrieve depth
    }
    cv::Mat temp, temp2;
    temp = slMat2cvMat(l);
    cv::cvtColor(temp, left, CV_BGRA2RGB);
    temp2 = slMat2cvMat(r);
    cv::cvtColor(temp2, right, CV_BGRA2RGB);
}

cv::Mat ZED::slMat2cvMat(Mat& input) 
{
    // Mapping between MAT_TYPE and CV_TYPE
    int cv_type = -1;
    switch (input.getDataType()) 
    {
        case MAT_TYPE_32F_C1: cv_type = CV_32FC1; break;
        case MAT_TYPE_32F_C2: cv_type = CV_32FC2; break;
        case MAT_TYPE_32F_C3: cv_type = CV_32FC3; break;
        case MAT_TYPE_32F_C4: cv_type = CV_32FC4; break;
        case MAT_TYPE_8U_C1: cv_type = CV_8UC1; break;
        case MAT_TYPE_8U_C2: cv_type = CV_8UC2; break;
        case MAT_TYPE_8U_C3: cv_type = CV_8UC3; break;
        case MAT_TYPE_8U_C4: cv_type = CV_8UC4; break;
        default: break;
    }

    // Since cv::Mat data requires a uchar* pointer, we get the uchar1 pointer from sl::Mat (getPtr<T>())
    // cv::Mat and sl::Mat will share a single memory structure
    return cv::Mat(input.getHeight(), input.getWidth(), cv_type, input.getPtr<sl::uchar1>(MEM_CPU));
}

void ZED::CameraInfoLog()
{
    cout << "ZED Model                 : " << toString(zed.getCameraInformation().camera_model) << endl;
    cout << "ZED Model                 : " << toString(zed.getCameraInformation().camera_model) << endl;
    cout << "ZED Serial Number         : " << zed.getCameraInformation().serial_number << endl;
    cout << "ZED Firmware              : " << zed.getCameraInformation().firmware_version << endl;
    cout << "ZED Camera Resolution     : " << (int) zed.getResolution().width << ", " << (int) zed.getResolution().height << endl;
    cout << "ZED Camera FPS            : " << (int) zed.getCameraFPS() << endl;
}

#endif