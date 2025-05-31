## Explicación
Aqui se explicará el código linea por linea:
### Importación de Servo.h:
Aqui es importada la librería Servo.h para el funcionamiento del servomotor. También se define el nombre que se utilizará para emplear la función en el código
```cpp
#include <Servo.h>  // Librería estándar Servo

Servo servo;  // Servo estándar
```
### Definición de pines
Aqui se guarda en constantes el numero de pin al que se conecta cada sensor (el del bluetooth no se define porque utiliza el 0 (RX) y el 1 (TX), pines especiales que no lo necesitan).
```cpp
const int sensorPin1 = 12;
const int sensorPin2 = 2;
const int sensorPin3 = 3;
const int buzzerPin = A5;
const int servoPin = 10;
const int ledPin = 8;
const int aguaPin = 7;
```
### Definición de variables
Aqui se definen las 3 variables principales binarias. Por defecto valen 0 (apagadas)
```cpp
int variable1 = 0;
int variable2 = 0;
int variable3 = 0;
```
### Definición de flags
Después se crean las flags, variables booleanas (verdadero o falso) que nos ayudan a dejar constancia de la activación de funciones
```cpp
bool variable1Activable = false;
bool variable2Activable = false;
bool parpadeoCajaRealizado = false;
bool parpadeoCierreRealizado = false;
bool temporizadorActivo1 = false;
bool temporizadorActivo2 = false;
bool alarmaAguaActivada = false;

```
### Otras variables
Aqui se crean las variables de los temporizadores y del monitor de variables, además de un par más de flags
```cpp
unsigned long tiempoActivacion1 = 0;
unsigned long tiempoActivacion2 = 0;
const unsigned long TIEMPO_ESPERA = 2UL * 30UL * 1000UL;

unsigned long ultimoPrint = 0;
const unsigned long intervaloPrint = 1000;

bool alarmaActivada = false;
bool cajaAbierta = false;
```
### abrirCaja()
Despues se crean la primera función, abrirCaja(). Lo que hace es activar la flag y girar el motor.
```cpp
void abrirCaja() {
  cajaAbierta = true;
  servo.write(180);  // Abrir al máximo
}
```
### cerrarCaja()
Aqui se crea la función cerrarCaja(), que reinicia las variables y las flags y gira el motor
```cpp
void cerrarCaja() {
  variable1 = 0;
  variable2 = 0;
  cajaAbierta = false;
  temporizadorActivo1 = false;
  temporizadorActivo2 = false;
  parpadeoCajaRealizado = false;
  parpadeoCierreRealizado = false;
  servo.write(90);  // Posición cerrada (media posición)
}
```
### setup()
Aqui está la función setup, una función que por defecto se activa al iniciarse o reiniciarse la placa. Aqui se definen los pines de los sensores como INPUT, y el altavoz y la luz como OUTPUT, ya que reciben ordenea. También se pone el motor en su posición por defecto, pero esto se hace por comodidad al testear. Además se inicia la comunicación Serial a 9600 baudios para el bluetooth
```cpp
void setup() {
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(aguaPin, INPUT_PULLUP);

  servo.attach(servoPin);
  servo.write(90);  // Posición cerrada inicial

  Serial.begin(9600);
}
```
Estas son las funciones de ejecutar sirena, es un bucle que con el tiempo y unas variables hace el tono necesario.
```cpp
void ejecutarSirena() {
  static int freq = 1000;
  static bool subiendo = true;
  static unsigned long ultimaActualizacion = 0;
  const int incremento = 30;
  const unsigned long intervalo = 4;

  if (millis() - ultimaActualizacion >= intervalo) {
    ultimaActualizacion = millis();
    tone(buzzerPin, freq);
    if (subiendo) {
      freq += incremento;
      if (freq >= 4000) subiendo = false;
    } else {
      freq -= incremento;
      if (freq <= 1000) subiendo = true;
    }
  }
}

void ejecutarSirenaAgua() {
  static unsigned long ultimaNota = 0;
  static bool estado = false;
  const unsigned long duracionNota = 300;

  if (millis() - ultimaNota >= duracionNota) {
    ultimaNota = millis();
    estado = !estado;

    if (estado) {
      tone(buzzerPin, 1000);
    } else {
      noTone(buzzerPin);
    }
  }
}
```
Ahora empieza la funcion más importante, el loop(). Está es una función predeterminada que se ejecuta en bucle esperando condiciones y actualizaciones. Al incio, recoge el estado de los 43 sensores (agua, luz, y magnetismo). El estadoSensor2 está para pruebas con otros sensores si es necesario pero no se utiliza.
```cpp
  int estadoSensor1 = digitalRead(sensorPin1);
  int estadoSensor2 = digitalRead(sensorPin2);
  int estadoSensor3 = digitalRead(sensorPin3);
  int estadoSensor4 = digitalRead(aguaPin);
```
Después, se realiza el encendido y apagado del LED al cerrase o abrirse la caja (3 parpadeos), y al sonar la alarma (hasta que se apage).
```cpp
  if (cajaAbierta && !parpadeoCajaRealizado) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH); delay(100);
      digitalWrite(ledPin, LOW); delay(100);
    }
    parpadeoCajaRealizado = true;
  } else if (!cajaAbierta && !parpadeoCierreRealizado) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH); delay(100);
      digitalWrite(ledPin, LOW); delay(100);
    }
    parpadeoCierreRealizado = true;
  }

  if (alarmaActivada || alarmaAguaActivada) {
    static unsigned long ultimoCambio = 0;
    static bool estadoLED = false;
    if (millis() - ultimoCambio >= 100) {
      ultimoCambio = millis();
      estadoLED = !estadoLED;
      digitalWrite(ledPin, estadoLED ? HIGH : LOW);
    }
  } else {
    digitalWrite(ledPin, LOW);
  }
```
Aqui se realiza la activación de la alarma. En el if (alarmaActivada) se da preferencia a la alarma de robo por encima de la de agua. Además, arriba se activan las flags si el sensor detecta algo.
```cpp
  if (!alarmaActivada && estadoSensor3 == HIGH && variable3 == 0) {
    alarmaActivada = true;
  }

  bool alarmaAguaAnterior = alarmaAguaActivada;
  alarmaAguaActivada = (estadoSensor4 == LOW);

  if (alarmaActivada) {
    ejecutarSirena();
  } else if (alarmaAguaActivada) {
    ejecutarSirenaAgua();
  } else {
    noTone(buzzerPin);
  }

  if (alarmaAguaAnterior && !alarmaAguaActivada) {
    servo.detach();
    delay(10);
    servo.attach(servoPin);
    if (cajaAbierta) {
      servo.write(180);
    } else {
      servo.write(90);
    }
  }
```
Aqui se define la activación de la variable1, el sensor de linea. El primer condicional hace que solo el sensor detecte algo si las alarmas no están activada, después mira el estado del sensor y si está encendido algo hace la variable1 activable. Depues revisa si el sensor a detectado algo, y si la variable no estaba ya activada la activa e incia el contador de 2 minutos (si la variable 3 no está activada), que si se acaba la variable vuelve a 0.
```cpp
  if (!alarmaActivada && !alarmaAguaActivada) {
    if (estadoSensor1 == HIGH) {
      variable1Activable = true;
    }
    if (estadoSensor1 == LOW && variable1 == 0 && variable1Activable) {
      variable1 = 1;
      variable1Activable = false;
      if (variable2 == 1) variable3 = 1;
      if (variable3 == 0) {
        tiempoActivacion1 = millis();
        temporizadorActivo1 = true;
      } else {
        temporizadorActivo1 = false;
      }
    }
  } else {
    variable1Activable = false;
  }
```
Aqui se define la activación de la variable2. En el primer condicional revisa si el sensor está activo y espera un mensaje, que si existe hace la variable2 activable y lo reenvia para comprobar que ha llegado bien. Después, si la variable2 es activable y el mensaje existe, Elije el mensaje que responder:
* Si la alarma está activada y recibe el código, la desactiva y responde "Alarma desactivada"
* Si la alarma está activada y el codigo que recibe es incorrecto, responde "Alarma activada, codigo incorrecto."
* Si la alarma de agua esta activada, responde "Alarma de agua activada, mire si la caja está mojada o llévela a reparación."
* Si la variable2 (el bluetooth) y está activada y la caja está cerrada, responde "Ya se habia introducido el codigo, introduzca la llave."
* Si la variable2 está activada, la caja abierta y el codigo es distinto del codigo de cierre de la caja, responde "Ya se habia introducido el codigo, la caja esta abierta."
* Si las variables 1, 2 y 3 están desactivadas y le llega el código correcto, responde "Codigo correcto, introduzca la llave.", cambia variable2 = 1 y activa el temporizador2.
* Si la caja está abierta y el recibe la contraseña de cierre, responde "Cerrando caja...", ejecuta cerrarCaja() y pone variable3 a 0
* Si el mensaje es el código, estando la caja cerrada, la variable2 en 0 y la 1 en 1, responde "Caja abierta", cambia variable 2 y 3 a 1 y desactiva el temporizador2.
* Si el mensaje es incorrecto, responde "Código incorrecto"
* Si no se cumple ninguna condición, responde "Error"
Al final vuelve a reiniciar el mensaje a "".
```cpp
  if (Serial.available()) {
    mensajeBT = Serial.readStringUntil('\n');
    mensajeBT.trim();
    if (mensajeBT != "") {
      variable2Activable = true;
      Serial.println("Codigo recibido: " + mensajeBT);
      delay(10);
    }
  }

  if (variable2Activable && mensajeBT != "") {
    if (alarmaActivada) {
      if (mensajeBT == passwordBT) {
        alarmaActivada = false;
        Serial.println("Alarma desactivada.");
        delay(10);
        variable2 = 0; variable3 = 0;
        temporizadorActivo1 = false; temporizadorActivo2 = false;
      } else {
        Serial.println("Alarma activada, codigo incorrecto.");
        delay(10);
      }
    } else if (alarmaAguaActivada) {
      Serial.println("Alarma de agua activada, mire si la caja está mojada o llévela a reparación.");
      delay(10);
    } else {
      if (variable2 == 1 && variable3 == 0 && !cajaAbierta) {
        Serial.println("Ya se habia introducido el codigo, introduzca la llave.");
        delay(10);
      } else if (variable2 == 1 && variable3 == 1 && cajaAbierta && mensajeBT != passwordCerrarBT) {
        Serial.println("Ya se habia introducido el codigo, la caja esta abierta.");
        delay(10);
      } else if (mensajeBT == passwordBT && variable3 == 0 && variable2 == 0 && !cajaAbierta && variable1 == 0) {
        Serial.println("Codigo correcto, introduzca la llave.");
        delay(10);
        variable2 = 1;
        tiempoActivacion2 = millis();
        temporizadorActivo2 = true;
      } else if (mensajeBT == passwordCerrarBT && variable3 == 1 && cajaAbierta) {
        variable3 = 0;
        Serial.println("Cerrando caja...");
        delay(10);
      } else if (mensajeBT == passwordBT && variable3 == 0 && variable2 == 0 && !cajaAbierta && variable1 == 1) {
        Serial.println("Caja abierta.");
        delay(10);
        variable2 = 1; variable3 = 1;
        temporizadorActivo2 = false;
      } else if (mensajeBT != passwordBT) {
        Serial.println("Codigo incorrecto.");
        delay(10);
      } else {
        Serial.println("Error.");
        delay(10);
      }
    }
    mensajeBT = "";
  }
```
Después empieza la lógica de contadores. Sería mas sencillo hacerla con delays pero utiliza millis() con un bucle para no parar la función loop. Si pasan 2 minutos y no se ha activado la otra variable, la reinicia.
```cpp
  if (temporizadorActivo1 && millis() - tiempoActivacion1 > TIEMPO_ESPERA) {
    variable1 = 0;
    temporizadorActivo1 = false;
  }

  if (temporizadorActivo2) {
    if (variable1 == 1) {
      temporizadorActivo2 = false;
    } else if (millis() - tiempoActivacion2 > TIEMPO_ESPERA) {
      variable2 = 0;
      temporizadorActivo2 = false;
    }
  }
```
Esta parte abre la caja y apaga los contadores si variable3 pasa a 1.
```cpp
  if (variable3 == 1 && !cajaAbierta) {
    abrirCaja();
    temporizadorActivo1 = false;
    temporizadorActivo2 = false;
  } else if (variable3 == 0 && cajaAbierta) {
    cerrarCaja();
  }
```
Esta parte es del monitor en tiempo real para el desarrollo.
```cpp
  if (millis() - ultimoPrint >= intervaloPrint) {
    //printStatus();
    ultimoPrint = millis();
  }
```