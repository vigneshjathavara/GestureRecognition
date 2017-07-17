#include <QueueArray.h>
#include "CurieIMU.h"

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



void setup() {
  Serial.begin(9600);
  while (!Serial);
  outputQueue.setPrinter(Serial);
  Serial.println(sizeof(String));
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

void loop() {
  CurieIMU.readAccelerometer(ax, ay, az);
  String s = IMUFrame(ax,ay,az);
  AddToQueue(s);
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
