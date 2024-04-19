float value=0.00;
float voltage;
float current;
const float R1=99.9;
const float R2=19.83;
const float R1i=100;
const float R2i=20;

const float K=((R1+R2)/R2)/((R1i+R2i)/R2i);

const float AVCC=4.99;

void setup() 
{
  Serial.begin(9600);

}

void loop() 
{
  value=(float)analogRead(A4);
  voltage=value*(AVCC/1024)*((R1+R2)/R2)*K;
  Serial.println(voltage,2);
  delay(500);

}
