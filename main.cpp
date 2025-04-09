#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/features2d/features2d.hpp"
// #include "opencv2/nonfree/nonfree.hpp"
#include "opencv2/flann/flann.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <fstream>
#include <time.h>
#include <videostab.h>
#include <opencv2/core/utility.hpp>
#include <opencv2/core/core_c.h>
#include <opencv2/videoio.hpp>  // For newer versions
#include "matsender.h"

using namespace std;
using namespace cv;

MatUdpSender *sender = nullptr;

// This class redirects cv::Exception to our process so that we can catch it and handle it accordingly.
class cvErrorRedirector {
public:
    int cvCustomErrorCallback( )
    {
        std::cout << "A cv::Exception has been caught. Skipping this frame..." << std::endl;
        return 0;
    }

    cvErrorRedirector() {
        cvRedirectError((cv::ErrorCallback)cvCustomErrorCallback(), this);
    }
};

const int HORIZONTAL_BORDER_CROP = 30;

void print_instructions(const char* ipAddr) {
    printf("--------------------------------------------------------------------------------------\n");
    printf("This program stabilizes a video stream from a camera.\n");
    printf("   It assumes the video node is /dev/video0 \n");
    printf("   To run: %s <ipaddress> \n", ipAddr);
    printf("      <ipaddress> is the IP address of the receiver (eg: 192.168.2.1).\n");
    printf("   The receiver can pickup the data with the pipeline: \n");
    printf("      gst-launch-1.0 udpsrc port=5000 ! application/x-rtp,payload=26 ! rtpjpegdepay ! jpegdec ! videoconvert ! xvimagesink \n");
    printf("--------------------------------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
    print_instructions(argv[0]);

    cvErrorRedirector redir;
    
    //Create a object of stabilization class
    VideoStab stab;

    //Initialize the VideoCapture object
    VideoCapture cap(0);

    Mat frame_2, frame2;
    Mat frame_1, frame1;

    cap >> frame_1;
    cvtColor(frame_1, frame1, COLOR_BGR2GRAY);

    Mat smoothedMat(2,3,CV_64F);

    // VideoWriter outputVideo;
    // outputVideo.open("com.avi" , cv::VideoWriter::fourcc('X' , 'V' , 'I' , 'D'), 30 , frame_1.size());

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <host>" << std::endl;
        return -1;
    }
    auto host = argv[1];
    std::cout << "Initializing sender to host..." << host << std::endl;
    sender = new MatUdpSender(host, 5000);
    if (!sender->isInitialized && !sender->initialize()) {
        return false;
    }

    while(true)
    {
        try {
            cap >> frame_2;

            if(frame_2.data == NULL)
            {
                break;
            }

            cvtColor(frame_2, frame2, COLOR_BGR2GRAY);

            Mat smoothedFrame;

            smoothedFrame = stab.stabilize(frame_1 , frame_2);

            // outputVideo.write(smoothedFrame);

            // show the output video (uncomment to see the stabilized video)
            // imshow("Stabilized Video" , smoothedFrame);
            // send the frame
            sender->sendFrame(smoothedFrame);

            waitKey(10);

            frame_1 = frame_2.clone();
            frame2.copyTo(frame1);
        } catch (cv::Exception& e) {
            cap >> frame_1;
            cvtColor(frame_1, frame1, COLOR_BGR2GRAY);
        }

    }

    return 0;
}


