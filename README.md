# Real-Time-Video-Stabilization

This is a basic implementation of a real time video stabilization algorithm based on my paper: <br>
http://www.sciencedirect.com/science/article/pii/S1877050916314624

### Prerequisites

The code requires the following 3rd Party Libraries

- opencv-2.4.9
- g++-5.4.1


### How to Run

- Clone the repository and compile using `g++` and `opencv`.
- Change the parameters of Kalman Filter in `videostab.cpp`. (optional)
- Give the path of your input file or webcam in `main.cpp`.
- Run the program using these dependencies: -lopencv_core -lopencv_calib3d -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_stitching -lopencv_videoio -lopencv_video -lopencv_xfeatures2d

#### Alternatively
- The `Linux` branch of this repository includes a Makefile, run `make` to create the executable


### April 9 2025

* I did a build of the desktop code and cleaned it up, so i remember how to use it
* I rebuilt witorch br2 so that i get a baseline on the opencv libs
* I updated the makefile, and found I needed to allocate the PKG_CONFIG variable
* It starts to build, but complains opencv_stitching is missing, so i added it 
   * need to sideload

### April 16 2025
* back up the makefile for witorch, IT ALMOST works, but it just so happens it works better in qt

### April 21 2025
* work with fakesinks to find the error
* works
``` c++
"appsrc name=src format=time is-live=true ! "
   "fakesink";
```
* works
``` c++
"appsrc name=src format=time is-live=true ! videoconvert ! video/x-raw,format=I420 ! jpegenc quality=85 ! fakesink"
```

* works
``` c++
"appsrc name=src format=time is-live=true ! videoconvert ! video/x-raw,format=I420 ! jpegenc quality=85 ! rtpjpegpay ! fakesink "
```

* I got it working, either my string concatenation with int was incorrect, or the sync=false was needed 
* I reduced the complexity of kalman filter, and the number of corners for improved latency. 
* WiTorch could be improved if the pipeline was tee'd and the translation pushed into existing pipeline
* Ideally, opencv is the industry standard in this type of thing & there should be hw accelerators available 
