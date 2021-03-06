/*
This work is public domain.

Please note: Although there are a LOT pin settings here.
You can get by with as few as TWO pins per Axis. (dir & step)
ie: 3 axies = 6 pins used. (minimum)
    9 axies = 18 pins or an entire UNO (using virtual limits switches only)

Note concerning switches: Be smart!
  AT LEAST use HOME switches.
  Switches are cheap insurance.
  You'll find life a lot easier if you use them entirely.
  
  If you choose to build with threaded rod for lead screws but leave out the switches
  You'll have one of two possible outcomes;
    You'll get tired really quickly of resetting the machine by hand.
    Or worse, you'll forget (only once) to reset it, and upon homing
    it WILL destroy itself while you go -> WTF!? -> OMG! -> PANIC! -> FACEPALM!

 List of axies. All 9 of them.
     AXIS_0 = X (Left/Right)
     AXIS_1 = Y (Near/Far) Lathes use this for tool depth.
     AXIS_2 = Z (Up/Down) Not typically used for lathes. Except lathe/mill combo.
     AXIS_3 = A (Rotation parallel to X axis) lathe chuck.
     AXIS_4 = B (Rotation parallel to Y axis)
     AXIS_5 = C (Rotation parallel to Z axis)
     AXIS_6 = U (Rotation perpendicular to X axis)
     AXIS_7 = V (Rotation perpendicular to Y axis)
     AXIS_8 = W (Rotation perpendicular to Z axis)

  DYI robot builders: You can monitor/control this sketch via a serial interface.
  Example commands:
  
    jog x200;
    jog x-215.25 y1200 z0.002 a5;
    
    PS: If you choose to control this with your own interface then also modify the
    divisor variable further down.
*/

#include <Arduino.h>
#include <HardwareSerial.h>

// You'll need this library. Get the interrupt safe version.
#include "digitalWriteFast/digitalWriteFast.h" // http://code.google.com/p/digitalwritefast/

#define BAUD    (115200)

// These will be used in the near future.
#define VERSION "00072"      // 5 caracters
#define ROLE    "ALL-IN-ONE" // 10 characters

#define stepsPerInchX 3200
#define stepsPerInchY 3200
#define stepsPerInchZ 3200
#define stepsPerInchA 3200
#define stepsPerInchB 3200
#define stepsPerInchC 3200
#define stepsPerInchU 3200
#define stepsPerInchV 3200
#define stepsPerInchW 3200

#define minStepTime 512 //delay in MICROseconds between step pulses.

// step pins (required)
#define stepPin0 62
#define stepPin1 63
#define stepPin2 64
#define stepPin3 67
#define stepPin4 68
#define stepPin5 69
#define stepPin6 66
#define stepPin7 65
#define stepPin8 -1

// dir pins (required)
#define dirPin0 37
#define dirPin1 39
#define dirPin2 41
#define dirPin3 50
#define dirPin4 51
#define dirPin5 53
#define dirPin6 45
#define dirPin7 43
#define dirPin8 -1

// microStepping pins (optional)
#define chanXms1 -1
#define chanXms2 -1
#define chanXms3 -1
#define chanYms1 -1
#define chanYms2 -1
#define chanYms3 -1
#define chanZms1 -1
#define chanZms2 -1
#define chanZms3 -1
#define chanAms1 -1
#define chanAms2 -1
#define chanAms3 -1
#define chanBms1 -1
#define chanBms2 -1
#define chanBms3 -1
#define chanCms1 -1
#define chanCms2 -1
#define chanCms3 -1
#define chanUms1 -1
#define chanUms2 -1
#define chanUms3 -1
#define chanVms1 -1
#define chanVms2 -1
#define chanVms3 -1
#define chanWms1 -1
#define chanWms2 -1
#define chanWms3 -1

#define xEnablePin 46
#define yEnablePin 46
#define zEnablePin 46
#define aEnablePin 46
#define bEnablePin 46
#define cEnablePin 46
#define uEnablePin 46
#define vEnablePin 46
#define wEnablePin -1

#define useEstopSwitch  true
#define usePowerSwitch  true
#define useProbe        true
#define useStartSwitch  true
#define useStopSwitch   true
#define usePauseSwitch  true
#define useResumeSwitch true
#define useStepSwitch   true

// Set to true if your using real switches for MIN positions.
#define useRealMinX false
#define useRealMinY false
#define useRealMinZ false
#define useRealMinA false
#define useRealMinB false
#define useRealMinC false
#define useRealMinU false
#define useRealMinV false
#define useRealMinW false

// Set to true if your using real switches for HOME positions.
#define useRealHomeX true
#define useRealHomeY true
#define useRealHomeZ true
#define useRealHomeA true
#define useRealHomeB true
#define useRealHomeC true
#define useRealHomeU true
#define useRealHomeV true
#define useRealHomeW false

// Set to false if your using real switches for MAX positions.
#define useRealMaxX false
#define useRealMaxY false
#define useRealMaxZ false
#define useRealMaxA false
#define useRealMaxB false
#define useRealMaxC false
#define useRealMaxU false
#define useRealMaxV false
#define useRealMaxW false

// If your using REAL switches you'll need real pins (ignored if using Virtual switches).
// -1 = not used.
#define xMinPin -1
#define yMinPin -1
#define zMinPin -1
#define aMinPin -1
#define bMinPin -1
#define cMinPin -1
#define uMinPin -1
#define vMinPin -1
#define wMinPin -1

#define xHomePin 47
#define yHomePin 40
#define zHomePin 36
#define aHomePin 52
#define bHomePin 42
#define cHomePin 38
  //TAUX
#define uHomePin 56
  //TE3
#define vHomePin 57
#define wHomePin -1

#define xMaxPin -1
#define yMaxPin -1
#define zMaxPin -1
#define aMaxPin -1
#define bMaxPin -1
#define cMaxPin -1
#define uMaxPin -1
#define vMaxPin -1
#define wMaxPin -1

#define powerSwitchIsMomentary true // Set to true if your using a momentary switch.
#define powerPin    27 // Power switch. Optional
#define powerLedPin -1 // Power indicator. Optional

#define eStopPin         26 // E-Stop switch. You really, REALLY should have this one.
#define eStopLedPin      -1 // E-Stop indicator. Optional

#define probePin  48 // CNC Touch probe input.     Optional
#define startPin  30 // CNC Program start switch.  Optional
#define stopPin  34 // CNC Stop program switch.   Optional
#define pausePin  32 // CNC Pause program switch.  Optional
#define resumePin 33 // CNC Resume program switch. Optional
#define stepPin   31 // CNC Program step switch.   Optional

// Spindle pin config
#define spindleEnablePin         5 // Optional
#define spindleEnableInverted    true // Set to true if you need +5v to activate.
#define spindleDirection         2 // Optional
#define spindleDirectionInverted true // Set to true if spindle runs in reverse.

#define spindleTach      18 // Must be an interrupt pin. Optional.
                            // UNO can use pin 2 or 3.
                            // Mega2560 can use 2,3,18,19,20 or 21.

#define coolantMistPin   3 // Controls coolant mist pump.   Optional
#define coolantFloodPin  6 // Controls coolant flood pump.  Optional
#define powerSupplyPin   46 // Controls power supply ON/OFF. (ENABLE STEPPER DRIVERS)
#define powerSupplyInverted true // Set to "true" for +5v = ON

//basic safe start (avoid pin 7 and 8), if wrong firmware loads onto 3d printer
#define pin_HE0_force_low 7
#define pin_HE1_force_low 6
#define pin_HE2_force_low 3
#define pin_HE3_force_low 2
#define pin_HEAux_force_low 5
#define pin_HEBed_force_low 8



// Signal inversion for real switch users. (false = ground trigger signal, true = +5vdc trigger signal.)
// Note: Inverted switches will need pull-down resistors (less than 10kOhm) to lightly ground the signal wires.
#define xMinPinInverted false
#define yMinPinInverted false
#define zMinPinInverted false
#define aMinPinInverted false
#define bMinPinInverted false
#define cMinPinInverted false
#define uMinPinInverted false
#define vMinPinInverted false
#define wMinPinInverted false

#define xHomePinInverted false
#define yHomePinInverted false
#define zHomePinInverted false
#define aHomePinInverted false
#define bHomePinInverted false
#define cHomePinInverted false
#define uHomePinInverted false
#define vHomePinInverted false
#define wHomePinInverted false

#define xMaxPinInverted false
#define yMaxPinInverted false
#define zMaxPinInverted false
#define aMaxPinInverted false
#define bMaxPinInverted false
#define cMaxPinInverted false
#define uMaxPinInverted false
#define vMaxPinInverted false
#define wMaxPinInverted false

#define eStopPinInverted  true
#define powerPinInverted  false
#define probePinInverted  false
#define startPinInverted  false
#define stopPinInverted   false
#define pausePinInverted  false
#define resumePinInverted false
#define stepPinInverted   false

// Where should the VIRTUAL Min switches be set to (ignored if using real switches).
// Set to whatever you specified in the StepConf wizard.
#define xMin -9999.9
#define yMin -9999.9
#define zMin -9999.9
#define aMin -9999.9
#define bMin -9999.9
#define cMin -9999.9
#define uMin -9999.9
#define vMin -9999.9
#define wMin -9999.9

// Where should the VIRTUAL home switches be set to (ignored if using real switches).
// Set to whatever you specified in the StepConf wizard.
#define xHome 0
#define yHome 0
#define zHome 0
#define aHome 0
#define bHome 0
#define cHome 0
#define uHome 0
#define vHome 0
#define wHome 0

// Where should the VIRTUAL Max switches be set to (ignored if using real switches).
// Set to whatever you specified in the StepConf wizard.
#define xMax 9999.9
#define yMax 9999.9
#define zMax 9999.9
#define aMax 9999.9
#define bMax 9999.9
#define cMax 9999.9
#define uMax 9999.9
#define vMax 9999.9
#define wMax 9999.9

#define giveFeedBackX false
#define giveFeedBackY false
#define giveFeedBackZ false
#define giveFeedBackA false
#define giveFeedBackB false
#define giveFeedBackC false
#define giveFeedBackU false
#define giveFeedBackV false
#define giveFeedBackW false

/*
  This indicator led will let you know how hard you pushing the Arduino.

  To test: Issue a G0 in the GUI command to send all axies to near min limits then to near max limits.
  Watch the indicator led as you do this. Adjust "Max Velocity" setting to suit.

  MOSTLY ON  = You can safely go faster.
  FREQUENT BLINK = This is a safe speed. The best choice.
  OCCASIONAL BLINK = Your a speed demon. Pushing it to the limits.
  OFF COMPLETELY = Pushing it too hard. Slow down! The Arduino can't cope, your CNC will break bits and make garbage.
  
*/
#define idleIndicator 4

// Invert direction of movement for an axis by setting to false.
boolean dirState0=true;
boolean dirState1=true;
boolean dirState2=true;
boolean dirState3=true;
boolean dirState4=true;
boolean dirState5=true;
boolean dirState6=true;
boolean dirState7=true;
boolean dirState8=true;

////////////////////////////////////////////////////////////////////////////////
/////////////////////////////END OF USER SETTINGS///////////////////////////////

//void countSpindleRevs();
void jog(float x, float y, float z, float a, float b, float c, float u, float v, float w);

////////////////////////////////////////////////////////////////////////////////

char buffer[128];
int sofar;
 
float pos_x;
float pos_y;
float pos_z;
float pos_a;
float pos_b;
float pos_c;
float pos_u;
float pos_v;
float pos_w;

float revs_in=0;
float spindleRPSin=0;

boolean stepState=LOW;
unsigned long stepTimeOld=0;
unsigned long spindleTimeOld=0;
long stepper0Pos=0;
long stepper0Goto=0;
long stepper1Pos=0;
long stepper1Goto=0;
long stepper2Pos=0;
long stepper2Goto=0;
long stepper3Pos=0;
long stepper3Goto=0;
long stepper4Pos=0;
long stepper4Goto=0;
long stepper5Pos=0;
long stepper5Goto=0;
long stepper6Pos=0;
long stepper6Goto=0;
long stepper7Pos=0;
long stepper7Goto=0;
long stepper8Pos=0;
long stepper8Goto=0;
int stepModeX=-1; // don't set these here, look at stepMode() for info.
int stepModeY=-1;
int stepModeZ=-1;
int stepModeA=-1;
int stepModeB=-1;
int stepModeC=-1;
int stepModeU=-1;
int stepModeV=-1;
int stepModeW=-1;

boolean xMinState=false;
boolean yMinState=false;
boolean zMinState=false;
boolean aMinState=false;
boolean bMinState=false;
boolean cMinState=false;
boolean uMinState=false;
boolean vMinState=false;
boolean wMinState=false;
boolean xMinStateOld=false;
boolean yMinStateOld=false;
boolean zMinStateOld=false;
boolean aMinStateOld=false;
boolean bMinStateOld=false;
boolean cMinStateOld=false;
boolean uMinStateOld=false;
boolean vMinStateOld=false;
boolean wMinStateOld=false;

boolean xHomeState=false;
boolean yHomeState=false;
boolean zHomeState=false;
boolean aHomeState=false;
boolean bHomeState=false;
boolean cHomeState=false;
boolean uHomeState=false;
boolean vHomeState=false;
boolean wHomeState=false;
boolean xHomeStateOld=false;
boolean yHomeStateOld=false;
boolean zHomeStateOld=false;
boolean aHomeStateOld=false;
boolean bHomeStateOld=false;
boolean cHomeStateOld=false;
boolean uHomeStateOld=false;
boolean vHomeStateOld=false;
boolean wHomeStateOld=false;

boolean xMaxState=false;
boolean yMaxState=false;
boolean zMaxState=false;
boolean aMaxState=false;
boolean bMaxState=false;
boolean cMaxState=false;
boolean uMaxState=false;
boolean vMaxState=false;
boolean wMaxState=false;
boolean xMaxStateOld=false;
boolean yMaxStateOld=false;
boolean zMaxStateOld=false;
boolean aMaxStateOld=false;
boolean bMaxStateOld=false;
boolean cMaxStateOld=false;
boolean uMaxStateOld=false;
boolean vMaxStateOld=false;
boolean wMaxStateOld=false;

boolean eStopStateOld=false;
boolean powerStateOld=false;
boolean probeStateOld=false;
boolean startStateOld=false;
boolean stopStateOld=false;
boolean pauseStateOld=false;
boolean resumeStateOld=false;
boolean stepStateOld=false;
int globalBusy=0;

long divisor=1000000; // input divisor. Our HAL script wont send the six decimal place floats that EMC cranks out.
                      // A simple workaround is to multply it by 1000000 before sending it over the wire.
                      // So here we have to put the decimal back to get the real numbers.
                      // Used in: processCommand()

boolean psuState=powerSupplyInverted;
boolean spindleState=spindleDirectionInverted;

float fbx=1;
float fby=1;
float fbz=1;
float fba=1;
float fbb=1;
float fbc=1;
float fbu=1;
float fbv=1;
float fbw=1;

float fbxOld=0;
float fbyOld=0;
float fbzOld=0;
float fbaOld=0;
float fbbOld=0;
float fbcOld=0;
float fbuOld=0;
float fbvOld=0;
float fbwOld=0;


void jog(float x, float y, float z, float a, float b, float c, float u, float v, float w)
{
  pos_x=x;
  pos_y=y;
  pos_z=z;
  pos_a=a;
  pos_b=b;
  pos_c=c;
  pos_u=u;
  pos_v=v;
  pos_w=w;
  // Handle our limit switches.
    // Compressed to save visual space. Otherwise it would be several pages long!
    
  if(!useRealMinX){if(pos_x > xMin){xMinState=true;}else{xMinState=false;}}else{xMinState=digitalReadFast(xMinPin);if(xMinPinInverted)xMinState=!xMinState;}
  if(!useRealMinY){if(pos_y > yMin){yMinState=true;}else{yMinState=false;}}else{yMinState=digitalReadFast(yMinPin);if(yMinPinInverted)yMinState=!yMinState;}
  if(!useRealMinZ){if(pos_z > zMin){zMinState=true;}else{zMinState=false;}}else{zMinState=digitalReadFast(zMinPin);if(zMinPinInverted)zMinState=!zMinState;}
  if(!useRealMinA){if(pos_a > aMin){aMinState=true;}else{aMinState=false;}}else{aMinState=digitalReadFast(aMinPin);if(aMinPinInverted)aMinState=!aMinState;}
  if(!useRealMinB){if(pos_b > bMin){bMinState=true;}else{bMinState=false;}}else{bMinState=digitalReadFast(bMinPin);if(bMinPinInverted)bMinState=!bMinState;}
  if(!useRealMinC){if(pos_c > cMin){cMinState=true;}else{cMinState=false;}}else{cMinState=digitalReadFast(cMinPin);if(cMinPinInverted)cMinState=!cMinState;}
  if(!useRealMinU){if(pos_u > uMin){uMinState=true;}else{uMinState=false;}}else{uMinState=digitalReadFast(uMinPin);if(uMinPinInverted)uMinState=!uMinState;}
  if(!useRealMinV){if(pos_v > vMin){vMinState=true;}else{vMinState=false;}}else{vMinState=digitalReadFast(vMinPin);if(vMinPinInverted)vMinState=!vMinState;}
  if(!useRealMinW){if(pos_w > wMin){wMinState=true;}else{wMinState=false;}}else{wMinState=digitalReadFast(wMinPin);if(wMinPinInverted)wMinState=!wMinState;}

  if(!useRealMaxX){if(pos_x > xMax){xMaxState=true;}else{xMaxState=false;}}else{xMaxState=digitalReadFast(xMaxPin);if(xMaxPinInverted)xMaxState=!xMaxState;}
  if(!useRealMaxY){if(pos_y > yMax){yMaxState=true;}else{yMaxState=false;}}else{yMaxState=digitalReadFast(yMaxPin);if(yMaxPinInverted)yMaxState=!yMaxState;}
  if(!useRealMaxZ){if(pos_z > zMax){zMaxState=true;}else{zMaxState=false;}}else{zMaxState=digitalReadFast(zMaxPin);if(zMaxPinInverted)zMaxState=!zMaxState;}
  if(!useRealMaxA){if(pos_a > aMax){aMaxState=true;}else{aMaxState=false;}}else{aMaxState=digitalReadFast(aMaxPin);if(aMaxPinInverted)aMaxState=!aMaxState;}
  if(!useRealMaxB){if(pos_b > bMax){bMaxState=true;}else{bMaxState=false;}}else{bMaxState=digitalReadFast(bMaxPin);if(bMaxPinInverted)bMaxState=!bMaxState;}
  if(!useRealMaxC){if(pos_c > cMax){cMaxState=true;}else{cMaxState=false;}}else{cMaxState=digitalReadFast(cMaxPin);if(cMaxPinInverted)cMaxState=!cMaxState;}
  if(!useRealMaxU){if(pos_u > uMax){uMaxState=true;}else{uMaxState=false;}}else{uMaxState=digitalReadFast(uMaxPin);if(uMaxPinInverted)uMaxState=!uMaxState;}
  if(!useRealMaxV){if(pos_v > vMax){vMaxState=true;}else{vMaxState=false;}}else{vMaxState=digitalReadFast(vMaxPin);if(vMaxPinInverted)vMaxState=!vMaxState;}
  if(!useRealMaxW){if(pos_w > wMax){wMaxState=true;}else{wMaxState=false;}}else{wMaxState=digitalReadFast(wMaxPin);if(wMaxPinInverted)wMaxState=!wMaxState;}

  if(!useRealHomeX){if(pos_x > xHome){xHomeState=true;}else{xHomeState=false;}}else{xHomeState=digitalReadFast(xHomePin);if(xHomePinInverted)xHomeState=!xHomeState;}
  if(!useRealHomeY){if(pos_y > yHome){yHomeState=true;}else{yHomeState=false;}}else{yHomeState=digitalReadFast(yHomePin);if(yHomePinInverted)yHomeState=!yHomeState;}
  if(!useRealHomeZ){if(pos_z > zHome){zHomeState=true;}else{zHomeState=false;}}else{zHomeState=digitalReadFast(zHomePin);if(zHomePinInverted)zHomeState=!zHomeState;}
  if(!useRealHomeA){if(pos_a > aHome){aHomeState=true;}else{aHomeState=false;}}else{aHomeState=digitalReadFast(aHomePin);if(aHomePinInverted)aHomeState=!aHomeState;}
  if(!useRealHomeB){if(pos_b > bHome){bHomeState=true;}else{bHomeState=false;}}else{bHomeState=digitalReadFast(bHomePin);if(bHomePinInverted)bHomeState=!bHomeState;}
  if(!useRealHomeC){if(pos_c > cHome){cHomeState=true;}else{cHomeState=false;}}else{cHomeState=digitalReadFast(cHomePin);if(cHomePinInverted)cHomeState=!cHomeState;}
  if(!useRealHomeU){if(pos_u > uHome){uHomeState=true;}else{uHomeState=false;}}else{uHomeState=digitalReadFast(uHomePin);if(uHomePinInverted)uHomeState=!uHomeState;}
  if(!useRealHomeV){if(pos_v > vHome){vHomeState=true;}else{vHomeState=false;}}else{vHomeState=digitalReadFast(vHomePin);if(vHomePinInverted)vHomeState=!vHomeState;}
  if(!useRealHomeW){if(pos_w > wHome){wHomeState=true;}else{wHomeState=false;}}else{wHomeState=digitalReadFast(wHomePin);if(wHomePinInverted)wHomeState=!wHomeState;}

  if(xMinState != xMinStateOld){xMinStateOld=xMinState;Serial.print("x");Serial.print(xMinState);}
  if(yMinState != yMinStateOld){yMinStateOld=yMinState;Serial.print("y");Serial.print(yMinState);}
  if(zMinState != zMinStateOld){zMinStateOld=zMinState;Serial.print("z");Serial.print(zMinState);}
  if(aMinState != aMinStateOld){aMinStateOld=aMinState;Serial.print("a");Serial.print(aMinState);}
  if(bMinState != bMinStateOld){bMinStateOld=bMinState;Serial.print("b");Serial.print(bMinState);}
  if(cMinState != cMinStateOld){cMinStateOld=cMinState;Serial.print("c");Serial.print(cMinState);}
  if(uMinState != uMinStateOld){uMinStateOld=uMinState;Serial.print("u");Serial.print(uMinState);}
  if(vMinState != vMinStateOld){vMinStateOld=vMinState;Serial.print("v");Serial.print(vMinState);}
  if(wMinState != wMinStateOld){wMinStateOld=wMinState;Serial.print("w");Serial.print(wMinState);}

  if(xHomeState != xHomeStateOld){xHomeStateOld=xHomeState;Serial.print("x");Serial.print(xHomeState+4);}
  if(yHomeState != yHomeStateOld){yHomeStateOld=yHomeState;Serial.print("y");Serial.print(yHomeState+4);}
  if(zHomeState != zHomeStateOld){zHomeStateOld=zHomeState;Serial.print("z");Serial.print(zHomeState+4);}
  if(aHomeState != aHomeStateOld){aHomeStateOld=aHomeState;Serial.print("a");Serial.print(aHomeState+4);}
  if(bHomeState != bHomeStateOld){bHomeStateOld=bHomeState;Serial.print("b");Serial.print(bHomeState+4);}
  if(cHomeState != cHomeStateOld){cHomeStateOld=cHomeState;Serial.print("c");Serial.print(cHomeState+4);}
  if(uHomeState != uHomeStateOld){uHomeStateOld=uHomeState;Serial.print("u");Serial.print(uHomeState+4);}
  if(vHomeState != vHomeStateOld){vHomeStateOld=vHomeState;Serial.print("v");Serial.print(vHomeState+4);}
  if(wHomeState != wHomeStateOld){wHomeStateOld=wHomeState;Serial.print("w");Serial.print(wHomeState+4);}

  if(xMaxState != xMaxStateOld){xMaxStateOld=xMaxState;Serial.print("x");Serial.print(xMaxState+1);}
  if(yMaxState != yMaxStateOld){yMaxStateOld=yMaxState;Serial.print("y");Serial.print(yMaxState+1);}
  if(zMaxState != zMaxStateOld){zMaxStateOld=zMaxState;Serial.print("z");Serial.print(zMaxState+1);}
  if(aMaxState != aMaxStateOld){aMaxStateOld=aMaxState;Serial.print("a");Serial.print(aMaxState+1);}
  if(bMaxState != bMaxStateOld){bMaxStateOld=bMaxState;Serial.print("b");Serial.print(bMaxState+1);}
  if(cMaxState != cMaxStateOld){cMaxStateOld=cMaxState;Serial.print("c");Serial.print(cMaxState+1);}
  if(uMaxState != uMaxStateOld){uMaxStateOld=uMaxState;Serial.print("u");Serial.print(uMaxState+1);}
  if(vMaxState != vMaxStateOld){vMaxStateOld=vMaxState;Serial.print("v");Serial.print(vMaxState+1);}
  if(wMaxState != wMaxStateOld){wMaxStateOld=wMaxState;Serial.print("w");Serial.print(wMaxState+1);}

  if(xMinState && !xMaxState)stepper0Goto=pos_x*stepsPerInchX*2;
  if(yMinState && !yMaxState)stepper1Goto=pos_y*stepsPerInchY*2;
  if(zMinState && !zMaxState)stepper2Goto=pos_z*stepsPerInchZ*2; // we need the *2 as we're driving a flip-flop routine (in stepLight function)
  if(aMinState && !aMaxState)stepper3Goto=pos_a*stepsPerInchA*2;
  if(bMinState && !bMaxState)stepper4Goto=pos_b*stepsPerInchB*2;
  if(cMinState && !cMaxState)stepper5Goto=pos_c*stepsPerInchC*2;
  if(uMinState && !uMaxState)stepper6Goto=pos_u*stepsPerInchU*2;
  if(vMinState && !vMaxState)stepper7Goto=pos_v*stepsPerInchV*2;
  if(wMinState && !wMaxState)stepper8Goto=pos_w*stepsPerInchW*2;

}

void processCommand()
{
    float xx=pos_x;
    float yy=pos_y;
    float zz=pos_z;
    float aa=pos_a;
    float bb=pos_b;
    float cc=pos_c;
    float uu=pos_u;
    float vv=pos_v;
    float ww=pos_w;

    float ss=revs_in;

  char *ptr=buffer;
  while(ptr && ptr<buffer+sofar)
  {
    ptr=strchr(ptr,' ')+1;
    switch(*ptr) {
      
      // These are axis move commands
      case 'x': case 'X': xx=atof(ptr+1); xx=xx/divisor; break;
      case 'y': case 'Y': yy=atof(ptr+1); yy=yy/divisor; break;
      case 'z': case 'Z': zz=atof(ptr+1); zz=zz/divisor; break;
      case 'a': case 'A': aa=atof(ptr+1); aa=aa/divisor; break;
      case 'b': case 'B': bb=atof(ptr+1); bb=bb/divisor; break;
      case 'c': case 'C': cc=atof(ptr+1); cc=cc/divisor; break;
      case 'u': case 'U': uu=atof(ptr+1); uu=uu/divisor; break;
      case 'v': case 'V': vv=atof(ptr+1); vv=vv/divisor; break;
      case 'w': case 'W': ww=atof(ptr+1); ww=ww/divisor; break;
      
      // Spindle speed command. In revs per second
      case 's': case 'S': ss=atof(ptr+1); spindleRPSin=ss; break;

    default: ptr=0; break;
    }
  }
  jog(xx,yy,zz,aa,bb,cc,uu,vv,ww);
  if(globalBusy<15)
  {
    // Insert LCD call here. (Updated when mostly idle.) Future project.
  }
}

void stepLight() // Set by jog() && Used by loop()
{
  unsigned long curTime=micros();
  if(curTime - stepTimeOld >= minStepTime)
  {
    stepState=!stepState;
    int busy=0;

    if(stepper0Pos != stepper0Goto){busy++;if(stepper0Pos > stepper0Goto){digitalWriteFast(dirPin0,!dirState0);digitalWriteFast(stepPin0,stepState);stepper0Pos--;}else{digitalWriteFast(dirPin0, dirState0);digitalWriteFast(stepPin0,stepState);stepper0Pos++;}}
    if(stepper1Pos != stepper1Goto){busy++;if(stepper1Pos > stepper1Goto){digitalWriteFast(dirPin1,!dirState1);digitalWriteFast(stepPin1,stepState);stepper1Pos--;}else{digitalWriteFast(dirPin1, dirState1);digitalWriteFast(stepPin1,stepState);stepper1Pos++;}}
    if(stepper2Pos != stepper2Goto){busy++;if(stepper2Pos > stepper2Goto){digitalWriteFast(dirPin2,!dirState2);digitalWriteFast(stepPin2,stepState);stepper2Pos--;}else{digitalWriteFast(dirPin2, dirState2);digitalWriteFast(stepPin2,stepState);stepper2Pos++;}}
    if(stepper3Pos != stepper3Goto){busy++;if(stepper3Pos > stepper3Goto){digitalWriteFast(dirPin3,!dirState3);digitalWriteFast(stepPin3,stepState);stepper3Pos--;}else{digitalWriteFast(dirPin3, dirState3);digitalWriteFast(stepPin3,stepState);stepper3Pos++;}}
    if(stepper4Pos != stepper4Goto){busy++;if(stepper4Pos > stepper4Goto){digitalWriteFast(dirPin4,!dirState4);digitalWriteFast(stepPin4,stepState);stepper4Pos--;}else{digitalWriteFast(dirPin4, dirState4);digitalWriteFast(stepPin4,stepState);stepper4Pos++;}}
    if(stepper5Pos != stepper5Goto){busy++;if(stepper5Pos > stepper5Goto){digitalWriteFast(dirPin5,!dirState5);digitalWriteFast(stepPin5,stepState);stepper5Pos--;}else{digitalWriteFast(dirPin5, dirState5);digitalWriteFast(stepPin5,stepState);stepper5Pos++;}}
    if(stepper6Pos != stepper6Goto){busy++;if(stepper6Pos > stepper6Goto){digitalWriteFast(dirPin6,!dirState6);digitalWriteFast(stepPin6,stepState);stepper6Pos--;}else{digitalWriteFast(dirPin6, dirState6);digitalWriteFast(stepPin6,stepState);stepper6Pos++;}}
    if(stepper7Pos != stepper7Goto){busy++;if(stepper7Pos > stepper7Goto){digitalWriteFast(dirPin7,!dirState7);digitalWriteFast(stepPin7,stepState);stepper7Pos--;}else{digitalWriteFast(dirPin7, dirState7);digitalWriteFast(stepPin7,stepState);stepper7Pos++;}}
    if(stepper8Pos != stepper8Goto){busy++;if(stepper8Pos > stepper8Goto){digitalWriteFast(dirPin8,!dirState8);digitalWriteFast(stepPin8,stepState);stepper8Pos--;}else{digitalWriteFast(dirPin8, dirState8);digitalWriteFast(stepPin8,stepState);stepper8Pos++;}}
    if(busy){digitalWriteFast(idleIndicator,LOW);if(globalBusy<255){globalBusy++;}}else{digitalWriteFast(idleIndicator,HIGH);if(globalBusy>0){globalBusy--;}
      if(giveFeedBackX){fbx=stepper0Pos/4/(stepsPerInchX*0.5);if(!busy){if(fbx!=fbxOld){fbxOld=fbx;Serial.print("fx");Serial.println(fbx,6);}}}
      if(giveFeedBackY){fby=stepper1Pos/4/(stepsPerInchY*0.5);if(!busy){if(fby!=fbyOld){fbyOld=fby;Serial.print("fy");Serial.println(fby,6);}}}
      if(giveFeedBackZ){fbz=stepper2Pos/4/(stepsPerInchZ*0.5);if(!busy){if(fbz!=fbzOld){fbzOld=fbz;Serial.print("fz");Serial.println(fbz,6);}}}
      if(giveFeedBackA){fba=stepper3Pos/4/(stepsPerInchA*0.5);if(!busy){if(fba!=fbaOld){fbaOld=fba;Serial.print("fa");Serial.println(fba,6);}}}
      if(giveFeedBackB){fbb=stepper4Pos/4/(stepsPerInchB*0.5);if(!busy){if(fbb!=fbbOld){fbbOld=fbb;Serial.print("fb");Serial.println(fbb,6);}}}
      if(giveFeedBackC){fbc=stepper5Pos/4/(stepsPerInchC*0.5);if(!busy){if(fbc!=fbcOld){fbcOld=fbc;Serial.print("fc");Serial.println(fbc,6);}}}
      if(giveFeedBackU){fbu=stepper6Pos/4/(stepsPerInchU*0.5);if(!busy){if(fbu!=fbuOld){fbuOld=fbu;Serial.print("fu");Serial.println(fbu,6);}}}
      if(giveFeedBackV){fbv=stepper7Pos/4/(stepsPerInchV*0.5);if(!busy){if(fbv!=fbvOld){fbvOld=fbv;Serial.print("fv");Serial.println(fbv,6);}}}
      if(giveFeedBackW){fbw=stepper8Pos/4/(stepsPerInchW*0.5);if(!busy){if(fbw!=fbwOld){fbwOld=fbw;Serial.print("fw");Serial.println(fbw,6);}}}
    }

    stepTimeOld=curTime;
  }
}

void stepMode(int axis, int mode) // May be omitted in the future. (Undecided)
{
  // called just once during setup()
  
  // This works OPPOSITE of what you might think.
  // Mode 1 = 1/16 step.
  // Mode 2 = 1/8 step.
  // Mode 4 = 1/4 step.
  // Mode 8 = 1/2 step.
  // Mode 16 = Full step.
  // Its simular to a car's gearbox with gears from low to high as in 1,2,4,8,16

  // Originally intended for dynamic microstepping control to reduce mpu overhead and speed steppers when moving large distances.
  // Real world result: Increased overhead and slowed steppers while juggling unessessary math and pin commands.
  
  boolean ms1;
  boolean ms2;
  boolean ms3;
  int count;
  if(mode>=16){ms1=LOW;ms2=LOW;ms3=LOW;count=16;}
  if(mode>=8 && mode<=15){ms1=HIGH;ms2=LOW;ms3=LOW;count=8;}
  if(mode>=4 && mode<=7){ms1=LOW;ms2=HIGH;ms3=LOW;count=4;}
  if(mode>=2 && mode<=3){ms1=HIGH;ms2=HIGH;ms3=LOW;count=2;}
  if(mode<=1){ms1=HIGH;ms2=HIGH;ms3=HIGH;count=1;}
  if(axis == 0 || 9){if(mode!=stepModeX){digitalWriteFast(chanXms1,ms1);digitalWriteFast(chanXms2,ms2);digitalWriteFast(chanXms3,ms3);stepModeX=count;}}
  if(axis == 1 || 9){if(mode!=stepModeY){digitalWriteFast(chanYms1,ms1);digitalWriteFast(chanYms2,ms2);digitalWriteFast(chanYms3,ms3);stepModeY=count;}}
  if(axis == 2 || 9){if(mode!=stepModeZ){digitalWriteFast(chanZms1,ms1);digitalWriteFast(chanZms2,ms2);digitalWriteFast(chanZms3,ms3);stepModeZ=count;}}
  if(axis == 3 || 9){if(mode!=stepModeA){digitalWriteFast(chanAms1,ms1);digitalWriteFast(chanAms2,ms2);digitalWriteFast(chanAms3,ms3);stepModeA=count;}}
  if(axis == 4 || 9){if(mode!=stepModeB){digitalWriteFast(chanBms1,ms1);digitalWriteFast(chanBms2,ms2);digitalWriteFast(chanBms3,ms3);stepModeB=count;}}
  if(axis == 5 || 9){if(mode!=stepModeC){digitalWriteFast(chanCms1,ms1);digitalWriteFast(chanCms2,ms2);digitalWriteFast(chanCms3,ms3);stepModeC=count;}}
  if(axis == 6 || 9){if(mode!=stepModeU){digitalWriteFast(chanUms1,ms1);digitalWriteFast(chanUms2,ms2);digitalWriteFast(chanUms3,ms3);stepModeU=count;}}
  if(axis == 7 || 9){if(mode!=stepModeV){digitalWriteFast(chanVms1,ms1);digitalWriteFast(chanVms2,ms2);digitalWriteFast(chanVms3,ms3);stepModeV=count;}}
  if(axis == 8 || 9){if(mode!=stepModeW){digitalWriteFast(chanWms1,ms1);digitalWriteFast(chanWms2,ms2);digitalWriteFast(chanWms3,ms3);stepModeW=count;}}
}

int determinInterrupt(int val)
{
  if(val<0) return -1;
  if(val==2) return 0;
  if(val==3) return 1;
  if(val==18) return 5;
  if(val==19) return 4;
  if(val==20) return 3;
  if(val==21) return 2;
}

volatile unsigned long spindleRevs=0;
float spindleRPS=0;
float spindleRPM=0;

void countSpindleRevs()
{
  spindleRevs++;
}

float updateSpindleRevs()
{
  unsigned long spindleTime=millis();
  if(spindleTime - spindleTimeOld >= 100)
  {
    spindleRPS=spindleRevs*10.0;
    spindleRPM=spindleRPS*60.0;
    spindleRevs=0;
  }
}

boolean spindleEnabled=false;
boolean spindleEnableState=spindleEnableInverted;

boolean spindleAtSpeed()
{
  if(spindleTach>0)
  {
    if(spindleRPSin<spindleRPS)
    {
      if(spindleRPSin*1.05<spindleRPS || !spindleEnabled)
      { /* Slow down. */
        if(spindleEnablePin>0){digitalWriteFast(spindleEnablePin,!spindleEnableState);}
      }else{
        if(spindleEnabled)
        { /* Speed up. */
          if(spindleEnablePin>0){digitalWriteFast(spindleEnablePin,spindleEnableState);}
        }
      }
      return true;
    }else{
      return false;
    }
  }else{
    return spindleEnabled; // No tach? We'll fake it.
  }
}

void setup()
{
  // Disable PWM drivers by default
  pinMode(pin_HE0_force_low,OUTPUT);digitalWrite(pin_HE0_force_low,LOW);
  pinMode(pin_HE1_force_low,OUTPUT);digitalWrite(pin_HE1_force_low,LOW);
  pinMode(pin_HE2_force_low,OUTPUT);digitalWrite(pin_HE2_force_low,LOW);
  pinMode(pin_HE3_force_low,OUTPUT);digitalWrite(pin_HE3_force_low,LOW);
  pinMode(pin_HEAux_force_low,OUTPUT);digitalWrite(pin_HEAux_force_low,LOW);
  pinMode(pin_HEBed_force_low,OUTPUT);digitalWrite(pin_HEBed_force_low,LOW); 
	
  // If using a spindle tachometer, setup an interrupt for it.
  if(spindleTach>0){int result=determinInterrupt(spindleTach);attachInterrupt(result,countSpindleRevs,FALLING);}
  
  // Setup Min limit switches.
  if(useRealMinX){pinMode(xMinPin,INPUT_PULLUP);if(!xMinPinInverted)digitalWriteFast(xMinPin,HIGH);}
  if(useRealMinY){pinMode(yMinPin,INPUT_PULLUP);if(!yMinPinInverted)digitalWriteFast(yMinPin,HIGH);}
  if(useRealMinZ){pinMode(zMinPin,INPUT_PULLUP);if(!zMinPinInverted)digitalWriteFast(zMinPin,HIGH);}
  if(useRealMinA){pinMode(aMinPin,INPUT_PULLUP);if(!aMinPinInverted)digitalWriteFast(aMinPin,HIGH);}
  if(useRealMinB){pinMode(bMinPin,INPUT_PULLUP);if(!bMinPinInverted)digitalWriteFast(bMinPin,HIGH);}
  if(useRealMinC){pinMode(cMinPin,INPUT_PULLUP);if(!cMinPinInverted)digitalWriteFast(cMinPin,HIGH);}
  if(useRealMinU){pinMode(uMinPin,INPUT_PULLUP);if(!uMinPinInverted)digitalWriteFast(uMinPin,HIGH);}
  if(useRealMinV){pinMode(vMinPin,INPUT_PULLUP);if(!vMinPinInverted)digitalWriteFast(vMinPin,HIGH);}
  if(useRealMinW){pinMode(wMinPin,INPUT_PULLUP);if(!wMinPinInverted)digitalWriteFast(wMinPin,HIGH);}

  // Setup Max limit switches.
  if(useRealMaxX){pinMode(xMaxPin,INPUT_PULLUP);if(!xMaxPinInverted)digitalWriteFast(xMaxPin,HIGH);}
  if(useRealMaxY){pinMode(yMaxPin,INPUT_PULLUP);if(!yMaxPinInverted)digitalWriteFast(yMaxPin,HIGH);}
  if(useRealMaxZ){pinMode(zMaxPin,INPUT_PULLUP);if(!zMaxPinInverted)digitalWriteFast(zMaxPin,HIGH);}
  if(useRealMaxA){pinMode(aMaxPin,INPUT_PULLUP);if(!aMaxPinInverted)digitalWriteFast(aMaxPin,HIGH);}
  if(useRealMaxB){pinMode(bMaxPin,INPUT_PULLUP);if(!bMaxPinInverted)digitalWriteFast(bMaxPin,HIGH);}
  if(useRealMaxC){pinMode(cMaxPin,INPUT_PULLUP);if(!cMaxPinInverted)digitalWriteFast(cMaxPin,HIGH);}
  if(useRealMaxU){pinMode(uMaxPin,INPUT_PULLUP);if(!uMaxPinInverted)digitalWriteFast(uMaxPin,HIGH);}
  if(useRealMaxV){pinMode(vMaxPin,INPUT_PULLUP);if(!vMaxPinInverted)digitalWriteFast(vMaxPin,HIGH);}
  if(useRealMaxW){pinMode(wMaxPin,INPUT_PULLUP);if(!wMaxPinInverted)digitalWriteFast(wMaxPin,HIGH);}

  // Setup Homing switches.
  if(useRealHomeX){pinMode(xHomePin,INPUT_PULLUP);if(!xHomePinInverted)digitalWriteFast(xHomePin,HIGH);}
  if(useRealHomeY){pinMode(yHomePin,INPUT_PULLUP);if(!yHomePinInverted)digitalWriteFast(yHomePin,HIGH);}
  if(useRealHomeZ){pinMode(zHomePin,INPUT_PULLUP);if(!zHomePinInverted)digitalWriteFast(zHomePin,HIGH);}
  if(useRealHomeA){pinMode(aHomePin,INPUT_PULLUP);if(!aHomePinInverted)digitalWriteFast(aHomePin,HIGH);}
  if(useRealHomeB){pinMode(bHomePin,INPUT_PULLUP);if(!bHomePinInverted)digitalWriteFast(bHomePin,HIGH);}
  if(useRealHomeC){pinMode(cHomePin,INPUT_PULLUP);if(!cHomePinInverted)digitalWriteFast(cHomePin,HIGH);}
  if(useRealHomeU){pinMode(uHomePin,INPUT_PULLUP);if(!uHomePinInverted)digitalWriteFast(uHomePin,HIGH);}
  if(useRealHomeV){pinMode(vHomePin,INPUT_PULLUP);if(!vHomePinInverted)digitalWriteFast(vHomePin,HIGH);}
  if(useRealHomeW){pinMode(wHomePin,INPUT_PULLUP);if(!wHomePinInverted)digitalWriteFast(wHomePin,HIGH);}

  // Enable stepper drivers.
  pinMode(xEnablePin,OUTPUT);digitalWrite(xEnablePin,LOW);
  pinMode(yEnablePin,OUTPUT);digitalWrite(yEnablePin,LOW);
  pinMode(zEnablePin,OUTPUT);digitalWrite(zEnablePin,LOW);
  pinMode(aEnablePin,OUTPUT);digitalWrite(aEnablePin,LOW);
  pinMode(bEnablePin,OUTPUT);digitalWrite(bEnablePin,LOW);
  pinMode(cEnablePin,OUTPUT);digitalWrite(cEnablePin,LOW);
  pinMode(uEnablePin,OUTPUT);digitalWrite(uEnablePin,LOW);
  pinMode(vEnablePin,OUTPUT);digitalWrite(vEnablePin,LOW);
  pinMode(wEnablePin,OUTPUT);digitalWrite(wEnablePin,LOW);

  // Setup step pins.
  pinMode(stepPin0,OUTPUT);
  pinMode(stepPin1,OUTPUT);
  pinMode(stepPin2,OUTPUT);
  pinMode(stepPin3,OUTPUT);
  pinMode(stepPin4,OUTPUT);
  pinMode(stepPin5,OUTPUT);
  pinMode(stepPin6,OUTPUT);
  pinMode(stepPin7,OUTPUT);
  pinMode(stepPin8,OUTPUT);
  
  // Setup dir pins.
  pinMode(dirPin0,OUTPUT);
  pinMode(dirPin1,OUTPUT);
  pinMode(dirPin2,OUTPUT);
  pinMode(dirPin3,OUTPUT);
  pinMode(dirPin4,OUTPUT);
  pinMode(dirPin5,OUTPUT);
  pinMode(dirPin6,OUTPUT);
  pinMode(dirPin7,OUTPUT);
  pinMode(dirPin8,OUTPUT);

  // Setup microStepping pins.
  pinMode(chanXms1,OUTPUT);pinMode(chanXms2,OUTPUT);pinMode(chanXms3,OUTPUT);
  pinMode(chanYms1,OUTPUT);pinMode(chanYms2,OUTPUT);pinMode(chanYms3,OUTPUT);
  pinMode(chanZms1,OUTPUT);pinMode(chanZms2,OUTPUT);pinMode(chanZms3,OUTPUT);
  pinMode(chanAms1,OUTPUT);pinMode(chanAms2,OUTPUT);pinMode(chanAms3,OUTPUT);
  pinMode(chanBms1,OUTPUT);pinMode(chanBms2,OUTPUT);pinMode(chanBms3,OUTPUT);
  pinMode(chanCms1,OUTPUT);pinMode(chanCms2,OUTPUT);pinMode(chanCms3,OUTPUT);
  pinMode(chanUms1,OUTPUT);pinMode(chanUms2,OUTPUT);pinMode(chanUms3,OUTPUT);
  pinMode(chanVms1,OUTPUT);pinMode(chanVms2,OUTPUT);pinMode(chanVms3,OUTPUT);
  pinMode(chanWms1,OUTPUT);pinMode(chanWms2,OUTPUT);pinMode(chanWms3,OUTPUT);
  
  // Setup eStop, power, start, stop, pause, resume, program step, spindle, coolant, LED and probe pins.
  if(useEstopSwitch){pinMode(eStopPin,INPUT_PULLUP);if(!eStopPinInverted){digitalWriteFast(eStopPin,HIGH);}}
  if(usePowerSwitch){pinMode(powerPin,INPUT_PULLUP);if(!powerPinInverted){digitalWriteFast(powerPin,HIGH);}}
  if(useProbe){pinMode(probePin,INPUT_PULLUP);if(!probePinInverted){digitalWriteFast(probePin,HIGH);}}
  if(useStartSwitch){pinMode(startPin,INPUT_PULLUP);if(!startPinInverted){digitalWriteFast(startPin,HIGH);}}
  if(useStopSwitch){pinMode(stopPin,INPUT_PULLUP);if(!stopPinInverted){digitalWriteFast(stopPin,HIGH);}}
  if(usePauseSwitch){pinMode(pausePin,INPUT_PULLUP);if(!pausePinInverted){digitalWriteFast(pausePin,HIGH);}}
  if(useResumeSwitch){pinMode(resumePin,INPUT_PULLUP);if(!resumePinInverted){digitalWriteFast(resumePin,HIGH);}}
  if(useStepSwitch){pinMode(stepPin,INPUT_PULLUP);if(!stepPinInverted){digitalWriteFast(stepPin,HIGH);}}
  if(powerLedPin > 0){pinMode(powerLedPin,OUTPUT);digitalWriteFast(powerLedPin,HIGH);}
  if(eStopLedPin>0){pinMode(eStopLedPin,OUTPUT);digitalWriteFast(eStopLedPin,LOW);}
  if(spindleEnablePin>0){pinMode(spindleEnablePin,OUTPUT);digitalWriteFast(spindleEnablePin,HIGH);}
  if(spindleDirection>0){pinMode(spindleDirection,OUTPUT);digitalWriteFast(spindleDirection,LOW);}
  if(spindleTach>0){pinMode(spindleTach,INPUT_PULLUP);digitalWriteFast(spindleTach,HIGH);}
  if(coolantMistPin>0){pinMode(coolantMistPin,OUTPUT);digitalWriteFast(coolantMistPin,LOW);}
  if(coolantFloodPin>0){pinMode(coolantFloodPin,OUTPUT);digitalWriteFast(coolantFloodPin,LOW);}
  if(powerSupplyPin>0){pinMode(powerSupplyPin,OUTPUT);digitalWriteFast(powerSupplyPin,psuState);}

  // Setup idle indicator led.
  pinMode(idleIndicator,OUTPUT);

  // Actually SET our microStepping mode. (If you must change this, re-adjust your stepsPerInch settings.)
  stepMode(9,1);
  
  // Setup serial link.
  Serial.begin(BAUD);
  
  // Initialize serial command buffer.
  sofar=0;
}

boolean spindleAtSpeedStateOld;

void loop()
{
  if(useEstopSwitch==true){boolean eStopState=digitalReadFast(eStopPin);if(eStopPinInverted){eStopState=!eStopState;}if(eStopState != eStopStateOld || eStopStateOld){eStopStateOld=eStopState;Serial.print("e");Serial.println(eStopState);delay(500);}}
  if(usePowerSwitch==true){boolean powerState=digitalReadFast(powerPin);if(powerPinInverted){powerState=!powerState;}if(powerState != powerStateOld){powerStateOld=powerState;if(powerSwitchIsMomentary){Serial.println("pt");}else{Serial.print("p");Serial.println(powerState);}}}
  if(useProbe==true){boolean probeState=digitalReadFast(probePin);if(probePinInverted){probeState=!probeState;}if(probeState != probeStateOld){probeStateOld=probeState;Serial.print("P");Serial.println(probeState);}}
  if(useStartSwitch==true){boolean startState=digitalReadFast(startPin);if(startPinInverted){startState=!startState;}if(startState != startStateOld){startStateOld=startState;Serial.print("h");Serial.println(startState);}}
  if(useStopSwitch==true){boolean stopState=digitalReadFast(stopPin);if(stopPinInverted){stopState=!stopState;}if(stopState != stopStateOld){stopStateOld=stopState;Serial.print("h");Serial.println(stopState+2);}}
  if(usePauseSwitch==true){boolean pauseState=digitalReadFast(pausePin);if(pausePinInverted){pauseState=!pauseState;}if(pauseState != pauseStateOld){pauseStateOld=pauseState;Serial.print("h");Serial.println(pauseState+4);}}
  if(useResumeSwitch==true){boolean resumeState=digitalReadFast(resumePin);if(resumePinInverted){resumeState=!resumeState;}if(resumeState != resumeStateOld){resumeStateOld=resumeState;Serial.print("h");Serial.println(resumeState+6);}}
  if(useStepSwitch==true){boolean stepState=digitalReadFast(stepPin);if(stepPinInverted){stepState=!stepState;}if(stepState != stepStateOld){stepStateOld=stepState;Serial.print("h");Serial.println(stepState+8);}}


  // listen for serial commands
  while(Serial.available() > 0) {
    buffer[sofar++]=Serial.read();
    if(buffer[sofar-1]==';') break;  // in case there are multiple instructions
    
  }
  // Received a "+" turn something on.
  if(sofar>0 && buffer[sofar-3]=='+') {
    if(sofar>0 && buffer[sofar-2]=='P') { /* Power LED & PSU   ON */ if(powerLedPin>0){digitalWriteFast(powerLedPin,HIGH);}if(powerSupplyPin>0){digitalWriteFast(powerSupplyPin,psuState);}}
    if(sofar>0 && buffer[sofar-2]=='E') { /* E-Stop Indicator  ON */ if(eStopLedPin>0){digitalWriteFast(eStopLedPin,HIGH);}}
    if(sofar>0 && buffer[sofar-2]=='S') { /* Spindle power     ON */ spindleEnabled=true;}
    if(sofar>0 && buffer[sofar-2]=='D') { /* Spindle direction CW */ if(spindleDirection>0){digitalWriteFast(spindleDirection,spindleState);}}
    if(sofar>0 && buffer[sofar-2]=='M') { /* Coolant Mist      ON */ if(coolantMistPin>0){digitalWriteFast(coolantMistPin,HIGH);}}
    if(sofar>0 && buffer[sofar-2]=='F') { /* Coolant Flood     ON */ if(coolantFloodPin>0){digitalWriteFast(coolantFloodPin,HIGH);}}
  }

  // Received a "-" turn something off.
  if(sofar>0 && buffer[sofar-3]=='-') {
    if(sofar>0 && buffer[sofar-2]=='P') { /* Power LED & PSU   OFF */ if(powerLedPin>0){digitalWriteFast(powerLedPin,LOW);}if(powerSupplyPin>0){digitalWriteFast(powerSupplyPin,!psuState);}}
    if(sofar>0 && buffer[sofar-2]=='E') { /* E-Stop Indicator  OFF */ if(eStopLedPin>0){digitalWriteFast(eStopLedPin,LOW);}}
    if(sofar>0 && buffer[sofar-2]=='S') { /* Spindle power     OFF */ spindleEnabled=false;}
    if(sofar>0 && buffer[sofar-2]=='D') { /* Spindle direction CCW */ if(spindleDirection>0){digitalWriteFast(spindleDirection,!spindleState);}}
    if(sofar>0 && buffer[sofar-2]=='M') { /* Coolant Mist      OFF */ if(coolantMistPin>0){digitalWriteFast(coolantMistPin,LOW);}}
    if(sofar>0 && buffer[sofar-2]=='F') { /* Coolant Flood     OFF */ if(coolantFloodPin>0){digitalWriteFast(coolantFloodPin,LOW);}}
  }
 
  // Received a "?" about something give an answer.
  if(sofar>0 && buffer[sofar-3]=='?') {
    if(sofar>0 && buffer[sofar-2]=='V') { /* Report version */ Serial.println(VERSION);}
    if(sofar>0 && buffer[sofar-2]=='R') { /* Report role    */ Serial.println(ROLE);}
  }
  
  // if we hit a semi-colon, assume end of instruction.
  if(sofar>0 && buffer[sofar-1]==';') {
 
    buffer[sofar]=0;
 
    // do something with the command
    processCommand();
 
    // reset the buffer
    sofar=0;
  }
  updateSpindleRevs();
  if(!globalBusy){
       if(spindleEnabled!=true)
      { /* motor off */
        if(spindleEnablePin>0){digitalWriteFast(spindleEnablePin,!spindleEnableState);}
       }else{
	boolean spindleAtSpeedState=spindleAtSpeed();
	if(spindleAtSpeedState != spindleAtSpeedStateOld){
		  spindleAtSpeedStateOld=spindleAtSpeedState;
		  Serial.print("S");
		  Serial.println(spindleAtSpeedState);
	}
	}
  }
  
  stepLight(); // call every loop cycle to update stepper motion.
}
