#include "CurieIMU.h"
#include "CuriePME.h"
#include <SerialFlash.h>
#include <SPI.h>

//Digital pin for flash chip CS pin
const int FlashChipSelect = 21;

//No of instances to be trained per gesture
const unsigned int trainingReps = 4;//training per gesture

//No of Gestures available
const unsigned int numberOfGestures = 4;

//Pin which tells to train or predict------To be replaced by flex
const unsigned int buttonPin = 4;

//Accelerometer Sample Rate
const unsigned int sampleRate = 200;

//Bytes one neuron can hold
const unsigned int vectorNumBytes = 128;

//Number returned when no Match found
const unsigned int noMatch = 0x7fff;

//No of samples that can fit in one neuron
const unsigned int samplesPerVector = (vectorNumBytes/3);

//Max No of raw acclerometer samples read per training or detection instance
const unsigned int sensorBufferSize = 2048;


//Max values that can be stored in int data type
const int IMULow = -32768;
const int IMUHigh = 32767;





/**************************************************************************************************************
    Function :Setup -> The function called only once at the start of the device. Used to setup the environment
    
    Input:   void -> 
    
    Output:  void -> 
***************************************************************************************************************/
void setup()
{
  Serial.begin(9600);
  
  //wait for serial
  while(!Serial);

  //seting the input button pin
  pinMode(buttonPin, INPUT);

  //Start the IMU
  CurieIMU.begin();

  //Start the PME
  CuriePME.begin();

  CurieIMU.setAccelerometerRate(sampleRate);
  CurieIMU.setAccelerometerRange(2);

  trainGestures();

  Serial.println("Training complete. Detection should work :) ");

  if (!SerialFlash.begin(FlashChipSelect)) 
  {
    Serial.println("Unable to access SPI Flash chip");
  }
}





/**************************************************************************************************************
    Function : Loop -> the main loop of the program. This function run continuously throughout.
    
    Input:   void -> 
    
    Output:  void -> 
***************************************************************************************************************/
void loop()
{
  byte vector[vectorNumBytes];
  unsigned int category;

  readFromIMU(vector);
   /*
    for(unsigned int i=0;i<vectorNumBytes-3;i+=3)
        {
              Serial.print(vector[i]);
              Serial.print("\t");
              Serial.print(vector[i+1]);
              Serial.print("\t");
              Serial.print(vector[i+2]);
              Serial.println();
          
        }
   */
  if(idleCheck(vector))
    category = noMatch;
  else  
    category = CuriePME.classify(vector, vectorNumBytes);

  if(category == noMatch)
  {
      Serial.println("Don't recognise that one-- try again.");
  } 
  else 
  {
      Serial.println(category);
  }
}





/**************************************************************************************************************
    Function : IdleCheck -> Check if the sample read for detection is a possible valid gesture movement or hand reamined
               idle.
    
    Input:  byte vector[] -> A Byte array of size vectorNumBytes containing condensed and filtered samples.  
    
    Output: bool -> true if the samples denote an Idle hand
                    false if the samples denote a possible valid gesture
***************************************************************************************************************/
bool idleCheck(byte vector[])
{
  byte oldX=vector[0], oldY=vector[1], oldZ=vector[2];
  int sumX=0, sumY=0, sumZ=0;
  for(unsigned int i=0;i<vectorNumBytes-3;i+=3)
  {
    sumX += abs(vector[i] - oldX);
    sumY += abs(vector[i+1] - oldY);
    sumZ += abs(vector[i+2] - oldZ);
    oldX = vector[i];
    oldY = vector[i+1];
    oldZ = vector[i+2];
  }

  float meanX = sumX/(samplesPerVector-1);
  float meanY = sumY/(samplesPerVector-1);
  float meanZ = sumZ/(samplesPerVector-1);

  /*
  Serial.println(meanX);
  Serial.println(meanY);
  Serial.println(meanZ);
  */
  if (meanX==0 && meanY==0 && meanZ==0)
    return true;
  else
    return false;  
  
}





/**************************************************************************************************************
    Function : GetAverageSamples -> A moving average filter
    
    Input:  byte samples[] -> A byte array containing raw acclerometer samples
            unsigned int num -> Size of array samples
            unsigned int pos -> Current position at which filtering is to be done 
            unsigned int step -> The step size denoting the number of samples to be condensed into one
    
    Output: byte -> The value after applying filter and condensing. 
***************************************************************************************************************/
byte getAverageSample(byte samples[], unsigned int num, unsigned int pos, unsigned int step)
{
  unsigned int ret;
  unsigned int size = step*2;

  if(pos < (step * 3) || pos > (num * 3) - (step * 3))
  {
    ret = samples[pos];
  }
  else
  {
    ret=0;
    pos -= (step*3);
    for(unsigned int i=0; i<size; i++)
    {
      ret += samples[pos - (3*i)];
    }
    ret /= size;
  }

  return (byte)ret;
}





/**************************************************************************************************************
    Function : UnderSample-> Condense the raw samples from accelerometer to fit into vectorNumBytes samples and 
                             Store it in vector
    
    Input:  byte samples[] -> A byte array containing raw accelerometer readings
            int numSamples -> The number of raw samples read from acclerometer
            byte vector[] -> A byte vector of size vectorNumBytes which stores the condensed sample values
    
    Output: void -> 
***************************************************************************************************************/
void underSample(byte samples[], int numSamples, byte vector[])
{
  unsigned int vi = 0;
  unsigned int si = 0;
  unsigned int step = numSamples / samplesPerVector;
  unsigned int remainder = numSamples - (step * samplesPerVector);

  samples += (remainder / 2) * 3;

  for(unsigned int i =0; i<samplesPerVector; i++)
  {
    for(unsigned int j = 0; j<3; j++)
    {
      vector[vi+j] = getAverageSample(samples,numSamples, si+j, step);
    }

    si += (step * 3);
    vi += 3;
  }
  
}





/**************************************************************************************************************
    Function : -> ReadFromIMU -> Reads a set of Data either to train or detect one instance of a gesture.
    
    Input:   byte vector[]-> An array of byte of size vectorNumBytes 
    
    Output:  void-> 
***************************************************************************************************************/
void readFromIMU( byte vector[])
{
  byte accelerometerData[sensorBufferSize];
  int raw[3];

  unsigned int samples=0;
  unsigned int i=0;


  while(digitalRead(buttonPin)==LOW);

  while(digitalRead(buttonPin)==HIGH)
  {
    if(CurieIMU.accelDataReady())
    {
      CurieIMU.readAccelerometer(raw[0],raw[1],raw[2]);

      /*Map raw data to one byte value between 0-255*/
      accelerometerData[i] = (byte)map(raw[0],IMULow,IMUHigh,0,255);
      accelerometerData[i+1] = (byte)map(raw[1],IMULow,IMUHigh,0,255);
      accelerometerData[i+2] = (byte)map(raw[2],IMULow,IMUHigh,0,255);

      i+=3;
      ++samples;

      if((i+3) > sensorBufferSize)
         break;
         
    }
  }

  underSample(accelerometerData,samples,vector);
}





/**************************************************************************************************************
    Function : TrainGesture-> Train a particular gesture
    
    Input:   int n -> The category number of the gesture. 
    
    Output:  void -> 
***************************************************************************************************************/
void trainGesture(int n)
{
    for(unsigned int i=0;i<trainingReps;i++)
    {
      byte vector[vectorNumBytes];
      
      if(i)
        Serial.println("Again.....");
        readFromIMU(vector);
        CuriePME.learn(vector, vectorNumBytes,n);
        Serial.println("Done");
        delay(1000);
    }
  
}





/**************************************************************************************************************
    Function : TrainGestures-> Trains Gestures. 
                               The number of Gestures trained depends on variable numberOfGestures.
    
    Input:  void ->
     
    Output: void -> 
***************************************************************************************************************/
void trainGestures()
{
  for(unsigned int i=1; i<=numberOfGestures; i++)
  {
    Serial.print("Hold down the button and draw the Gesture '");
    Serial.println(getGestureName(i));

    trainGesture(i);
    Serial.println("Done with this Gesture");
    delay(2500);
  }
  
}





/**************************************************************************************************************
    Function : GetGestureName -> Function used to get the name of the gesture from its category number
    
    Input:  integer n -> This is the category of the Gesture
    
    Output: String -> The name of the Gesture
***************************************************************************************************************/
String getGestureName(unsigned int n)
{
  switch(n)
  {
    case 1: return "ScrollLeft";
    case 2: return "ScrollRight";
    case 3: return "ScrollUp";
    case 4: return "ScrollDown";
    case 5: return "Copy";
    case 6: return "Paste";
    case 7: return "VolumeUp";
    case 8: return "VolumeDown";
    case 9: return "Play";
    case 10: return "SwypeLeft";
    case 11: return "SwypeRight";
    case 12: return "ZoomIn";
    case 13: return "ZoomOut";  
    default: return "Not Identified";
  }
}





/**************************************************************************************************************
    Function : SaveNetworkKnowledge -> Save the current state of the neural network on a file called 
               NeurData.dat on flash memory
               
    Input:  void -> 
    
    Output: void -> 
***************************************************************************************************************/
void saveNetworkKnowledge ( void )
{
  const char *filename = "NeurData.dat";
  SerialFlashFile file;

  uint16_t savedState = CuriePME.beginSaveMode();
  Intel_PMT::neuronData neuronData;
  uint32_t fileSize = 128 * sizeof(neuronData);

  Serial.print( "File Size to save is = ");
  Serial.print( fileSize );
  Serial.print("\n");

  createIfNotExists( filename, fileSize );
  // Open the file and write test data
  file = SerialFlash.open(filename);
  file.erase();

  if (file) 
  {
    // iterate over the network and save the data.
    while( uint16_t nCount = CuriePME.iterateNeuronsToSave(neuronData)) 
    {
      if( nCount == 0x7FFF)
        break;

      Serial.print("Saving Neuron: ");
      Serial.print(nCount);
      Serial.print("\n");
      uint16_t neuronFields[4];

      neuronFields[0] = neuronData.context;
      neuronFields[1] = neuronData.influence;
      neuronFields[2] = neuronData.minInfluence;
      neuronFields[3] = neuronData.category;

      file.write( (void*) neuronFields, 8);
      file.write( (void*) neuronData.vector, 128 );
    }
  }

  CuriePME.endSaveMode(savedState);
  Serial.print("Knowledge Set Saved. \n");
}





/**************************************************************************************************************
    Function :  RestoreNetworkKnowledge-> Restore the neural network to a state as stored in  flash memory in a 
                file called NeurData.dat
                
    Input:  void -> 
    
    Output: void -> 
***************************************************************************************************************/
void restoreNetworkKnowledge ( void )
{
  const char *filename = "NeurData.dat";
  SerialFlashFile file;
  int32_t fileNeuronCount = 0;

  uint16_t savedState = CuriePME.beginRestoreMode();
  Intel_PMT::neuronData neuronData;

  // Open the file and write test data
  file = SerialFlash.open(filename);

  if (file) 
  {
    // iterate over the network and save the data.
    while(1) 
    {
      Serial.print("Reading Neuron: ");

      uint16_t neuronFields[4];
      file.read( (void*) neuronFields, 8);
      file.read( (void*) neuronData.vector, 128 );

      neuronData.context = neuronFields[0] ;
      neuronData.influence = neuronFields[1] ;
      neuronData.minInfluence = neuronFields[2] ;
      neuronData.category = neuronFields[3];

      if (neuronFields[0] == 0 || neuronFields[0] > 127)
        break;

      fileNeuronCount++;

      // this part just prints each neuron as it is restored,
      // so you can see what is happening.
      Serial.print(fileNeuronCount);
      Serial.print("\n");

      Serial.print( neuronFields[0] );
      Serial.print( "\t");
      Serial.print( neuronFields[1] );
      Serial.print( "\t");
      Serial.print( neuronFields[2] );
      Serial.print( "\t");
      Serial.print( neuronFields[3] );
      Serial.print( "\t");

      Serial.print( neuronData.vector[0] );
      Serial.print( "\t");
      Serial.print( neuronData.vector[1] );
      Serial.print( "\t");
      Serial.print( neuronData.vector[2] );

      Serial.print( "\n");
      CuriePME.iterateNeuronsToRestore( neuronData );
    }
  }

  CuriePME.endRestoreMode(savedState);
  Serial.print("Knowledge Set Restored. \n");
}





/**************************************************************************************************************
    Function : CreateIfNotExisits -> Check if a file exists. If it does not then create it in Flash Memory
    
    Input: const char *filename  -> The name of the file
           uint32_t fileSize  ->  The Size of the File to be created.
           
    Output: bool -> true if File exists or File successfully created. 
                    false if File creation failed. 
***************************************************************************************************************/
bool createIfNotExists (const char *filename, uint32_t fileSize) {
  if (!SerialFlash.exists(filename)) 
  {
    Serial.println("Creating file " + String(filename));
    return SerialFlash.createErasable(filename, fileSize);
  }

  Serial.println("File " + String(filename) + " already exists");
  return true;
}
