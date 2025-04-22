#ifndef VIDEOSTAB_H
#define VIDEOSTAB_H

#include <opencv2/opencv.hpp>
#include <opencv2/features2d.hpp>
#include <iostream>

#define HORIZONTAL_BORDER_CROP 20 // Horizontal border cropping amount

using namespace cv;
using namespace std;



class VideoStab
{
public:
    VideoStab();
    Mat stabilize(Mat &frame_1, Mat &frame_2);

private:
    // Current and previous frame in grayscale
    Mat frame1, frame2;

    // Transformation and smoothing matrices
    Mat smoothedMat;

    // Flag for first frame
    int k;

    // Variables for accumulated and smoothed motion
    float sumTransX, sumTransY;
    float smoothX, smoothY;

    // Kalman filter error variables
    float errTransX, errTransY;

    // Kalman filter process noise
    float Q_transX, Q_transY;

    // Kalman filter measurement noise
    float R_transX, R_transY;

    // Simplified Kalman filter function
    void simpleKalmanFilter(float *transX, float *transY);
};

#endif // VIDEOSTAB_H
