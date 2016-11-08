#include "ofApp.h"


bool start = false;

std::mutex inputDataMutex;

GRT::MatrixDouble inputData;

std::thread readerThread;

void  readSerial()
{
    double x,y,z;
    std::vector<unsigned char> buffer_;//character buffer holding chars read from device
    cout<<"READER::start="<<start<<"\n";
    
    //Setup Serial connection
    ofSerial serial;
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    int baud = 9600;
    serial.setup(0, baud);
    
    while(start)
    {
        const int kBufSize = 18;
        unsigned char buf[kBufSize];// temp store for read values
        //cout<<"Available:"<<serial.available()<<"\n";
        
        //sleep till data is available
        while(serial.available() <18)
        {
            //cout<<"sleeping\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        //read data
        while(serial.available()>=18){
            int result = serial.readBytes(buf, kBufSize);
            //cout<<"Result:"<<result<<"\n";
            if ( result == OF_SERIAL_ERROR ) {
                //ofLog( OF_LOG_ERROR, "unrecoverable error reading from serial" );
                break;
            } else if ( result != OF_SERIAL_NO_DATA ) {
                //cout<<"writing\n";
                //Add only read data to buffer_
                /*
                 for(int i=0;i<result;i++)
                 cout<<buf[i];
                 cout<<endl;
                */
                buffer_.insert(buffer_.end(), buf, buf + result);
            }
            
        }
        
        
        auto newline = find(buffer_.begin(), buffer_.end(), '\n');
        if (newline != buffer_.end())
        {
            //cout<<"extracting\n";
            std::string s(buffer_.begin(), newline);
            buffer_.erase(buffer_.begin(), ++newline);
            
            if(std::string::npos != s.find("Tap"))
            {
                cout<<"TAP"<<endl;
            }
            else
            {
                istringstream iss(s);
                vector<double> data;
                double d;
                
                while (iss >> d) data.push_back(d);
                
                if (data.size() > 0)
                {
                    std::lock_guard<std::mutex> guard(inputDataMutex);
                    inputData.push_back(data);
                    //std::cout<<data[0]<<"\t"<<data[1]<<"\t"<<data[2]<<"\n";
                }
            }
        }
    }
    
    
}


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(30);
    
    
    infoText = "";
    trainingClassLabel = 1;
    record = false;
    uprightCalibration = false;
    upsideDownCalibration = false;
    
    
    trainingData.setNumDimensions(3);
    
    
    DTW dtw(false,true,0.4);
    dtw.enableTrimTrainingData(true, 0.1, 75);
    
    
    //dtw.setOffsetTimeseriesUsingFirstSample(true);
    start = true;
    readerThread = std::thread(readSerial);
    
    pipeline.setClassifier( dtw );
    pipeline.addPostProcessingModule(ClassLabelTimeoutFilter(500));
    
    
    isCalibratedUp = false;
    
    //zeroGs = VectorDouble(3);
    
    
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    
    // cout<<"Update1";
    
    MatrixDouble input;
    
    {
        std::lock_guard<std::mutex> guard(inputDataMutex);
        for (int i = 0; i < inputData.getNumRows(); i++)
            input.push_back(inputData.getRowVector(i));
        inputData.clear();
    }
    //cout<<"Update2";
    
    /*
     
     VectorDouble sample(2);
     sample[0] = mouseX;
     sample[1] = mouseY;
     */
    
    
    
    for (int i = 0; i < input.getNumRows(); i++)
    {
        //cout<<"Update3";
        vector<double> sample = input.getRowVector(i);
        //cout<<"X:"<<sample[0]<<"\nY:"<<sample[1]<<"\n";
        if( record )
        {
            
            
            VectorDouble result(3);
            for (int i = 0; i < 3; i++)
            {
                result[i] = (sample[i] - zeroGs[i]) / range;
            }
            
            timeseries.push_back( result );
        }
        
        if(uprightCalibration)
        {
            uprightData.push_back(sample);
            cout<<"UprightData\n";
        }
        
        if(upsideDownCalibration)
        {
            upsideDownData.push_back(sample);
            cout<<"UpsideDownData\n";
        }
        
        
        if( pipeline.getTrained() )
        {
            VectorDouble result(3);
            for (int i = 0; i < 3; i++)
            {
                result[i] = (sample[i] - zeroGs[i]) / range;
            }
            
            
            
            pipeline.predict( result );
            
            int label = pipeline.getPredictedClassLabel();
            if(label == 3)
                cout<<"Circle\n";
            if(label == 5)
                cout<<"V\n";
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0, 0, 0);
    
    string text;
    int textX = 20;
    int textY = 20;
    
    //Draw the training info
    ofSetColor(255, 255, 255);
    text = "------------------- TrainingInfo -------------------";
    ofDrawBitmapString(text, textX,textY);
    
    if( record ) ofSetColor(255, 0, 0);
    else ofSetColor(255, 255, 255);
    textY += 15;
    text = record ? "RECORDING" : "Not Recording";
    ofDrawBitmapString(text, textX,textY);
    
    ofSetColor(255, 255, 255);
    textY += 15;
    text = "TrainingClassLabel: " + ofToString(trainingClassLabel);
    ofDrawBitmapString(text, textX,textY);
    
    textY += 15;
    text = "NumTrainingSamples: " + ofToString(trainingData.getNumSamples());
    ofDrawBitmapString(text, textX,textY);
    
    
    //Draw the prediction info
    textY += 30;
    text = "------------------- Prediction Info -------------------";
    ofDrawBitmapString(text, textX,textY);
    
    textY += 15;
    text =  pipeline.getTrained() ? "Model Trained: YES" : "Model Trained: NO";
    ofDrawBitmapString(text, textX,textY);
    
    textY += 15;
    text = "PredictedClassLabel: " + ofToString(pipeline.getPredictedClassLabel());
    ofDrawBitmapString(text, textX,textY);
    
    textY += 15;
    text = "Likelihood: " + ofToString(pipeline.getMaximumLikelihood());
    ofDrawBitmapString(text, textX,textY);
    
    textY += 15;
    text = "SampleRate: " + ofToString(ofGetFrameRate(),2);
    ofDrawBitmapString(text, textX,textY);
    
    
    //Draw the info text
    textY += 30;
    text = "InfoText: " + infoText;
    ofDrawBitmapString(text, textX,textY);
    
    textY +=30;
    //text = "X:"+ ofToString(x) +"\tY:"+ ofToString(y) +"\tZ:"+ ofToString(z);
    ofDrawBitmapString(text, textX,textY);
    //Draw the timeseries data
    /* if( record ){
     ofFill();
     for(UINT i=0; i<timeseries.getNumRows(); i++){
     double x = timeseries[i][0];
     double y = timeseries[i][1];
     double r = ofMap(i,0,timeseries.getNumRows(),0,255);
     double g = 0;
     double b = 255-r;
     
     ofSetColor(r,g,b);
     ofDrawEllipse(x,y,5,5);
     }
     }
     
     if( pipeline.getTrained() ){
     
     //Draw the data in the DTW input buffer
     DTW *dtw = pipeline.getClassifier< DTW >();
     
     if( dtw != NULL ){
     vector< VectorDouble > inputData = dtw->getInputDataBuffer();
     for(UINT i=0; i<inputData.size(); i++){
     double x = inputData[i][0];
     double y = inputData[i][1];
     double r = ofMap(i,0,inputData.size(),0,255);
     double g = 0;
     double b = 255-r;
     
     ofSetColor(r,g,b);
     ofDrawEllipse(x,y,5,5);
     }
     }
     }*/
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    infoText = "";
    
    switch ( key) {
            
        case '1':
            uprightCalibration = !uprightCalibration;
            if(!uprightCalibration)
            {
                //calibrate for upright data
                
                
            }
            break;
            
        case '2':
            upsideDownCalibration = !upsideDownCalibration;
            if(!upsideDownCalibration)
            {
                
                //calibrate for upsideDown data
            }
            break;
            
        case '3':
            for (int i = 0; i < 3; i++) {
                zeroGs.push_back((uprightData.getMean()[i] + upsideDownData.getMean()[i]) / 2);
                //zeroGs.push_back(i);
                
            }
            
            range = (uprightData.getMean()[2] - upsideDownData.getMean()[2]) / 2;
            uprightData.clear();
            upsideDownData.clear();
            break;
            
            
        case 'r':
            record = !record;
            if( !record ){
                trainingData.addSample(trainingClassLabel, timeseries);
                
                //Clear the timeseries for the next recording
                timeseries.clear();
            }
            break;
        case '[':
            if( trainingClassLabel > 1 )
                trainingClassLabel--;
            break;
        case ']':
            trainingClassLabel++;
            break;
        case 't':
            if( pipeline.train( trainingData ) ){
                infoText = "Pipeline Trained";
            }else infoText = "WARNING: Failed to train pipeline";
            break;
        case 's':
            if( trainingData.saveDatasetToFile("TrainingData.txt") ){
                infoText = "Training data saved to file";
            }else infoText = "WARNING: Failed to save training data to file";
            break;
        case 'l':
            if( trainingData.loadDatasetFromFile("TrainingData.txt") ){
                infoText = "Training data saved to file";
            }else infoText = "WARNING: Failed to load training data from file";
            break;
        case 'c':
            trainingData.clear();
            infoText = "Training data cleared";
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}

//--------------------------------------------------------------
void ofApp::exit() {
    if (readerThread.joinable()) {
        start = false;
        readerThread.join();
    }
}
