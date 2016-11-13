#include "ofApp.h"





#define SERVICE_NAME "it_unbit_foohid"

#define FOOHID_CREATE 0  // create selector
#define FOOHID_SEND 2  // send selector

#define DEVICE_NAME "Foohid Virtual Mouse"
#define DEVICE_SN "SN 123456"

bool button = false;
unsigned char report_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

























bool start = false, sc=false, dc=false, ch=false;
int dcRound=0, scRound=0;

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
            
            if(std::string::npos != s.find("SC"))
            {
                cout<<"Single_Click"<<endl;
                sc = true;
            }
            else if(std::string::npos != s.find("DC"))
            {
                cout<<"Double_Click"<<endl;
                dc = true;
                
            }
            else if(std::string::npos != s.find("CH"))
            {
                cout<<"Click_Hold"<<endl;
                ch=true;
            }
            else if(std::string::npos != s.find("HR"))
            {
                cout<<"Hold_Release"<<endl;
                ch = false;
                
            }
            else if(std::string::npos != s.find("Tap"))
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
    
    
    isCalibrated = false;
    
    //zeroGs = VectorDouble(3);
    
    
    
    
    
    ret = IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceMatching(SERVICE_NAME), &iterator);
    
    if (ret != KERN_SUCCESS) {
        printf("Unable to access IOService.\n");
        ::exit(1);
    }
    
    // Iterate till success
    int found = 0;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, &connect);
        
        if (ret == KERN_SUCCESS) {
            found = 1;
            break;
        }
        
        IOObjectRelease(service);
    }
    IOObjectRelease(iterator);
    
    if (!found) {
        printf("Unable to open IOService.\n");
        ::exit(1);
    }
    
    // Fill up the input arguments.
    uint32_t input_count = 8;
    // uint64_t mouseInput[input_count];
    mouseInput[0] = (uint64_t) strdup(DEVICE_NAME);  // device name
    mouseInput[1] = strlen((char *)mouseInput[0]);  // name length
    mouseInput[2] = (uint64_t) report_descriptor;  // report descriptor
    mouseInput[3] = sizeof(report_descriptor);  // report descriptor len
    mouseInput[4] = (uint64_t) strdup(DEVICE_SN);  // serial number
    mouseInput[5] = strlen((char *)mouseInput[4]);  // serial number len
    mouseInput[6] = (uint64_t) 2;  // vendor ID
    mouseInput[7] = (uint64_t) 3;  // device ID
    
    ret = IOConnectCallScalarMethod(connect, FOOHID_CREATE, mouseInput, input_count, NULL, 0);
    if (ret != KERN_SUCCESS) {
        printf("Unable to create HID device. May be fine if created previously.\n");
    }
    
    // Arguments to be passed through the HID message.
    
    send_count = 4;
    //send[send_count];
    send[0] = (uint64_t)mouseInput[0];  // device name
    send[1] = strlen((char *)mouseInput[0]);  // name length
    send[2] = (uint64_t) &mouse;  // mouse struct
    send[3] = sizeof(struct mouse_report_t);  // mouse struct len
    
    conversionFactor = 100;
    mouse.buttons = 0;
}

//--------------------------------------------------------------
void ofApp::update(){
    
    
    
    // cout<<"Update1";
    
    MatrixDouble inputLocal;
    
    {
        std::lock_guard<std::mutex> guard(inputDataMutex);
        for (int i = 0; i < inputData.getNumRows(); i++)
            inputLocal.push_back(inputData.getRowVector(i));
        inputData.clear();
    }
    //cout<<"Update2";
    
    /*
     
     VectorDouble sample(2);
     sample[0] = mouseX;
     sample[1] = mouseY;
     
     
     mouse.buttons = 0;
     mouse.x = rand();
     mouse.y = rand();
     
     ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
     if (ret != KERN_SUCCESS) {
     printf("Unable to send message to HID device.\n");
     }
     
     */
    
    
    
    for (int i = 0; i < inputLocal.getNumRows(); i++)
    {
        //cout<<"Update3";
        vector<double> sample = inputLocal.getRowVector(i);
        VectorDouble result(3);
        //cout<<"X:"<<sample[0]<<"\nY:"<<sample[1]<<"\n";
        
        if(!uprightCalibration && !upsideDownCalibration && isCalibrated==true)
        {
            //00000cout<<"TEST!\n";
            for (int i = 0; i < 3; i++)
            {
                result[i] = (sample[i] - zeroGs[i]) / range;
            }
            //cout<<"X:"<<result[0]<<"\tY:"<<result[1]<<"\n";
            
            
            if(result[0]>=0.2 || result[0]<=-0.2)
            {
                //cout<<"Changing mouseX\n";
                mouse.x = result[0] * (-conversionFactor);
                
            }
            else
            {
                mouse.x =0;
            }
            
            if(result[1]>=0.2 || result[1] <=-0.2)
            {
                //cout<<"Changing MouseY\n";
                mouse.y = (result[1])*(conversionFactor);
            }
            else
            {
                mouse.y = 0;
            }
            
            if(sc)
            {
                if(scRound == 0)
                {
                    mouse.buttons =1;
                    scRound++;
                    cout<<"Single Click press\n";
                }
                else
                {
                    mouse.buttons = 0;
                    scRound = 0;
                    sc = false;
                    cout<<"Single Click Release\n";
                }
            }
            else if( dc)
            {
                if(dcRound == 0)
                {
                    mouse.buttons = 1;
                    dcRound++;
                    cout<<"Double Click press1\n";
                }
                else if(dcRound==1)
                {
                    mouse.buttons = 0;
                    dcRound++;
                    cout<<"Double Click release1\n";
                }
                else if(dcRound==2)
                {
                    mouse.buttons = 1;
                    dcRound++;
                    cout<<"Double Click press2\n";
                }
                else
                {
                    mouse.buttons=0;
                    dcRound = 0;
                    dc = false;
                    cout<<"Double Click release2\n";
                }
                
                
            }
            else if(ch)
            {
                mouse.buttons=1;
                cout<<"Click Hold\n";
            }
            else
            {
                mouse.buttons=0;
                cout<<"Release State\n";
            }
            /*if(button ==true)
            {
                mouse.buttons |=1;
                //button = false;
                unsigned char *p = (unsigned char*)&mouse;
                for (size_t i=0; i<sizeof(mouse); ++i)
                    printf("%.2x", p[i]);
                cout<<endl;
            }
            */
            
            //mouse.buttons = 0;
            
            
            
            
            ret = IOConnectCallScalarMethod(connect, FOOHID_SEND, send, send_count, NULL, 0);
            if (ret != KERN_SUCCESS) {
                printf("Unable to send message to HID device.\n");
            }
            
            
        }
        
        
        
        
        if( record )
        {
            
            
            
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
            
            
            
            
            pipeline.predict( result );
            
            int label = pipeline.getPredictedClassLabel();
            switch(label)
            {
                case 3:
                    cout<<"RightCircle\n";
                    break;
                case 6:
                    cout<<"LeftCircle\n";
                    break;
                case 5:
                    cout<<"Next\n";
                    break;
                case 4:
                    cout<<"Previous\n";
                    break;
                case 1:
                    cout<<"UP\n";
                    break;
                case 2:
                    cout<<"Down\n";
                    break;
                    
                    
                    
            }
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
            
        case '0':
            button = !button ;
            break;
            
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
            isCalibrated = true;
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
    /*  switch(key)
     {
     case '0':
     mouse.buttons = 0;
     break;
     }*/
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
