//boya_climatica.h

#include <LiquidCrystal_I2C.h> // Librería para usar Display
#include <DHT.h> // Librería para usar DHT22
#include "RTClib.h" // Librería para usar RTC
#include <Servo.h> // Librería para usar servomotor
#include <Stepper.h> // Librería para usar el motor de la turbina
#include <IRremote.h> // Librería para usar el mando a distancia

#define LDRPIN A0 // Pin asociado a la resistencia fotosensible
#define DHTPIN 12 // Pin asociado al DHT22
#define DHTTYPE DHT22 // Tipo de sensor de la familia DHT
#define MQ2PIN 7 // Pin asociado al MQ-2
#define CALENTADORPIN 4 // Pin asociado al calentador de las baterías
#define SERVOPIN 3 // Pin asociado al servo que abre la compuerta
#define BUTTONPIN 13 // Pin asociado al botón
#define MOTORPIN1 8 // Pines asociados al motor de la turbina
#define MOTORPIN2 9
#define MOTORPIN3 10
#define MOTORPIN4 11
#define RECEIVERPIN 2 // Pin asociado al mando a distancia

String lastLine0 = "";
String lastLine1 = "";
String line0 = "";
String line1 = "";

uint8_t mode = 0; // Variable que indica el modo de operación
uint8_t lastMode = 0;
volatile bool acquisition = false; // Bandera que marcará lectura
volatile bool motor_flag = false; // Bandera que marcará caundo el motor se debe activar

// Constantes del LDR
const float GAMMA = 0.7; // Constante de la relación logarítimica entre la impedancia del LDR y la luminiscencia (lux)
const float RL10 = 50; // Impedancia del LDR a 10 lux

unsigned long lastButtonTouch = 0; // Variable que almacena la última vez que se pulsó el botón
const unsigned long ButtonDelay = 200; // Intervalo temporal que se debe cumplir entre pulsaciones

unsigned long previousMillis = 0; // // Variable que almacena la última vez que se levó a cabo una interrupción
const unsigned long interval = 1000; // Intervalo temporal de 1s que se debe cumplir entre interrupciones

bool MQ2Value; // Variable que almacena el valor digital de salida del MQ2

DateTime now;  // Variable de tipo DateTime (clase incluida en la librería RTClib.h que representa una fecha y una hora completas)

float humidity; // Variable de punto flotante que alamacena el valor de humedad medido por el DHT22
float temperature; // Variable de punto flotante que almacena el valor de temperatura medido por el DHT22
float temp_limite = 25.00; // Límite
float margen = 3.00;
float limiteInferior = temp_limite - margen; // Límite inferior del margen de temperatura deseado en la boya
float limiteSuperior = temp_limite + margen; // Límite superior del margen de temperatura deseado en la boya
// Valores de cambio de temperatura por segundo
const float cambioCalor = 0.04;   // °C/s al calentar
const float cambioFrio  = -0.28;  // °C/s al enfriar
const float dt = 1.0;             // intervalo en segundos entre mediciones
float delta = 0.0;

float lux; // Variable de punto flotante que alamacena el valor de luminiscencia medido por la resistencia fotosensible

const int stepsPerRevolution = 200; // Número de pasos por revolución del motor de la turbina

int estadoAnterior;
int estadoActual;
bool pwr_on_flag = false;
int button_value = 0;

DHT dht(DHTPIN, DHTTYPE); // Declaración del objeto DHT
LiquidCrystal_I2C lcd(0x27, 16, 2); // Declaración del objeto LCD
RTC_DS1307 rtc; // Declaración del objeto RTC
Servo compuerta; // Declaración del objeto asociado al servo que abre la compuerta
Stepper turbina(stepsPerRevolution, MOTORPIN1, MOTORPIN2, MOTORPIN3, MOTORPIN4); // Declaración del objeto asociado al motor de la turbina
IRrecv receiver(RECEIVERPIN); //Declaración del objeto del mando a distancia

// Función que lee los valores de humedad y temperatura medidos por el DHT22
void read_dht() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
}

// Función que lee los valores de luminiscencia medido por la resistencia fotosensible
void read_lux() {
  int analogValue = analogRead(LDRPIN);
  //Conversión del valor analógico a unidades de lux
  float voltage = analogValue / 1024. * 5;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
}

// Función que lee los valores de fecha y hora medidos por el RTC DS1317
void read_rtc() {
  now = rtc.now();
}

// Función que lee el valor digital medido por el MQ2
void read_mq2() {
  MQ2Value = digitalRead(MQ2PIN);
}

// Función que actualiza el valor del limite de temperatura
void mod_temp_lim() {
  if (button_value == 144) {
  	temp_limite += 0.2;
	  limiteInferior = temp_limite - margen;
	  limiteSuperior = temp_limite + margen;
  } else if (button_value == 224) {
  	temp_limite -= 0.2;
	  limiteInferior = temp_limite - margen;
	  limiteSuperior = temp_limite + margen;
  }
}

// Función que actualiza el valor del margen de temperatura
void mod_temp_margin() {
  if (button_value == 2) {
  	margen += 0.2;
	  limiteInferior = temp_limite - margen;
	  limiteSuperior = temp_limite + margen;
  } else if (button_value == 152) {
  	margen -= 0.2;
	  limiteInferior = temp_limite - margen;
	  limiteSuperior = temp_limite + margen;
  }
}

// Función que controla que la temperatura de la boya esté en los margenes deseados
void temp_control() {
 // Lógica de control con simulación térmica
    if (temperature > limiteSuperior) {
      // Enfriar
      compuerta.write(0);
      motor_flag = true;
      temperature += cambioFrio * dt;
    } else if (temperature < limiteInferior) {
      // Calentar
      digitalWrite(CALENTADORPIN, HIGH);
      temperature += cambioCalor * dt;
    } else {
      // Estable: apagar todo
      compuerta.write(90);
      analogWrite(CALENTADORPIN, 0);
      motor_flag = false;
      if(button_value!=90 && button_value!=66 && button_value!=74 && button_value!=82) {
        read_dht();
      }
    }
}

// Función que activa el sistema de refrigeración
void cold_pwr_on() {
    // Enfriar
    compuerta.write(0);
    motor_flag = true;
    temperature += cambioFrio * dt;
}

// Función que desactiva el sistema de refrigeración
void cold_pwr_off() {
    // Apagar todo
    compuerta.write(90);
    motor_flag = false;
}

// Función que activa el sistema calefactable
void warm_pwr_on() {
    // Calentar
    digitalWrite(CALENTADORPIN, HIGH);
    temperature += cambioCalor * dt;
}

// Función que desactiva el sistema calefactable
void warm_pwr_off() {
    // Apagar todo
    analogWrite(CALENTADORPIN, 0);
}

// Función que limpia la consola y actualiza los valores
void clear_lcd(String &line0, String &lastLine0, String &line1, String &lastLine1) {
  // Solo actualizar si algo ha cambiado
  if (line0 != lastLine0 || line1 != lastLine1 || mode != lastMode) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line0);
    lcd.setCursor(0, 1);
    lcd.print(line1);
    lastLine0 = line0;
    lastLine1 = line1;
    lastMode = mode;
  }
}

// Función que representa la bienvenida
void display_welcome() {
  line0 = "Bienvenido ";
  line1 = "a iBoya";
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa la despedida
void display_goodbay() {
  line0 = "Modo reposo";
  line1 = "iBoya";
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa los valores de fecha y hora medidos por el RTC DS1317
void display_date_hour() {
  line0 = "Fecha: " + String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
  line1 = "Hora: " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa los valores de luminiscencia medido por la resistencia fotosensible
void display_lux() {
  line0 = "Lux: " + String(lux, 2);
  line1 = "                            ";
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa los valores de humedad y temperatura medidos por el DHT22
void display_dht22() {
  line0 = "H(%): " + String(humidity, 2);
  line1 = "T(\xDF""C): " + String(temperature, 2);
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa los valores de limite de temperatura y margen actualizados
void display_temp_lim_margin() {
  line0 = "T_lim(\xDF""C): " + String(temp_limite, 2);
  line1 = "Margen(\xDF""C): " + String(margen, 2);
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa el valor digital medido por el MQ2
void display_MQ2() {
  line0 = "Calidad aire: ";
  line1 = MQ2Value ? "Limpio" : "Contaminado";
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa el control de la temperatura de la boya
void display_temp_ctl() {
  if (temperature > limiteSuperior) {
    // Enfriar
    line0 = "Enfriando";
    line1 = "T(\xDF""C): " + String(temperature, 2);
    clear_lcd(line0, lastLine0, line1, lastLine1);
  } else if (temperature < limiteInferior) {
    // Calentar
    line0 = "Calentando";
    line1 = "T(\xDF""C): " + String(temperature, 2);
    clear_lcd(line0, lastLine0, line1, lastLine1);
  } else {
    // Estable
    line0 = "Estable";
    line1 = "T(\xDF""C): " + String(temperature, 2);
    clear_lcd(line0, lastLine0, line1, lastLine1);
  }
}

// Función que representa el enfriamiento de la temperatura de la boya
void display_cold_ctl() {
  // Enfriar
  line0 = "Enfriando";
  line1 = "T(\xDF""C): " + String(temperature, 2);
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa el calentamiento de la temperatura de la boya
void display_warm_ctl() {
  // Calentar
  line0 = "Calentando";
  line1 = "T(\xDF""C): " + String(temperature, 2);
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa la temperatura final tras apagado de refrigerador
void display_pwr_off_cold() {
  line0 = "Refrig. OFF";
  line1 = "T(\xDF""C): " + String(temperature, 2);
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

// Función que representa la temperatura final tras apagado de calentador
void display_pwr_off_warm() {
  line0 = "Calentador OFF";
  line1 = "T(\xDF""C): " + String(temperature, 2);
  clear_lcd(line0, lastLine0, line1, lastLine1);
}

void test_mode() { // Modo test cambiando de modo con botón
    switch (mode) {
      case 0: // Modo 0: Representa en el LCD fecha y hora
        display_date_hour();
      break;
      case 1: // Modo 1: Representa en el LCD el valor de luminiscencia
        display_lux();
      break;
      case 2: // Modo 2: Representa en el LCD los valores de humedad y temperatura
        display_dht22();
      break;
      case 3: // Modo 3: Representa en el LCD si el aire está contaminado o limpio
        display_MQ2();
      break;
      case 4: // Modo 4: Representa en el LCD el estado del sistema de monitorización térmica y la temperatura actual
        display_temp_ctl();
      break;
      default:
      break;
    }
}

void translateIR() {
    button_value = receiver.decodedIRData.command;
}

void IR_mode_one_time(){
      switch (button_value) {
      case 162: // Power: Encender/Apagar todo
        pwr_on_flag = !pwr_on_flag;
        if (pwr_on_flag) {
          display_welcome();
        } else {
          display_goodbay();
        }
      break;
      case 194: // Back: Estado anterior
        button_value = estadoAnterior;
      break;
      case 144: // Flecha derecha: Subir limite de temperatura
	      mod_temp_lim();
        display_temp_lim_margin();
      break;
      case 224: // Flecha izquierda: Bajar limite de temperatura
	      mod_temp_lim();
        display_temp_lim_margin();
      break;
      case 2: // Signo de suma - Subir margen
	      mod_temp_margin();
        display_temp_lim_margin();
      break;
      case 152: // Signo de resta: Bajar margen
	      mod_temp_margin();
        display_temp_lim_margin();
      break;
      case 48: // Modo 1: Representa en el LCD fecha y hora
	      estadoAnterior = estadoActual;
        estadoActual = 48;
      break;
      case 24: // Modo 2: Representa en el LCD el valor de luminiscencia
	      estadoAnterior = estadoActual;
        estadoActual = 24;
      break;
      case 122: // Modo 3: Representa en el LCD los valores de humedad y temperatura
	      estadoAnterior = estadoActual;
        estadoActual = 122;
      break;
      case 16: // Modo 4: Representa en el LCD si el aire está contaminado o limpio
	      estadoAnterior = estadoActual;
        estadoActual = 16;
      break;
      case 56: // Modo 5: Representa en el LCD el estado del sistema de monitorización térmica y la temperatura actual
	      estadoAnterior = estadoActual;
        estadoActual = 56;
      break;
      case 90: // Modo 6: Abrir compuerta y activar motor
	      estadoAnterior = estadoActual;
        estadoActual = 90;
      break;
      case 66: // Modo 7: Cerrar compuerta y desactivar motor
	      estadoAnterior = estadoActual;
        estadoActual = 66;
      break;
      case 74: // Modo 8: Encender calentador
	      estadoAnterior = estadoActual;
        estadoActual = 74;
      break;
      case 82: // Modo 9: Apagar calentador
	      estadoAnterior = estadoActual;
        estadoActual = 82;
      break;
      default:
      break;
    }
}

void IR_mode_continuo() { // Modo mando a distancia
    switch (button_value) {
      case 34: // Test: Entra en modo test
        test_mode();
      break;
      case 48: // Modo 1: Representa en el LCD fecha y hora
        display_date_hour();
      break;
      case 24: // Modo 2: Representa en el LCD el valor de luminiscencia
        display_lux();
      break;
      case 122: // Modo 3: Representa en el LCD los valores de humedad y temperatura
        display_dht22();
      break;
      case 16: // Modo 4: Representa en el LCD si el aire está contaminado o limpio
        display_MQ2();
      break;
      case 56: // Modo 5: Representa en el LCD el estado del sistema de monitorización térmica y la temperatura actual
        display_temp_ctl();
      break;
      case 90: // Modo 6: Abrir compuerta y activar motor
	      cold_pwr_on();
        display_cold_ctl();
      break;
      case 66: // Modo 7: Cerrar compuerta y desactivar motor
	      cold_pwr_off();
        display_pwr_off_cold();
      break;
      case 74: // Modo 8: Encender calentador
	      warm_pwr_on();
        display_warm_ctl();
      break;
      case 82: // Modo 9: Apagar calentador
	      warm_pwr_off();
        display_pwr_off_warm();
      break;
      default:
      break;
    }
}

