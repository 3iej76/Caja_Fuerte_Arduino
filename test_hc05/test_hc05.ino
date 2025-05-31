int variable3 = 0;
int variableHC05 = 0;
const String codigoCorrecto = "0101";

String verificarCodigoSerial(String codigoRecibido) {
  codigoRecibido.trim();

  if (variableHC05 == 1 && variable3 == 0) {
    return "Ya se ha introducido el código, introduzca la llave.";
  } 
  else if (variableHC05 == 1 && variable3 == 1) {
    return "Ya se ha introducido el codigo";
  }
  else if (codigoRecibido == codigoCorrecto && variable3 == 1 && variableHC05 == 0) {
    variableHC05 = 1;
    return "Caja abierta";
  }
  else if (codigoRecibido == codigoCorrecto && variable3 == 0) {
    variableHC05 = 1;
    return "Código correcto, introduzca la llave";
  }
  else if (codigoRecibido != codigoCorrecto) {
    return "Código incorrecto, intentar otra vez.";
  } else {
    return "Error";
  }
}

void setup() {
  Serial.begin(9600);  // Inicia el monitor serie
  Serial.println("\nIntroduce el código por el Serial Monitor:");
}

void loop() {
  if (Serial.available()) {
    String codigo = Serial.readStringUntil('\n');
    String mensaje = verificarCodigoSerial(codigo);
    Serial.println(mensaje);
  }
}
