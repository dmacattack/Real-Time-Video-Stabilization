#include <opencv2/opencv.hpp>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <iostream>
#include "matsender.h"

/**
 * ctor
 */
MatUdpSender::MatUdpSender(const std::string &host, int port)
: pipeline(nullptr)
, appsrc(nullptr)
, loop(nullptr)
, bus(nullptr)
, busWatchId(0)
, isInitialized(false)
, host(host)
, port(port) {}

/**
 * dtor
 */
MatUdpSender::~MatUdpSender() {
    cleanup();
}

bool MatUdpSender::initialize() {
    // Initialize GStreamer
    GError *error = nullptr;
    if (!gst_init_check(NULL, NULL, &error)) {
        std::cerr << "Failed to initialize GStreamer: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    
    // Create the pipeline with JPEG encoding and UDP sink
    std::string pipelineStr = 
        "appsrc name=src format=time is-live=true ! "
        "videoconvert ! "
        "video/x-raw,format=I420 ! "
        "jpegenc quality=85 ! "
        "rtpjpegpay ! "
        "udpsink host=" + host + " port=" + std::to_string(port);
    
    pipeline = gst_parse_launch(pipelineStr.c_str(), &error);
    if (!pipeline) {
        std::cerr << "Failed to create pipeline: " << error->message << std::endl;
        g_error_free(error);
        return false;
    }
    
    // Get the appsrc element
    appsrc = gst_bin_get_by_name(GST_BIN(pipeline), "src");
    
    // Create main loop
    loop = g_main_loop_new(NULL, FALSE);
    
    // Listen to pipeline bus for messages
    bus = gst_element_get_bus(pipeline);
    busWatchId = gst_bus_add_watch(bus, busCallback, this);
    
    // Set pipeline to playing state
    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "Failed to set pipeline to playing state" << std::endl;
        cleanup();
        return false;
    }
    
    isInitialized = true;
    return true;
}

bool MatUdpSender::sendFrame(const cv::Mat &frame) {
    if (!isInitialized) {
        std::cerr << "Pipeline not initialized. Call initialize() first." << std::endl;
        return false;
    }
    
    // Make sure we have a valid frame
    if (frame.empty()) {
        std::cerr << "Empty frame provided" << std::endl;
        return false;
    }
    
    // Convert BGR to RGB if needed
    cv::Mat rgbFrame;
    if (frame.channels() == 3 && frame.type() == CV_8UC3) {
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);
    } else {
        rgbFrame = frame.clone();
    }
    
    // Calculate buffer size (RGB data)
    gsize bufferSize = rgbFrame.total() * rgbFrame.elemSize();
    
    // Create GstBuffer
    currentBuffer = gst_buffer_new_wrapped_full(
        GST_MEMORY_FLAG_READONLY,
        rgbFrame.data,
        bufferSize,
        0,
        bufferSize,
        nullptr,  // No destroy notify callback needed for this example
        bufferDestroyCallback
    );
    
    // Set buffer timestamp
    GST_BUFFER_PTS(currentBuffer) = GST_CLOCK_TIME_NONE;
    GST_BUFFER_DTS(currentBuffer) = GST_CLOCK_TIME_NONE;
    
    // Set caps if not already set
    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw",
        "format", G_TYPE_STRING, "RGB",
        "width", G_TYPE_INT, rgbFrame.cols,
        "height", G_TYPE_INT, rgbFrame.rows,
        "framerate", GST_TYPE_FRACTION, 30, 1,
        NULL
    );
    
    gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);
    
    // Push buffer to appsrc
    GstFlowReturn flowReturn = gst_app_src_push_buffer(GST_APP_SRC(appsrc), currentBuffer);
    gst_caps_unref(caps);
    
    if (flowReturn != GST_FLOW_OK) {
        std::cerr << "Failed to push buffer: " << flowReturn << std::endl;
        return false;
    }
    
    return true;
}

void MatUdpSender::cleanup() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
    
    if (appsrc) {
        gst_object_unref(appsrc);
        appsrc = nullptr;
    }
    
    if (bus) {
        gst_object_unref(bus);
        bus = nullptr;
    }
    
    if (busWatchId > 0) {
        g_source_remove(busWatchId);
        busWatchId = 0;
    }
    
    if (loop) {
        g_main_loop_unref(loop);
        loop = nullptr;
    }
    
    isInitialized = false;
}

// Bus callback function
gboolean MatUdpSender::busCallback(GstBus *bus, GstMessage *message, gpointer data) {
    MatUdpSender *sender = static_cast<MatUdpSender*>(data);
    
    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(message, &err, &debug);
            std::cerr << "GStreamer Error: " << err->message << std::endl;
            g_error_free(err);
            g_free(debug);
            g_main_loop_quit(sender->loop);
            break;
        }
        case GST_MESSAGE_EOS:
            g_main_loop_quit(sender->loop);
            break;
        default:
            break;
    }
    return TRUE;
}

// // Example usage function
// bool sendMatOverUdp(const cv::Mat &frame, const std::string &host = "127.0.0.1", int port = 5000) {
//     static MatUdpSender sender(host, port);
    
//     // Initialize sender if not already initialized
//     if (!sender.isInitialized && !sender.initialize()) {
//         return false;
//     }
    
//     // Send the frame
//     return sender.sendFrame(frame);
// }