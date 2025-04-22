#include "videostab.h"
#include <cmath>

// Simplified parameters - only using translation
#define Q_TRANS 0.004
#define R_TRANS 0.5
#define MAX_FEATURES 50  // Reduced from 200
#define DOWNSAMPLE_FACTOR 2  // Process at half resolution

VideoStab::VideoStab()
{
    // Simplified state tracking (only translations)
    smoothedMat.create(2, 3, CV_64F);
    smoothedMat = Mat::eye(2, 3, CV_64F);
    
    // Initialize variables
    k = 1;
    
    // Only track translation errors and parameters
    errTransX = 1;
    errTransY = 1;
    
    Q_transX = Q_TRANS;
    Q_transY = Q_TRANS;
    
    R_transX = R_TRANS;
    R_transY = R_TRANS;
    
    sumTransX = 0;
    sumTransY = 0;
    
    smoothX = 0;
    smoothY = 0;
}

Mat VideoStab::stabilize(Mat &frame_1, Mat &frame_2)
{
    // Work at lower resolution
    Mat smallFrame1, smallFrame2;
    resize(frame_1, smallFrame1, Size(frame_1.cols/DOWNSAMPLE_FACTOR, frame_1.rows/DOWNSAMPLE_FACTOR));
    resize(frame_2, smallFrame2, Size(frame_2.cols/DOWNSAMPLE_FACTOR, frame_2.rows/DOWNSAMPLE_FACTOR));
    
    // Convert to grayscale
    cvtColor(smallFrame1, smallFrame1, COLOR_BGR2GRAY);
    cvtColor(smallFrame2, smallFrame2, COLOR_BGR2GRAY);
    
    // Fast feature detection
    vector<KeyPoint> keypoints;
    FAST(smallFrame1, keypoints, 20, true);
    
    // Limit number of keypoints
    if (keypoints.size() > MAX_FEATURES) {
        keypoints.resize(MAX_FEATURES);
    }
    
    // Convert keypoints to points
    vector<Point2f> points1;
    for (const auto& kp : keypoints) {
        points1.push_back(kp.pt);
    }
    
    // Calculate optical flow
    vector<Point2f> points2;
    vector<uchar> status;
    vector<float> err;
    calcOpticalFlowPyrLK(smallFrame1, smallFrame2, points1, points2, status, err);
    
    // Calculate average motion
    float dx = 0, dy = 0;
    int validPoints = 0;
    
    for (size_t i = 0; i < status.size(); i++) {
        if (status[i]) {
            dx += points2[i].x - points1[i].x;
            dy += points2[i].y - points1[i].y;
            validPoints++;
        }
    }
    
    // Get average displacement
    if (validPoints > 0) {
        dx /= validPoints;
        dy /= validPoints;
    }
    
    // Scale up displacement for original resolution
    dx *= DOWNSAMPLE_FACTOR;
    dy *= DOWNSAMPLE_FACTOR;
    
    // Accumulate motion
    sumTransX += dx;
    sumTransY += dy;
    
    // Apply Kalman filter smoothing (only for translations)
    if (k == 1) {
        k++;
        smoothX = sumTransX;
        smoothY = sumTransY;
    } else {
        // Simple Kalman update for translation only
        simpleKalmanFilter(&smoothX, &smoothY);
    }
    
    // Calculate correction
    float diffX = smoothX - sumTransX;
    float diffY = smoothY - sumTransY;
    
    // Create simplified transformation matrix (translation only)
    smoothedMat = Mat::eye(2, 3, CV_64F);
    smoothedMat.at<double>(0, 2) = diffX;
    smoothedMat.at<double>(1, 2) = diffY;
    
    // Apply stabilization transformation
    Mat stabilizedFrame;
    warpAffine(frame_1, stabilizedFrame, smoothedMat, frame_2.size());
    
    // Skip cropping and resizing to improve performance
    
    return stabilizedFrame;
}

// Simplified Kalman filter implementation (only for translation)
void VideoStab::simpleKalmanFilter(float *transX, float *transY)
{
    // Prediction step
    float predictX = *transX;
    float predictY = *transY;

    // Update prediction error
    float predictErrX = errTransX + Q_transX;
    float predictErrY = errTransY + Q_transY;

    // Calculate Kalman gain
    float gainX = predictErrX / (predictErrX + R_transX);
    float gainY = predictErrY / (predictErrY + R_transY);

    // Update state estimates
    *transX = predictX + gainX * (sumTransX - predictX);
    *transY = predictY + gainY * (sumTransY - predictY);

    // Update error estimates
    errTransX = (1 - gainX) * predictErrX;
    errTransY = (1 - gainY) * predictErrY;
}
