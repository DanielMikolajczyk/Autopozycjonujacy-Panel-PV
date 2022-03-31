#include <Servo.h>

//Global variables

#define TRIG 15
#define INPUT_VOLTAGE 5
#define S_MAX_POS_X 170
#define S_MAX_POS_Y 125
#define S_MIN_POS_X 10
#define S_MIN_POS_Y 30

#define FIX0  -19//-24
#define FIX1  -33//-42
#define FIX2  6//37
#define FIX3  54//-22

int ADCresolution = pow(2, 10);                          // 1024 bits
const float voltagePlot = INPUT_VOLTAGE / ADCresolution; // 5/1024 = 0,0048828125
int32_t XValue, YValue;                                    // additive X-Y voltage value from resistors
unsigned int delayTime = 100;

/* USER VARIABLES */
int samples = 20;                                       // Numbers of samples taken for every resistor, before averaging

int32_t mesValues[4];                                      //Measured values
float actualValue = 0;                                   //Sum of voltages in actual position (for 180 jump)
float previousValue = 0;                                 //Sum of voltagges in previous position (for 180 jump)

//Initializing pinout for servos
Servo ServoX;           // X axis (5-175)
Servo ServoY;           // Y axis (5-150)
int posX = 90, posY = 90;

/* SETTING UP */
void setup() {

  Serial.begin(57600);
  pinMode(A0, INPUT);       // sets the analog pin A0 as input (photoresistor)
  pinMode(A1, INPUT);       // sets the analog pin A1 as input (photoresistor)
  pinMode(A2, INPUT);       // sets the analog pin A2 as input (photoresistor)
  pinMode(A3, INPUT);       // sets the analog pin A3 as input (photoresistor)

  pinMode(A5, INPUT);       // sets the analog pin A5 as input (potentiometer)

  ServoX.attach(9);         // attaches the servo on pin 9 to the servo object
  ServoY.attach(10);        // attaches the servo on pin 10 to the servo object

  ServoX.write(posX);       // set X axis servo to deafault position
  delay(100);
  ServoY.write(posY);       // set X axis servo to deafault position
  delay(1000);
}

/* MAIN LOOP */
void loop() {
  //----------------------------
  //Read value from ADC potentiometer to set delay times
  delayTime = analogRead(A5);
  if (delayTime < 100) {
    delayTime = 100;
  }

  //----------------------------
  //Read values from ADC's photoresistors
  mesValues[0] = 0;
  mesValues[1] = 0;
  mesValues[2] = 0;
  mesValues[3] = 0;
  for (int i = 0; i < samples; i++) {
    mesValues[0] += analogRead(A0);
    mesValues[1] += analogRead(A1);
    mesValues[2] += analogRead(A2);
    mesValues[3] += analogRead(A3);
    delay(2);
  }

  //----------------------------
  //Averaging
  for (int i = 0; i < 4; i++) {
    mesValues[i] /= samples;
  }
  mesValues[0] += FIX0;
  mesValues[1] += FIX1;
  mesValues[2] += FIX2;
  mesValues[3] += FIX3;

  //----------------------------
  //Calculate X-Y values
  //XValue = mesValues[0] - mesValues[1];
  //YValue = mesValues[3] - mesValues[1];
  XValue = mesValues[0] + mesValues[2] - mesValues[1] - mesValues[3];
  //XValue = mesValues[3] + mesValues[2] - mesValues[1] - mesValues[0];
  YValue = mesValues[2] + mesValues[3] - mesValues[0] - mesValues[1];
  //YValue = mesValues[2] + mesValues[0] - mesValues[3] - mesValues[1];
  Serial.println("______WARTOSCI_________");
  Serial.println(XValue);
  Serial.println(YValue);
  Serial.println("______POZYCJA_________");
  Serial.println(posX);
  Serial.println(posY);
  Serial.println("______POSZCZEGOLNE_________");
  Serial.println(mesValues[0]);
  Serial.println(mesValues[1]);
  Serial.println(mesValues[2]);
  Serial.println(mesValues[3]);
  //      Serial.println(analogRead(A5));
  // Serial.println(" ");

  //-----------------------------
  //Rotate PV
  if (abs(XValue) > TRIG) {

    if (XValue < 0 && posX < S_MAX_POS_X) {
      ServoX.write(++posX);
      //Serial.println("lewo");
    } else if (XValue > 0 && posX > S_MIN_POS_X) {
      ServoX.write(--posX);
      //Serial.println("prawo");
    }
    delay(15);
  }
  if (abs(YValue) > TRIG) {
    if (YValue > 0 && posY > S_MIN_POS_Y ) {
      ServoY.write(--posY);
      //Serial.println("gora");
    } else if (YValue < 0 && posY < S_MAX_POS_Y) {
      ServoY.write(++posY);
      //Serial.println("dol");
    }
    delay(15);
  }
}

/*
  //Rotate PV with 180 degrees jump for X axis
  //TODO: allow to jump every 1000? rotations otherwise prevent from continuous jumping
  //Jump will take place if sum of Voltages of every photoresistor is greater than it was before
  if(abs(XValue) > 0.2){
    if(XValue > 0 && posX < S_MAX_POS_X){
      ServoX.write(++posX);
    }else if(XValue < 0 && posX > S_MIN_POS_X){
      ServoX.write(--posX);
    }
    //Jump from MAX to MIN ----------------- Jumping conditions
    else if(XValue > 0 && posX == S_MAX_POS_X){
      // read actual values from resistors
      posX = S_MIN_POS_X;
      ServoX.write(posX);
      delay(1000);
      previousValue =  mesValues[0] +
                       mesValues[1] +
                       mesValues[2] +
                       mesValues[3];
      actualValue   =  analogRead(A0)*voltagePlot +
                       analogRead(A1)*voltagePlot +
                       analogRead(A2)*voltagePlot +
                       analogRead(A3)*voltagePlot;
      delay(1000);
      if(actualValue <= previousValue){
        posX = S_MAX_POS_X;
        ServoX.write(posX);
      }
      //------------------------------------ Jump end
      //Jump from MIN to MAX
      else if(XValue < 0 && posX == S_MIN_POS_X){
      posX = S_MAX_POS_X;
      ServoX.write(posX);
      delay(1000);
      previousValue =  mesValues[0] +
                       mesValues[1] +
                       mesValues[2] +
                       mesValues[3];
      actualValue   =  analogRead(A0)*voltagePlot +
                       analogRead(A1)*voltagePlot +
                       analogRead(A2)*voltagePlot +
                       analogRead(A3)*voltagePlot;
      delay(1000);
      if(actualValue <= previousValue){
        posX = S_MIN_POS_X;
        ServoX.write(posX);
      }
    }
    delay(delayTime);
  }
*/
