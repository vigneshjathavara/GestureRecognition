#pragma once


#include "reader.h"
//#include "ofMain.h"
//#include "GRT/GRT.h"
using namespace GRT;



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
    bool uprightCalibration, upsideDownCalibration,isCalibratedUp,isCalibratedUpD;
    GRT::MatrixDouble uprightData, upsideDownData;
    //ofSerial	serial;
};
