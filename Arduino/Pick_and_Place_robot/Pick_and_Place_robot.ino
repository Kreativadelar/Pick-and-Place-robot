#include <AccelStepper.h>
#include <Makeblock.h>
#include <SoftwareSerial.h>
#include <Wire.h>

//Me_LimitSwitch module can only be connected to PORT_3, PORT_4, PORT_6,PORT_7,PORT_8 of base shield or from PORT_3 to PORT_8 of baseboard.
MeLimitSwitch limitSwitch1(PORT_4, SLOT2); 
MeLimitSwitch limitSwitch2(PORT_4, SLOT1); 
MeLineFollower lineFinder(PORT_6);
MeLightSensor lightSensor(PORT_3);
MeDCMotor vaccum(M1);
MeDCMotor valve(M2);

//Rotating axel
int dir_rot_Pin = mePort[PORT_2].s1;//the direction pin connect to Base Board PORT1 SLOT1
int stp_rot_Pin = mePort[PORT_2].s2;//the Step pin connect to Base Board PORT1 SLOT2
AccelStepper rotStepper(AccelStepper::DRIVER,stp_rot_Pin,dir_rot_Pin); 

//Rotating axel
int dir_arm_Pin = mePort[PORT_1].s1;//the direction pin connect to Base Board PORT1 SLOT1
int stp_arm_Pin = mePort[PORT_1].s2;//the Step pin connect to Base Board PORT1 SLOT2
AccelStepper armStepper(AccelStepper::DRIVER,stp_arm_Pin,dir_arm_Pin); 

void setup()
{
    Serial.begin(9600);
    Serial.println("Start.");
    rotStepper.setMaxSpeed(10000);
    rotStepper.setCurrentPosition(0);
    armStepper.setMaxSpeed(10000);
    armStepper.setCurrentPosition(0);
}
void loop()
{
   static int seq = 0;
   static bool once = false;
   int sensorState = lineFinder.readSensors();
   int color = 0;
   int count = 0;
   const int limit = 2;

   Serial.print("Sensor: ");
   Serial.println(sensorState);
   
  if(!once){
    syncingAllAxis();
  }

  if(once){
    // Go to homepose
    Serial.println("Going to home position"); 
    allAxisToPosition(-2350, 800, 2500, 1000);

    for(count = 0; count >= limit; count++){
      //Pick new part
      Serial.println("Picking up new part");
      suction(true);

      //Move to pick up sensor
      Serial.println("Move to pick up sensor");
      
      //Check that part i suction cup
      Serial.println("Check that we got a part");
      sensorState = lineFinder.readSensors();
      if(sensorState>0){
        count = 0;
        break;
      }
    }

    // check for error
    if(count > 0){
      bool showError = false;
      while(true){
        if(!showError){
          Serial.println("------------ ERROR ------------");
          Serial.println("Error no parts in storage!");
          Serial.println("-------------------------------");
          showError = true;
        }
      }
      delay(200);
    }
    
    //Move to color sensor
    Serial.println("Going to color sensor");

    //Check color
    Serial.println("Checking color");
    color = checkColor();
    
    //Drop part at right color
    if(color > 25 && color < 55){
      // Yellow part
      Serial.println("Placing yellow part");
      suction(false);
    }else if(color > 120 && color < 145){
      // red part
      Serial.println("Placing red part");
      suction(false);
    }else{
      // bad part
      Serial.println("----------- WARNING -----------");
      Serial.println("Placing bad part!");
      Serial.println("-------------------------------");
      suction(false);
    }

    /*
    allAxisToPosition(-2350, 2700, 2500, 1000);
    suction(true);
    allAxisToPosition(-2350, 800, 2500, 1000);
    allAxisToPosition(-4400, 800, 2500, 1000);
    allAxisToPosition(-4400, 1870, 2500, 1000);
    suction(false);
    allAxisToPosition(-4400, 800, 2500, 1000);
    allAxisToPosition(-2350, 800, 2500, 1000);
    */
  }
  

  //255
  delay(3000);

  once = true;
}

void allAxisToPosition(long _position1, long _position2, float _speed, float _acc){
  rotStepper.setSpeed(_speed);
  armStepper.setSpeed(_speed);
  rotStepper.setAcceleration(_acc);
  armStepper.setAcceleration(_acc);

  rotStepper.moveTo(_position1);
  armStepper.moveTo(_position2);
    
  while (rotStepper.currentPosition() != _position1 || armStepper.currentPosition() != _position2){ // Full speed basck to 0
    
    if(rotStepper.currentPosition() != _position1)
      rotStepper.run();
      
    if(armStepper.currentPosition() != _position2)
      armStepper.run();
  }

}

void syncingAllAxis(){
  syncingAxis(2);
  syncingAxis(1);
}

void syncingAxis(int axis){
  rotStepper.setSpeed(500);
  rotStepper.setAcceleration(100);
  armStepper.setSpeed(500);
  armStepper.setAcceleration(100);

  
  if(axis == 1){
    Serial.println("----------- WARNING -----------");
    Serial.println("Syncing axis 1");
    Serial.println("-------------------------------");
    rotStepper.moveTo(10000);
  
    while (rotStepper.currentPosition() != 10000 && !limitSwitch1.touched()){ // Full speed basck to 0
      rotStepper.run();
    }
    rotStepper.stop();
    rotStepper.setCurrentPosition(0);
    delay(10);
    rotStepper.moveTo(-100);
    
    while (rotStepper.currentPosition() != -100){ // Full speed basck to 0
      rotStepper.run();
    }

    Serial.println("Syncing for axis 1 is done");
  }else if(axis == 2){
    Serial.println("----------- WARNING -----------");
    Serial.println("Syncing axis 2");
    Serial.println("-------------------------------");
    armStepper.moveTo(-10000);
  
    while (armStepper.currentPosition() != -10000 && !limitSwitch2.touched()){ // Full speed basck to 0
      armStepper.run();
    }
    armStepper.stop();
    armStepper.setCurrentPosition(0);
    delay(10);
    armStepper.moveTo(100);
    
    while (armStepper.currentPosition() != 100){ // Full speed basck to 0
      armStepper.run();
    }
    
    Serial.println("Syncing for axis 2 is done");
  }
}

void suction(bool state){

  delay(100);
  
  if(state){
    valve.stop();
    vaccum.run(255);
    delay(500);
    
  }else{
    vaccum.stop();
    valve.run(255);
    delay(500);
    valve.stop();
  }
}

int checkColor(){
  int value = lightSensor.read();  

  // print the results to the serial monitor:
  Serial.print("value = " );                                            
  Serial.println(value); 

  return value;
}





