/*

  - Joystick.h: https://github.com/MHeironimus/ArduinoJoystickLibrary
*/

#include <Wire.h>
#include <Joystick.h>

// Configuración del Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
                   JOYSTICK_TYPE_GAMEPAD, 
                   3, // Número de botones
                   0, // Hat Switch
                   true, true, false, // X, Y, Z
                   false, false, false, // Rx, Ry, Rz
                   false, false,        // Rudder, Throttle
                   false, false, false);// Accelerator, Brake, Steering

// Direcciones y registros del MPU-6050
const int MPU6050_ADDR = 0x68;
const int MPU6050_ACCEL_XOUT_H = 0x3B;

// Pines de los botones
const int BUTTON_1 = 4;
const int BUTTON_2 = 5;
const int BUTTON_3 = 6;

// Variables para calibración
int16_t xOffset = 0;
int16_t yOffset = 0;

// Constantes ajustadas para mapeo
const int16_t X_MIN = -15500; // Valor mínimo para Accel X
const int16_t X_MAX = 15500;  // Valor máximo para Accel X
const int16_t Y_MIN = -15700; // Valor mínimo para Accel Y
const int16_t Y_MAX = 15800;  // Valor máximo para Accel Y

// Tamaño de la zona muerta
const int DEADZONE = 0;

void setup() {
  // Inicializar comunicación I2C
  Wire.begin();
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B); // Registro de Power Management
  Wire.write(0);    // Salir de modo de suspensión
  Wire.endTransmission(true);

  // Configurar botones como entradas con pull-up
  pinMode(BUTTON_1, INPUT);
  pinMode(BUTTON_2, INPUT);
  pinMode(BUTTON_3, INPUT);

  // Inicializar el puerto serie
  Serial.begin(115200);
  Serial.println("Iniciando calibración...");

  // Calibrar el sensor
  calibrateSensor();

  // Inicializar el Joystick
  Joystick.begin();
}

void loop() {
  // Leer datos del MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU6050_ADDR, 6, true); // Leer 6 bytes: X, Y, Z

  // Leer valores brutos del acelerómetro
  int16_t accelX = Wire.read() << 8 | Wire.read(); // Eje X
  int16_t accelY = Wire.read() << 8 | Wire.read(); // Eje Y

  // Ajustar con los valores de calibración
  accelX -= xOffset;
  accelY -= yOffset;

  // Mapear los valores a rango del joystick (0 a 1023)
  int joyX = map(accelX, X_MIN, X_MAX, 0, 1023);
  int joyY = map(accelY, Y_MIN, Y_MAX, 0, 1023);

  // Aplicar la zona muerta
  if (abs(joyX - 511) <= DEADZONE) joyX = 511; // Ajustar al centro si está dentro de la zona muerta
  if (abs(joyY - 511) <= DEADZONE) joyY = 511;

  // Limitar valores para evitar desbordes
  joyX = constrain(joyX, 0, 1023);
  joyY = constrain(joyY, 0, 1023);

  // Enviar datos al joystick
  Joystick.setXAxis(joyX);
  Joystick.setYAxis(joyY);

  // Leer estado de los botones y enviar al joystick
  Joystick.setButton(0, digitalRead(BUTTON_1) == HIGH); // Botón 1
  Joystick.setButton(1, digitalRead(BUTTON_2) == HIGH); // Botón 2
  Joystick.setButton(2, digitalRead(BUTTON_3) == HIGH); // Botón 3

  delay(10); // Control de frecuencia de actualización
}

void calibrateSensor() {
  int32_t sumX = 0;
  int32_t sumY = 0;
  const int numSamples = 500; // Número de muestras para calibración

  for (int i = 0; i < numSamples; i++) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_ACCEL_XOUT_H);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU6050_ADDR, 6, true);

    int16_t accelX = Wire.read() << 8 | Wire.read(); // Eje X
    int16_t accelY = Wire.read() << 8 | Wire.read(); // Eje Y

    sumX += accelX;
    sumY += accelY;

    delay(2); // Tiempo entre muestras (ajusta según sea necesario)
  }

  // Calcular valores promedio como offset inicial
  xOffset = sumX / numSamples;
  yOffset = sumY / numSamples;
}
