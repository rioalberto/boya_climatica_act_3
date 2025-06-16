# Mejora de Estación Climática Marina Autónoma

Este proyecto supone una mejora de la "Estación Climática Marina Autónoma" que puede ser encontrada en el siguiente [enlace](https://github.com/rioalberto/boya_climatica).

Este proyecto ha sido simulado empleando WOKWI. Este es el [enlace](https://wokwi.com/projects/433867442602833921) a dicha simulación.

---

## Hardware Adicional Requerido

Como sabemos, la boya está controlada por **Arduino Uno** y se le han añadido los siguientes elementos:

- Un **mando a distancia de luz infrarroja** que opera a 38KHz y dispone de 20 teclas.
- Un **receptor de infrarrojos** de 38KHz para recibir los estímulos provocados por el mando.
  
---

## Esquema de Conexiones Actualizado

![schematic](https://github.com/rioalberto/boya_climatica_act_3/blob/main/Schematic_act_3.png)


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
|    Celeste       | 2              | IR Receiver                      | Recepción pulsos infrarrojos del mando       |


---

## Gestión de modos del sistema con mando IR

El código gestiona distintos modos de funcionamiento para la boya climática empleando un mando a distancia infrarrojo para seleccionar distintas operaciones. A continuación se explican los estados posibles, divididos según cómo se gestionan:

### 1. Estados de visualización del sistema (modo test)

Estos se gestionan dentro de la función `test_mode()`, hacen refrencia a los estados de funcionamiento del proyecto anterior que alternan pulsando un botón. Se basan en el valor de la variable `mode`, que puede tomar cinco valores (0 a 4), cada uno asociado a una función de visualización en una pantalla LCD:

| `mode` | Estado                                      | Acción                   |
|--------|---------------------------------------------|--------------------------|
| 0      | Fecha y hora                                | `display_date_hour()`    |
| 1      | Luminiscencia                               | `display_lux()`          |
| 2      | Humedad y temperatura                       | `display_dht22()`        |
| 3      | Calidad del aire (limpio/contaminado)       | `display_MQ2()`          |
| 4      | Estado del sistema térmico + temperatura    | `display_temp_ctl()`     |

### 2. Estados por botón IR en `IR_mode_one_time()`

Este modo se ejecuta solo una vez por pulsación. Los botones cambian el estado actual (`estadoActual`) o modifican parámetros. Aquí tienes un resumen:

#### Encendido/Apagado general

| Botón IR (valor) | Estado         | Acción                       |
|------------------|----------------|------------------------------|
| 162  - Power     | Toggle ON/OFF  | `display_welcome()` / `display_goodbay()` |

#### Modificación de parámetros térmicos

| Botón IR | Acción                    | Funciones involucradas                  |
|----------|---------------------------|------------------------------------------|
| 144 - Flecha siguiente     | Subir límite de temperatura | `mod_temp_lim()` + `display_temp_lim_margin()` |
| 224 - Flecha anterior    | Bajar límite de temperatura | Idem                                     |
| 2 - Símbolo más        | Subir margen               | `mod_temp_margin()` + `display_temp_lim_margin()` |
| 152  - Símbolo menos    | Bajar margen               | Idem                                     |

#### Cambio de modo o visualización  
(Se cambian `estadoAnterior` y `estadoActual`)

| Botón IR | Modo LCD mostrado                                  |
|----------|-----------------------------------------------------|
| 48 - Botón 1      | Modo 1: Fecha y hora                                |
| 24 - Botón 2      | Modo 2: Luminiscencia                               |
| 122 - Botón 3     | Modo 3: Humedad y temperatura                       |
| 16 - Botón 4      | Modo 4: Calidad del aire                            |
| 56 - Botón 5      | Modo 5: Estado del sistema térmico                  |
| 90 - Botón 6     | Modo 6: Abrir compuerta y activar motor             |
| 66 - Botón 7      | Modo 7: Cerrar compuerta y desactivar motor         |
| 74 - Botón 8      | Modo 8: Encender calentador                         |
| 82 - Botón 9     | Modo 9: Apagar calentador                           |

### 3. Estados por botón IR en `IR_mode_continuo()`

Este modo se ejecuta continuamente mientras se mantenga activo. Usa los mismos botones IR, pero ejecuta directamente las funciones sin cambiar `estadoActual`. Sirve más como modo directo de control.

| Botón IR | Estado                                | Acción                               |
|----------|----------------------------------------|--------------------------------------|
| 34 - Test      | Modo test                              | `test_mode()`                        |
| 48 - Botón 1      | Fecha y hora                           | `display_date_hour()`                |
| 24 - Botón 2      | Luminiscencia                          | `display_lux()`                      |
| 122 - Botón 3     | Humedad y temperatura                  | `display_dht22()`                    |
| 16 - Botón 4      | Aire limpio/contaminado                | `display_MQ2()`                      |
| 56 - Botón 5      | Estado sistema térmico                 | `display_temp_ctl()`                 |
| 90 - Botón 6      | Abrir compuerta, activar motor         | `cold_pwr_on()` + `display_cold_ctl()` |
| 66 - Botón 7      | Cerrar compuerta, desactivar motor     | `cold_pwr_off()` + `display_pwr_off_cold()` |
| 74 - Botón 8      | Encender calentador                    | `warm_pwr_on()` + `display_warm_ctl()` |
| 82 - Botón 9      | Apagar calentador                      | `warm_pwr_off()` + `display_pwr_off_warm()` |

### 4. Estados internos del sistema

Variables que determinan el comportamiento del sistema:

- `mode`: controla qué se muestra en `test_mode()`.
- `button_value`: valor del último botón IR recibido.
- `estadoActual`: estado lógico actual del sistema, usado para mantener contexto entre botones.
- `estadoAnterior`: estado anterior, útil para retroceso o recuperación.
- `pwr_on_flag`: indica si el sistema está encendido o apagado.

---

### Conclusión

Las mejoras permiten alternar con control remoto entre:

- Un **modo test cíclico**
- Un **modo continuo directo**
- Un **modo basado en eventos IR** que actualiza el estado una sola vez por pulsación.

Todo esto proporciona una interfaz robusta para control remoto mediante un mando IR.


---

## Cómo Usarlo

1. Conecta todos los sensores y actuadores al microcontrolador como se indica en las definiciones de pines (`#define ...`).
2. Carga el código en el microcontrolador desde el IDE de Arduino.
3. Al iniciar, la boya se encontrará en modo standby, enciende el sistema con el botón del mando y alterna entre modos con él.

---

## Vídeos de Pruebas de Funcionamiento

Puedes encontrar el vídeo demostrativo del sistema en acción en el repositorio principal del proyecto bajo el nombre de "Video_funcionamiento_pr_3".

---

## Licencia

Este proyecto es de código abierto y se publica bajo la **MIT License**.
