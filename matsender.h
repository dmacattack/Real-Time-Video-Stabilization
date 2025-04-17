#ifndef MATSENDER_H
#define MATSENDER_H

#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

class MatUdpSender {
    
public:
    MatUdpSender(const std::string &host = "127.0.0.1", int port = 5000);
    ~MatUdpSender();

    bool initialize();
    bool sendFrame(const cv::Mat &frame);
    void cleanup();

public:
    bool isInitialized;
    
private:
    
    // Bus callback function
    static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer data);
    
    // Callback when buffer is no longer needed by GStreamer
    static void bufferDestroyCallback(gpointer data) {
        (void)(data);
        // This is called when GStreamer is done with our buffer
        // Nothing to do here as we manage the cv::Mat separately
    }

private:
    GstElement *pipeline;
    GstElement *appsrc;
    GMainLoop *loop;
    GstBus *bus;
    guint busWatchId;
    // bool isInitialized;
    std::string host;
    int port;
    // Need buffer to be available in global scope for callback
    GstBuffer *currentBuffer;

};

#endif
