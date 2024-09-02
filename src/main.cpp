#include <RotaryEncoder.h>
#include <Joystick.h>

// Define variables
#define NUM_PADS 4
#define NUM_PEDALS 1

// Pines de entrada para pads, pedales, botones, y encoder
int padPins[NUM_PADS] = {A0, A1, A2, A3}; // Pines de los pads
int pedalPins[NUM_PEDALS] = {16}; // Pines de los pedales
int buttonPins[] = {0, 1, 2, 3, 4, 5, 6}; // Pines para A, B, X, Y, Select, Start, Home
int encoderPinCW = 15; // pin del encoder movimiento horario
int encoderPinCCW = 14; // pin del encoder movimiento anti-horario
int encoderSwitchPin = 7; // Switch interno del encoder

// Instancia de encoder
RotaryEncoder encoder(encoderPinCW, encoderPinCCW, RotaryEncoder::LatchMode::FOUR3); 

// Umbral para detectar un golpe en los pads
int threshold = 80; // Umbral para detectar un golpe en los pads
int encoderDelay  = 5;  // Tiempo de espera luego de presionar una posición en el dpad

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

  // Control de modo del encoder
  bool encoderSwitchValue = digitalRead(encoderSwitchPin);

  if (encoderSwitchValue == LOW && lastEncoderSwitchValue == HIGH){
    Horizontal = !Horizontal;
    delay(200); // Delay para evitar rebotes en el switch
  }

  // Manejo de botones del encoder
  static int pos = 0;
  encoder.tick();
  int direction = (int)(encoder.getDirection());
  int newPos = encoder.getPosition();
  if (direction > 0 ) {
    if (Horizontal){  
      joystick.setHatSwitch(0, 270); // Dpad-LEFT
      Serial.println("Dpad-LEFT");
      delay(encoderDelay);
    }
    else {
      joystick.setHatSwitch(0, 0); // Dpad-UP
      Serial.println("Dpad-UP");
      delay(encoderDelay);
    }
  }
  if (direction < 0) {
    if (Horizontal){  
      joystick.setHatSwitch(0, 90); // Dpad-RIGHT
      Serial.println("Dpad-RIGHT");
      delay(encoderDelay);
    }
    else {
      joystick.setHatSwitch(0, 180); // Dpad-DOWN
      Serial.println("Dpad-DOWN");
      delay(encoderDelay);
    }
  }
  else {
    joystick.setHatSwitch(0, -1);
    // Serial.println("Dpad-rest");
    delay(1);
  }
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
        Serial.print("Pad ");
        Serial.print(i);
        Serial.println(" hit");
      }
    }
    else {
      // Resetear el estado del pad cuando la señal baja del umbral
      if (padState[i]) {
        padState[i] = false;
      }
    }
  }  
}
