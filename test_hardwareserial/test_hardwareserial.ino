// HARDWARE SERIAL
#include <Servo.h>  // Librería estándar Servo

Servo servo;  // Servo estándar

String mensajeBT = "";
String passwordBT = "0101";
String passwordCerrarBT = "Cerrar caja 0101";

const int sensorPin1 = 12;
const int sensorPin2 = 2;
const int sensorPin3 = 3;
const int buzzerPin = A5;
const int servoPin = 10;
const int ledPin = 8;
const int aguaPin = 7;

int variable1 = 0;
int variable2 = 0;
int variable3 = 0;

bool variable1Activable = false;
bool variable2Activable = false;
bool parpadeoCajaRealizado = false;
bool parpadeoCierreRealizado = false;
bool temporizadorActivo1 = false;
bool temporizadorActivo2 = false;
bool alarmaAguaActivada = false;

unsigned long tiempoActivacion1 = 0;
unsigned long tiempoActivacion2 = 0;
const unsigned long TIEMPO_ESPERA = 2UL * 30UL * 1000UL;

unsigned long ultimoPrint = 0;
const unsigned long intervaloPrint = 1000;

bool alarmaActivada = false;
bool cajaAbierta = false;

void abrirCaja() {
  cajaAbierta = true;
  // servo estándar usa grados 0-180, 2400 es valor para ServoTimer2, acá usamos grados
  servo.write(180);  // Abrir al máximo
}

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

/*void printStatus() {
  Serial.print("variable1: "); Serial.print(variable1);
  Serial.print(" | variable2: "); Serial.print(variable2);
  Serial.print(" | variable3: "); Serial.print(variable3);
  Serial.print(" | cajaAbierta: "); Serial.print(cajaAbierta ? "true" : "false");
  Serial.print(" | alarmaActivada: "); Serial.print(alarmaActivada ? "true" : "false");
  Serial.print(" | temporizadorActivo1: "); Serial.print(temporizadorActivo1 ? "true" : "false");
  Serial.print(" | tiempoTranscurrido1(ms): "); Serial.print(temporizadorActivo1 ? (millis() - tiempoActivacion1) : 0);
  Serial.print(" | temporizadorActivo2: "); Serial.print(temporizadorActivo2 ? "true" : "false");
  Serial.print(" | tiempoTranscurrido2(ms): "); Serial.println(temporizadorActivo2 ? (millis() - tiempoActivacion2) : 0);
}*/

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

void loop() {
  int estadoSensor1 = digitalRead(sensorPin1);
  int estadoSensor2 = digitalRead(sensorPin2);
  int estadoSensor3 = digitalRead(sensorPin3);
  int estadoSensor4 = digitalRead(aguaPin);

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

  // Lectura Bluetooth vía Serial hardware
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

  if (variable3 == 1 && !cajaAbierta) {
    abrirCaja();
    temporizadorActivo1 = false;
    temporizadorActivo2 = false;
  } else if (variable3 == 0 && cajaAbierta) {
    cerrarCaja();
  }

  if (millis() - ultimoPrint >= intervaloPrint) {
    //printStatus();
    ultimoPrint = millis();
  }
}

