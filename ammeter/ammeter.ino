

#include <ACS712.h>

/*

  This example shows how to measure DC current

*/



// We have 30 amps version sensor connected to A1 pin of arduino

// Replace with your version if necessary

ACS712 sensor(ACS712_20A, A5);

float  I=0.00;

void setup() {

  Serial.begin(9600);



  // This method calibrates zero point of sensor,

  // It is not necessary, but may positively affect the accuracy

  // Ensure that no current flows through the sensor at this moment

  sensor.calibrate();

}



void loop() {

  // Get current from sensor

  

  for(int i=0;i<500;i++)
  {
    I = I+( sensor.getCurrentDC());
  }
  I=I/500;
  //I=I-0.03;

  Serial.println(String("I = ") + I+ " A");

  

  // Wait one second before the new cycle

  delay(500);

}
