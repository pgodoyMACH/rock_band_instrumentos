#include <RotaryEncoder.h>
#include <Joystick.h>

// Define variables
#define NUM_PADS 7
#define NUM_PEDALS 1 

// Pines de entrada para pads, pedales, botones, y encoder
int padPins[NUM_PADS] = {A0, A1, A2, A3, A8, A9, A10}; // Pines de los pads
int pedalPins[NUM_PEDALS] = {16}; // Pines de los pedales
int buttonPins[] = {0, 1, 2, 3, 4, 5, 6}; // Pines para A, B, X, Y, Select, Start, Home
int encoderPinCW = 15; // pin del encoder movimiento horario
int encoderPinCCW = 14; // pin del encoder movimiento anti-horario
int encoderSwitchPin = 7; // Switch interno del encoder

// Instancia de encoder
RotaryEncoder encoder(encoderPinCW, encoderPinCCW, RotaryEncoder::LatchMode::FOUR3); 

// Umbral para detectar un golpe en los pads
int threshold = 100; // Umbral para detectar un golpe en los pads
int encoderDelay  = 50; //50 optimo // Tiempo de espera luego de presionar una posición en el dpad

// Variables para debounce de los pads
unsigned long padLastTriggerTime[NUM_PADS] = {0}; // Último tiempo que cada pad fue presionado
const unsigned long padDebounceTime = 50; // Tiempo de debounce en milisegundos

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

  // Control de modo del encoder
  bool encoderSwitchValue = digitalRead(encoderSwitchPin);

  if (encoderSwitchValue == LOW && lastEncoderSwitchValue == HIGH){
    Horizontal = !Horizontal;
    delay(20); // Delay para evitar rebotes en el switch
  }

  // Manejo de botones del encoder
  static int pos = 0;
  encoder.tick();
  int direction = (int)(encoder.getDirection());
  int newPos = encoder.getPosition();
  if (direction > 0 ) {
    if (Horizontal){  
      joystick.setHatSwitch(0, 270); // Dpad-LEFT
      delay(encoderDelay);
    }
    else {
      joystick.setHatSwitch(0, 0); // Dpad-UP
      delay(encoderDelay);
    }
  }
  if (direction < 0) {
    if (Horizontal){  
      joystick.setHatSwitch(0, 90); // Dpad-RIGHT
      delay(encoderDelay);
    }
    else {
      joystick.setHatSwitch(0, 180); // Dpad-DOWN
      delay(encoderDelay);
    }
  }
  else {
    joystick.setHatSwitch(0, -1);
    delay(0);

  }
  pos = newPos;
  lastEncoderSwitchValue = encoderSwitchValue;


  // Leer botones
  for (int i = 0; i < sizeof(buttonPins) / sizeof(buttonPins[0]); i++) {
    int buttonValue = digitalRead(buttonPins[i]);
    int correspondingPadIndex = -1;
    
    // Asignar botones a los pads correspondientes
    if (i == 5) correspondingPadIndex = 0; // Pin 5 al pad 0
    if (i == 4) correspondingPadIndex = 3; // Pin 4 al pad 3
    if (i == 6) correspondingPadIndex = 2; // Pin 6 al pad 2
    if (i == 3) correspondingPadIndex = 1; // Pin 3 al pad 1
    
    if (buttonValue == LOW && !buttonState[i]) {
      if (correspondingPadIndex >= 0) {
        joystick.pressButton(correspondingPadIndex);
        joystick.releaseButton(correspondingPadIndex);
      }
      joystick.pressButton(NUM_PADS + NUM_PEDALS + i);
      buttonState[i] = true;
    } else if (buttonValue == HIGH && buttonState[i]) {
      if (correspondingPadIndex >= 0) {
        joystick.releaseButton(correspondingPadIndex);
      }
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
    
  // Leer pads con debounce
  for (int i = 0; i < NUM_PADS; i++) {
    int sensorValue = analogRead(padPins[i]);
    unsigned long currentTime = millis();
    
    if (sensorValue > threshold) {
      // Verificar si el pad no está ya en estado presionado y ha pasado el tiempo de debounce
      if (!padState[i] && (currentTime - padLastTriggerTime[i] > padDebounceTime)) {
        // Registrar la pulsación del pad
        joystick.pressButton(i);
        joystick.releaseButton(i); // Liberar inmediatamente para registrar un solo evento
        padState[i] = true;
        padLastTriggerTime[i] = currentTime;
 
      }
    }
    else {
      // Resetear el estado del pad cuando la señal baja del umbral
      if (padState[i]) {
        padState[i] = false;
     }
    }
 }
 delay(1);
}
