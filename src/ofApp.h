#pragma once


#include "reader.h"
#include <IOKit/IOKitLib.h>
//#include "ofMain.h"
//#include "GRT/GRT.h"
using namespace GRT;





struct mouse_report_t {
    uint8_t buttons;
    int8_t x;
    int8_t y;
};







class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit() final;
    
    
    GRT::LabelledTimeSeriesClassificationData trainingData;   //This will store our training data
    GRT::MatrixDouble timeseries;                             //This will store a single training sample
    GRT::GestureRecognitionPipeline pipeline;     //This is a wrapper for our classifier and any pre/post processing modules
    bool record;          //This is a flag that keeps track of when we should record training data
    GRT::UINT trainingClassLabel; //This will hold the current label for when we are training the classifier
    string infoText;         //This string will be used to draw some info messages to the main app window
    
    GRT::VectorDouble zeroGs;
    double range;
    bool uprightCalibration, upsideDownCalibration,isCalibrated;
    GRT::MatrixDouble uprightData, upsideDownData;
    //ofSerial	serial;
    
    
    
    io_iterator_t iterator;
    io_service_t service;
    io_connect_t connect;

    kern_return_t ret;
    struct mouse_report_t mouse;
    float conversionFactor;
    uint32_t send_count;
    uint64_t send[4];
    uint64_t mouseInput[8];
    
    
    
};
