#include <RotaryEncoder.h>
#include <Joystick.h>



// Define variables
#define NUM_PADS 7
#define NUM_PEDALS 1

// Pines de entrada para pads, pedales, botones, y encoder
int padPins[NUM_PADS] = {A0, A1, A2, A3, A8, A9, A10}; // Pines de los pads
int pedalPins[NUM_PEDALS] = {2}; // Pines de los pedales
int buttonPins[] = {4, 5, 6, 7, 8, 9}; // Pines para A, B, X, Y, Select, Start, Home
int encoderPinCW = 14; // pin del encoder movimiento horario
int encoderPinCCW = 16; // pin del encoder movimiento anti-horario
int encoderSwitchPin = 10; // Switch interno del encoder

// Instancia de encoder
RotaryEncoder encoder(encoderPinCW, encoderPinCCW, RotaryEncoder::LatchMode::FOUR3); 

// Umbral para detectar un golpe en los pads
int threshold = 6000;

// Instancia del Joystick
Joystick_ joystick(
  JOYSTICK_DEFAULT_REPORT_ID,  // Report ID
  JOYSTICK_TYPE_GAMEPAD,       // Tipo de joystick
  32,                          // Número de botones
  1,  // Número de HAT switches
  false, false, false,         // Ejes X, Y, Z
  false, false, false,         // Rotación en X, Y, Z
  false, false,                // Rudder, throttle
  false, false, false          // Accelerator, brake, steering
);



  // Estado de los botones
bool padState[NUM_PADS] = {false};
bool pedalState[NUM_PEDALS] = {false};
bool buttonState[sizeof(buttonPins) / sizeof(buttonPins[0])] = {false};

void setup() {

            Serial.begin(9600);

            // Configuración de pines
            pinMode(encoderSwitchPin, INPUT_PULLUP);
              
            for (int i = 0; i < NUM_PADS; i++) {
              pinMode(padPins[i], INPUT);
            }

            for (int i = 0; i < NUM_PEDALS; i++) {
              pinMode(pedalPins[i], INPUT_PULLUP);
            }

            for (int i = 0; i < sizeof(buttonPins) / sizeof(buttonPins[0]); i++) {
              pinMode(buttonPins[i], INPUT_PULLUP);
            }

              // Inicializar Joystick
            joystick.begin();




}
bool Horizontal = false;
bool lastEncoderSwitchValue = true;
void loop() {

//Control de modo del encoder
bool encoderSwitchValue = digitalRead(encoderSwitchPin);

if (encoderSwitchValue == LOW && lastEncoderSwitchValue == HIGH){
      Horizontal = !Horizontal;
    }

//Manejo de botones del encoder

  static int pos = 0;
  encoder.tick();
  int direction = (int)(encoder.getDirection());
  int newPos = encoder.getPosition();
  if (direction > 0 ) {
    if (Horizontal){  
      joystick.setHatSwitch(0, 270); // Dpad-LEFT
      Serial.println("Dpad-LEFT");

     }
     else {joystick.setHatSwitch(0, 0); // Dpad-UP
       Serial.println("Dpad-UP");
      }}
  else if (direction < 0) {
    if (Horizontal){  
      joystick.setHatSwitch(0, 90); // Dpad-RIGHT}
      Serial.println("Dpad-RIGHT");
      }
      else {joystick.setHatSwitch(0, 180); // Dpad-DOWN
      Serial.println("Dpad-DOWN");
      }}
      else {joystick.setHatSwitch(0, -1);}
  pos = newPos;
  lastEncoderSwitchValue = encoderSwitchValue;


  // Leer botones
  for (int i = 0; i < sizeof(buttonPins) / sizeof(buttonPins[0]); i++) {
    int buttonValue = digitalRead(buttonPins[i]);
    if (buttonValue == LOW && !buttonState[i]) {
      joystick.pressButton(NUM_PADS + NUM_PEDALS + i);
      buttonState[i] = true;
    } else if (buttonValue == HIGH && buttonState[i]) {
      joystick.releaseButton(NUM_PADS + NUM_PEDALS + i);
      buttonState[i] = false;
    }
  }

  // Leer pedales
  for (int i = 0; i < NUM_PEDALS; i++) {
    int pedalValue = digitalRead(pedalPins[i]);
    if (pedalValue == LOW && !pedalState[i]) {
      joystick.pressButton(NUM_PADS + i);
      pedalState[i] = true;
    } else if (pedalValue == HIGH && pedalState[i]) {
      joystick.releaseButton(NUM_PADS + i);
      pedalState[i] = false;
    }
  }

  // Leer pads
  for (int i = 0; i < NUM_PADS; i++) {
    int sensorValue = analogRead(padPins[i]);
    if (sensorValue > threshold && !padState[i]) {
      joystick.pressButton(i);
      padState[i] = true;
    } else if (sensorValue <= threshold && padState[i]) {
      joystick.releaseButton(i);
      padState[i] = false;
    }    
   }  
   //
}