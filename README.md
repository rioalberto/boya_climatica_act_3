# Mejora de Estación Climática Marina Autónoma

Este proyecto supone una mejora de la "Estación Climática Marina Autónoma" que puede ser encontrada en el siguiente [enlace](https://github.com/rioalberto/boya_climatica).

Este proyecto ha sido simulado empleando WOKWI. Este es el [enlace](https://wokwi.com/projects/433867442602833921) a dicha simulación.

---

## Hardware Adicional Requerido

Como sabemos, la boya está controlada por **Arduino Uno** y se le han añadido los siguientes elementos:

- Un **mando a distancia de luz infrarroja** que opera a 38KHz y dispone de 20 teclas.
- Un **receptor de infrarrojos** de 38KHz para recibir los estímulos provocados por el mando.
  
---

## Esquema de Conexiones

![schematic]()


| Color del cable  | Pin en Arduino | Conectado a                      | Función                                      |
|------------------|----------------|----------------------------------|----------------------------------------------|
|    Rojo          | 5V             | DHT22, Pulsador, ULN2003, MQ-2, Pantalla LCD I2C, RTC DS1307 | Alimentación (VCC)                          |
|    Negro         | GND            | Todos los módulos (común)        | Tierra (GND)                                 |
|    Blanco        | 12             | DHT22                            | Señal de temperatura/humedad                 |
|    Amarillo      | 13             | Pulsador                         | Entrada de botón                             |
|    Naranja       | A0             | LDR                              | Sensor LDR                                   |
|    Gris          | 4              | FIT0845                          | Calentador                                   |
|    Verde claro   | 8              | Motor A+                         | Motor paso a paso                            |
|    Rosa          | 9              | Motor A-                         | Motor paso a paso                            |
|    Morado        | 10             | Motor B+                         | Motor paso a paso                            |
|    Marrón        | 11             | Motor B-                         | Motor paso a paso                            |
|    Verde         | 3              | Servo motor                      | PWM de control                               |
|    Violeta       | 7              | MQ-2                             | Lectura analógica del gas                    |
|    Verde         | A4 (SDA)       | RTC DS1307, Pantalla LCD I2C     | Comunicación I2C – datos                     |
|    Azul          | A5 (SCL)       | RTC DS1307, Pantalla LCD I2C     | Comunicación I2C – reloj                     |


---

## Funcionalidad del Sistema

### Sensorial

Lectura cada segundo de:

- Temperatura y humedad (DHT22)
- Luminiscencia (LDR)
- Calidad del aire (MQ-2)
- Fecha y hora (RTC)

### Control Térmico

- Si la **temperatura < 22 °C**:
  - Se activa el calentador
  - Se simula un aumento de temperatura de **+0.04 °C/s**
- Si la **temperatura > 28 °C**:
  - Se abre la compuerta con el servo
  - Se activa la turbina para distribuir agua marina (~20 °C)
  - Se simula un enfriamiento de **–0.28 °C/s**
- Si está **dentro del rango**:
  - Se apaga el calentador
  - Se cierra la compuerta
  - Se detiene la turbina

### Cálculos térmicos

Estos cálculos permiten estimar los efectos térmicos del calentador y del sistema de enfriamiento por convección en una masa de referencia de **200 g (0.2 kg)**, como podría ser una batería o compartimento.

#### 1. Estimación del calentamiento

**Supuestos:**

- Masa afectada:  
  `m = 0.2 kg`

- Calor específico (ej. batería de litio):  
  `c = 900 J/(kg·°C)`

- Potencia del PTC (valor práctico):  
  `Q = 8 J/s = 8 W`

**Fórmula aplicada:**

`ΔT = Q / (m · c) = 8 / (0.2 × 900) = 8 / 180 ≈ 0.044 °C/s`

**Resultado:**  
En condiciones ideales, el sistema podría incrementar la temperatura en **~0.044 °C por segundo**.

*En la práctica, este valor será menor por pérdidas térmicas. Además, el PTC se autorregula y limita su potencia conforme sube la temperatura, protegiendo el sistema de sobrecalentamientos.*

---

#### 2. Estimación del enfriamiento

Se modela como una **convección forzada**, al abrir la compuerta e introducir agua marina más fría con ayuda de una turbina.

##### Paso 1: Cálculo del flujo de calor

**Fórmula:**

`Q = h · A · ΔT`

**Parámetros usados:**

- Coeficiente de convección:  
  `h = 500 W/(m²·°C)`  
  *(valor razonable para agua con flujo moderado)*

- Área de intercambio térmico:  
  `A = 0.01 m²` *(10 cm × 10 cm)*

- Diferencia de temperatura:  
  `ΔT = 10 °C` *(Ejemplo: batería a 30 °C, agua a 20 °C)*

**Cálculo:**

`Q = 500 × 0.01 × 10 = 50 W`

##### Paso 2: Cálculo del descenso térmico

**Fórmula de calorimetría:**

`ΔT = Q / (m · c) = 50 / (0.2 × 900) = 50 / 180 ≈ 0.278 °C/s`

**Resultado:**  
La temperatura descendería idealmente **~0.28 °C por segundo** al enfriar la carcasa con agua.

*Este valor también depende del flujo, turbulencia, conductividad de los materiales, etc.*

---

#### Conclusión

- **Calentamiento simulado:**  
  `ΔT_calor ≈ +0.04 °C/s` *(valor ajustado en software)*

- **Enfriamiento simulado:**  
  `ΔT_frío ≈ –0.28 °C/s`

Estas tasas son implementadas en el software del Arduino para simular la evolución térmica del sistema, de forma que el comportamiento general sea realista en un entorno controlado.

### Visualización en LCD (controlado por botón):

- Fecha y hora actual
- Nivel de luminiscencia (lux)
- Temperatura y humedad
- Estado del aire (contaminado/limpio)
- Estado térmico y temperatura actual

---

## Estructura del Código

El código está organizado en varias funciones que separan claramente:

- La adquisición de datos
- La lógica de control térmico
- La visualización en pantalla
- El manejo de eventos como el pulsador

Este código está diseñado para controlar una boya inteligente de monitoreo ambiental, integrando sensores de temperatura, humedad, gas, luz, y actuadores como un servomotor, una turbina y un calentador, todo gestionado desde un microcontrolador (tipo Arduino). A continuación, te explico detalladamente cada parte del código:

### 1. Inclusión de librerías

```ino
#include <LiquidCrystal_I2C.h> 
#include <DHT.h>
#include "RTClib.h"
#include <Servo.h>
#include <Stepper.h>
```
Estas librerías permiten usar:
  - Pantalla LCD con interfaz I2C.
  - Sensor de temperatura y humedad DHT22.
  - Reloj de tiempo real (RTC).
  - Servo (para abrir compuerta).
  - Motor paso a paso (turbina de ventilación).

### 2. Definición de pines y constantes

```ino
#define LDRPIN A0
#define DHTPIN 12
#define MQ2PIN 7
...
```
Aquí se definen los pines para los sensores y actuadores:
  - LDRPIN: resistencia fotosensible (luz).
  - DHTPIN: sensor DHT22.
  - MQ2PIN: sensor de gases.
  - CALENTADORPIN: para activar calentador.
  - SERVOPIN: servo que abre una compuerta para ventilar.
  - Pines del motor paso a paso (MOTORPIN1 a MOTORPIN4).

También se definen:
  - Constantes para el sensor de luz (LDR).
  - Umbrales de temperatura.
  - Velocidades de calentamiento/enfriamiento simuladas.

### 3. Variables globales

Se almacenan datos actuales y anteriores del LCD, modo de operación, medidas, etc. Por ejemplo:
```ino
float temperature, humidity, lux;
bool acquisition = false;
uint8_t mode = 0;
...
```

### 4. Objetos

Se instancian los objetos de cada sensor/actuador:
```ino
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;
Servo compuerta;
Stepper turbina(stepsPerRevolution, MOTORPIN1, MOTORPIN2, MOTORPIN3, MOTORPIN4);
```

### 5. setup() – Inicialización

```ino
void setup() {
  pinMode(LDRPIN, INPUT);
  ...
  compuerta.write(90); // Compuerta cerrada
  ...
  lcd.init(); lcd.backlight();
}
```

- Se configuran los pines.
- Se inicializan sensores y actuadores.
- El servo inicia en posición cerrada.
- Se establece la velocidad del motor.

### 6. loop() – Ciclo principal

#### a. Temporizador:

```ino
acquisition_int();
if (acquisition) { ... }
```
Cada 1 segundo se activa la bandera acquisition.

#### b. Sensores y control:

```ino
read_lux(); read_rtc(); read_mq2(); temp_control();
```
Se leen valores de sensores y se ejecuta control de temperatura.

#### c. Modo de visualización:

```ino
switch (mode) {
  case 0: display_date_hour(); break;
  ...
}
```
El usuario cambia el contenido del LCD con un botón.

#### d. Motor:

```ino
if (motor_flag){
  turbina.step(stepsPerRevolution);
}
```
Activa la turbina cuando el sistema necesita enfriar.

### 7. acquisition_int()

```ino
if (currentMillis - previousMillis >= interval) {
  acquisition = true;
}
```
Simula una interrupción temporizada usando millis() para ejecutar lectura cada segundo.

### 8. Funciones de sensores

- read_dht(): lee humedad y temperatura.

- read_lux(): convierte la lectura analógica del LDR a lux.

- read_rtc(): obtiene fecha y hora del RTC.

- read_mq2(): lee el valor digital del sensor MQ2.

### 9. temp_control() – Control térmico

```ino
if (temperature > limiteSuperior) {
  compuerta.write(0); // abrir
  motor_flag = true;
  ...
} else if (temperature < limiteInferior) {
  digitalWrite(CALENTADORPIN, HIGH);
  ...
}
```
Controla si se debe calentar (enciende calentador) o enfriar (abre compuerta + turbina). La temperatura se ajusta simulando el cambio térmico.

### 10. Display en LCD (clear_lcd + funciones display_*)

```ino
display_date_hour();
display_lux();
display_dht22();
display_MQ2();
display_temp_ctl();
```
Cada una muestra distinta información en función del modo actual (fecha, lux, Tº/Hº, calidad de aire...). La función clear_lcd evita borrar la pantalla si los valores no han cambiado.

---

## Cómo Usarlo

1. Conecta todos los sensores y actuadores al microcontrolador como se indica en las definiciones de pines (`#define ...`).
2. Carga el código en el microcontrolador desde el IDE de Arduino.
3. Al iniciar, la boya se encontrará en modo de visualización 0 (fecha y hora).
4. Usa el botón para alternar entre los **5 modos de visualización**.

---

## Notas Técnicas

- El valor de temperatura es actualizado con lógica de simulación térmica dentro del código:  
  `temperature += cambioFrio * dt` o `cambioCalor * dt`
- Se asume que el sensor MQ-2 está configurado en modo digital:  
  `0 = limpio`, `1 = contaminado`
- La simulación **ignora la dinámica real del entorno marino** y sirve para pruebas en laboratorio o demostraciones educativas.

---

## Posibles Mejoras

- Usar sensores analógicos con calibración real (MQ-2 en modo analógico)
- Integrar energía solar para operación prolongada
- Medir temperatura del agua marina directamente

---

## Vídeos de Pruebas de Funcionamiento

Puedes encontrar el vídeo demostrativo del sistema en acción en el repositorio principal del proyecto bajo el nombre de "Video funcionamiento_pr_2".

---

## Licencia

Este proyecto es de código abierto y se publica bajo la **MIT License**.
