### Código:
```cpp
#include <SoftwareSerial.h>
#include <Servo.h>

#define BTRX 4
#define BTTX 5

Servo servo;
SoftwareSerial BT(BTRX, BTTX);

String mensajeBT = "";
String passwordBT = "0101";
String passwordCerrarBT = "Cerrar caja 0101";

const int sensorPin1 = 12;
const int sensorPin2 = 2;
const int sensorPin3 = 3;
const int buzzerPin = A5;
const int servoPin = 11;
const int ledPin = 10;

int variable1 = 0;
int variable2 = 0;
int variable3 = 0;

bool variable1Activable = false;
bool variable2Activable = false;
bool parpadeoCajaRealizado = false;
bool temporizadorActivo1 = false;
bool temporizadorActivo2 = false;

unsigned long tiempoActivacion1 = 0;
unsigned long tiempoActivacion2 = 0;
const unsigned long TIEMPO_ESPERA = 2UL * 30UL * 1000UL;

unsigned long ultimoPrint = 0;
const unsigned long intervaloPrint = 1000;

bool alarmaActivada = false;
bool cajaAbierta = false;

void abrirCaja() {
  cajaAbierta = true;
  servo.attach(servoPin);
  servo.writeMicroseconds(1400);
  delay(310);
  servo.writeMicroseconds(1500);
  servo.detach();
}

void cerrarCaja() {
  variable1 = 0;
  variable2 = 0;
  cajaAbierta = false;
  temporizadorActivo1 = false;
  temporizadorActivo2 = false;
  parpadeoCajaRealizado = false;

  servo.attach(servoPin);
  servo.writeMicroseconds(1600);
  delay(310);
  servo.writeMicroseconds(1500);
  servo.detach();
}

void setup() {
  pinMode(sensorPin1, INPUT);
  pinMode(sensorPin2, INPUT);
  pinMode(sensorPin3, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);
  BT.begin(9600);
  BT.setTimeout(200);
}

void printStatus() {
  Serial.print("variable1: ");
  Serial.print(variable1);
  Serial.print(" | variable2: ");
  Serial.print(variable2);
  Serial.print(" | variable3: ");
  Serial.print(variable3);
  Serial.print(" | cajaAbierta: ");
  Serial.print(cajaAbierta ? "true" : "false");
  Serial.print(" | temporizadorActivo1: ");
  Serial.print(temporizadorActivo1 ? "true" : "false");
  Serial.print(" | tiempoTranscurrido1(ms): ");
  Serial.print(temporizadorActivo1 ? (millis() - tiempoActivacion1) : 0);
  
  Serial.print(" | temporizadorActivo2: ");
  Serial.print(temporizadorActivo2 ? "true" : "false");
  Serial.print(" | tiempoTranscurrido2(ms): ");
  Serial.println(temporizadorActivo2 ? (millis() - tiempoActivacion2) : 0);
}

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

void loop() {
  int estadoSensor1 = digitalRead(sensorPin1);
  int estadoSensor2 = digitalRead(sensorPin2);
  int estadoSensor3 = digitalRead(sensorPin3);

  // LED
  if (cajaAbierta && !parpadeoCajaRealizado) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
    parpadeoCajaRealizado = true;
  }
  else if (alarmaActivada) {
    static unsigned long ultimoCambio = 0;
    static bool estadoLED = false;
    const unsigned long intervalo = 100;

    if (millis() - ultimoCambio >= intervalo) {
      ultimoCambio = millis();
      estadoLED = !estadoLED;
      digitalWrite(ledPin, estadoLED ? HIGH : LOW);
    }
  }
  else {
    digitalWrite(ledPin, LOW);
  }

  // ALARMA ACTIVACIÓN
  if (!alarmaActivada && estadoSensor3 == LOW && variable3 == 0) {
    alarmaActivada = true;
  }

  if (alarmaActivada) {
    ejecutarSirena();
  } else {
    noTone(buzzerPin);
  }

  // VARIABLE 1
  if (!alarmaActivada) {
    if (estadoSensor1 == HIGH) {
      variable1Activable = true;
    }

    if (estadoSensor1 == LOW && variable1 == 0 && variable1Activable) {
      variable1 = 1;
      variable1Activable = false;

      if (variable2 == 1) {
        variable3 = 1;
      }

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

  // VARIABLE 2
  if (BT.available()) {
    mensajeBT = BT.readStringUntil('\n'); 
    mensajeBT.trim();
    if (mensajeBT != "") {
      variable2Activable = true;
      BT.print("Codigo recibido: ");
      BT.println(mensajeBT);
    }
  }

  if (variable2Activable && mensajeBT != "") {
    if (alarmaActivada) {
      if (mensajeBT == passwordBT) {
        alarmaActivada = false;
        BT.println("Alarma desactivada.");
        variable2 = 0;
        variable3 = 0;
        temporizadorActivo1 = false;
        temporizadorActivo2 = false;
      } else {
        BT.println("Alarma activada, código incorrecto.");
      }
    } else {
      if (variable2 == 1 && variable3 == 0 && !cajaAbierta) {
        BT.println("Ya se habia introducido el codigo, introduzca la llave.");
      } else if (variable2 == 1 && variable3 == 1 && cajaAbierta && mensajeBT != passwordCerrarBT) {
        BT.println("Ya se habia introducido el codigo, la caja esta abierta.");
      } else if (mensajeBT == passwordBT && variable3 == 0 && variable2 == 0 && !cajaAbierta && variable1 == 0) {
        BT.println("Codigo correcto, introduzca la llave.");
        variable2 = 1;

        tiempoActivacion2 = millis();
        temporizadorActivo2 = true;
      } else if (mensajeBT == passwordCerrarBT && variable3 == 1 && cajaAbierta) {
        variable3 = 0;
        BT.println("Cerrando caja...");
      } else if (mensajeBT == passwordBT && variable3 == 0 && variable2 == 0 && !cajaAbierta && variable1 == 1) {
        BT.println("Caja abierta.");
        variable2 = 1;
        variable3 = 1;
        temporizadorActivo2 = false;
      } else if (mensajeBT != passwordBT) {
        BT.println("Codigo incorrecto.");
      } else {
        BT.println("Error.");
      }
    }
    mensajeBT = "";
  }

  // TEMPORIZADOR VARIABLE 1
  if (temporizadorActivo1) {
    if (millis() - tiempoActivacion1 > TIEMPO_ESPERA) {
      variable1 = 0;
      temporizadorActivo1 = false;
    }
  }

  // TEMPORIZADOR VARIABLE 2
  if (temporizadorActivo2) {
    if (variable1 == 1) {
      temporizadorActivo2 = false;
    } else if (millis() - tiempoActivacion2 > TIEMPO_ESPERA) {
      variable2 = 0;
      temporizadorActivo2 = false;
    }
  }

  // abrirCaja() y cerrarCaja()
  if (variable3 == 1 && !cajaAbierta) {
    abrirCaja();
    temporizadorActivo1 = false;
    temporizadorActivo2 = false;
  } else if (variable3 == 0 && cajaAbierta) {
    cerrarCaja();
  }

  // SERIAL MONITOR
  if (millis() - ultimoPrint >= intervaloPrint) {
    printStatus();
    ultimoPrint = millis();
  }
}

```