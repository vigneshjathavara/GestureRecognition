#include "CurieIMU.h"

int ax, ay, az;



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

  CurieIMU.begin();

  if (!CurieIMU.testConnection()) {
    Serial.println("CurieImu connection failed");
  }

  CurieIMU.setAccelerometerRange(8);
}

void loop() {
  CurieIMU.readAccelerometer(ax, ay, az);
  String s = IMUFrame(ax,ay,az);
  Serial.print(s);
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
