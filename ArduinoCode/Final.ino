#include <QueueArray.h>
#include "CurieIMU.h"
#include <Wire.h>
#include "Adafruit_DRV2605.h"

Adafruit_DRV2605 drv;



int ax, ay, az;
QueueArray<char> outputQueue;

void AddToQueue(String s)
{
  for(unsigned int i=0;i<s.length();i++)
  {
    outputQueue.enqueue(s.charAt(i));
  }
  
}

void PrintQueue()
{
  while(!outputQueue.isEmpty())
  {   
      Serial.print(outputQueue.dequeue());
  }
  
}

String ConvertIntToString(int num)
{
  String buf;
  if(num>=0)
    buf = "+";
  else
    buf = "-";  
  if(num<0)
    num += abs(num)*2;  
  if(num < 1000 && num > 99)
    buf+="0";
  else if (num < 100 && num > 9)
    buf+="00";
  else if (  num < 10)
    buf+= "000";
  else
     buf+="";

  buf += num;

  return buf;      
}

String IMUFrame(int x, int y, int z)
{
    String buf = ConvertIntToString(x) + "\t" + ConvertIntToString(y) + "\t" + ConvertIntToString(z) + "\n";
    return buf;
}


const int flexPin = A0; //pin A0 to read analog input
unsigned long time1;
bool clicks = false;
bool back = false;
bool hold = false;

int value; //save analog value


void setup() {
  Serial.begin(9600);
  outputQueue.setPrinter(Serial);
  drv.begin();
  
  drv.selectLibrary(1);
  
  // I2C trigger by sending 'go' command 
  // default, internal trigger when sending GO command
  drv.setMode(DRV2605_MODE_INTTRIG);
  while (!Serial);
  //Serial.println(sizeof(String));
  CurieIMU.begin();

  if (!CurieIMU.testConnection()) {
    Serial.println("CurieImu connection failed");
  }

   CurieIMU.attachInterrupt(eventCallback);

  CurieIMU.setAccelerometerRange(8);

  // Reduce threshold to allow detection of weaker taps (>= 750mg)
  CurieIMU.setDetectionThreshold(CURIE_IMU_TAP, 750); // (750mg)

  // Enable Double-Tap detection
  CurieIMU.interrupts(CURIE_IMU_TAP);
}

uint8_t effect = 1;

void loop() {
  CurieIMU.readAccelerometer(ax, ay, az);
  String s = IMUFrame(ax,ay,az);
  AddToQueue(s);


  
  value = analogRead(flexPin); //Read and save analog value from potentiometer
   //Serial.println(value);
  if(value>1000)
  {
    if(clicks == false && millis()-time1 >200)
    {
      clicks=true;
      time1 = millis();
    }

    else
    {
      if( back && millis()-time1<700)
      {
          clicks = false;
          back = false;
          //Serial.println("Double_Click"); 
          AddToQueue("DC000000000000000\n");
          time1 = millis();
          drv.setWaveform(0, effect);  // play effect 
          drv.setWaveform(1, 0);       // end waveform
          // play the effect!
          drv.go();
      }

      if( !back && millis()-time1>700 && hold==false)
      {
          //clicks = false;
          hold=true;
          //Serial.println("Click_Hold");  
          AddToQueue("CH000000000000000\n");           
      }
    }
  }

  if(clicks && value <940 && hold==false)
  {
      back =true;
  }



  if(clicks && millis()-time1>700 && back )    
      {
        clicks=false;
        back = false;
        //Serial.println("Single_Click");
        AddToQueue("SC000000000000000\n");
        drv.setWaveform(0, effect);  // play effect 
        drv.setWaveform(1, 0);       // end waveform

        // play the effect!
        drv.go();
      }

  if(hold == true && value<940)   
      {
          hold = false;
          clicks= false;
          //Serial.println("Hold_Release");
          AddToQueue("HR000000000000000\n");
      }



      
  PrintQueue();
  /*while(!outputQueue.isEmpty())
  {
      String outputString = outputQueue.dequeue();    
      Serial.println(outputString);
  }*/
  /*
  Serial.print(ax);
  Serial.print("\t");
  Serial.print(ay);
  Serial.print("\t");
  Serial.print(az);
  Serial.println();
  */
  delay(10);
}


static void eventCallback()
{
  if (CurieIMU.getInterruptStatus(CURIE_IMU_TAP)) {
      String s ="Tap00000000000000\n";
      for(unsigned int i=0;i<s.length();i++)
      {
        outputQueue.enqueue(s.charAt(i));
      }
  }
}
                                      
