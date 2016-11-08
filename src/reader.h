//
//  reader.h
//  DTW
//
//  Created by Vignesh Jathavar on 11/5/16.
//
//

#ifndef reader_h
#define reader_h


#endif /* reader_h */





#include "ofMain.h"
#include "GRT/GRT.h"
#include<thread>

using namespace std;



extern bool start;

extern std::mutex inputDataMutex;

extern GRT::MatrixDouble inputData;

extern std::thread readerThread;

//void  readSerial(ofSerial serial);



extern void  readSerial();
