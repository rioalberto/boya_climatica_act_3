//boya_climatica.ino
#include "boya_climatica.h"

void setup() {
  pinMode(LDRPIN, INPUT); // Declaración como entrada del pin asociado a la resistencia fotosensible
  pinMode(BUTTONPIN, INPUT_PULLUP); // Declaración como entrada con resistencia de Pull Up del pin asociado al botón
  pinMode(CALENTADORPIN, OUTPUT); // Declaración como salida del pin asociado a la alimentación del calentador
  analogWrite(CALENTADORPIN, 0);  // Apagar calentador al inicio

  compuerta.attach(SERVOPIN); // Inicialización del objeto servomotor
  compuerta.write(90); // Compuerta cerrada al inicio

  dht.begin(); // Inicialización del objeto DHT

  rtc.begin(); // Inicialización del objeto RTC

  turbina.setSpeed(1000); // Define la velocidad de la turbina en 1000 rpm

  receiver.enableIRIn(); // Inicialización del receptor del mando a distancia

  //Serial.begin(9600); // Inicialización puerto serie a 9600 baudios

  // Inicialización del objeto LCD
  lcd.init();
  lcd.backlight();

  read_dht(); // Leer temperatura y humedad iniciales
}

void loop() {

  acquisition_int();

  if (acquisition) { // Solo lee cuando al bandera de adquisición se activa cada segundo
    acquisition = false;  // Resetear bandera
    
    if(pwr_on_flag) {
    	//read_dht();
    	read_lux();
    	read_rtc();
    	read_mq2();
    	temp_control();
    }

    if (receiver.decode()) // Nueva pulsación detectada
    {
    	translateIR();
      IR_mode_one_time();
    	receiver.resume();  
    }

    IR_mode_continuo();

  }
  
  if (digitalRead(BUTTONPIN) == LOW) { // Lee pulsaciones en el botón
    if (millis() - lastButtonTouch > ButtonDelay) { //Evitar rebotes: si entre pulsaciones no han pasado 200ms no hace nada
      mode = mode + 1; // Cada vez que se pulsa el botón incrementa el modo en una unidad
      if (mode == 5) {
        mode = 0;
      }
      lastButtonTouch = millis(); // Guarda el último instante temporal en el que se pulsó el botón. millis() devuelve el número de milisegundos transcurridos desde que se inició el programa
    }
  }

  if (motor_flag){
    turbina.step(stepsPerRevolution); // Activa el motor de la turbina
  }
}

void acquisition_int() {
  // Simular temporizador con millis()
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    acquisition = true;
  }
}
