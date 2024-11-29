#include <Wire.h>
#include <Joystick.h>

// Configuración del Joystick
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
                   JOYSTICK_TYPE_GAMEPAD, 
                   0, // Botones
                   0, // Hat Switch
                   true, true, false, // X, Y, Z
                   false, false, false, // Rx, Ry, Rz
                   false, false,        // Rudder, Throttle
                   false, false, false);// Accelerator, Brake, Steering

// Direcciones y registros del MPU-6050
const int MPU6050_ADDR = 0x68;
const int MPU6050_ACCEL_XOUT_H = 0x3B;

// Variables para calibración
int16_t xOffset = 0;
int16_t yOffset = 0;

// Constantes ajustadas para mapeo
const int16_t X_MIN = -15500; // Valor mínimo para Accel X
const int16_t X_MAX = 15500;  // Valor máximo para Accel X
const int16_t Y_MIN = -15700; // Valor mínimo para Accel Y
const int16_t Y_MAX = 15800;  // Valor máximo para Accel Y

// Tamaño de la zona muerta
const int DEADZONE = 10; // Ajustada para rangos más grandes

void setup() {
  // Inicializar comunicación I2C
  Wire.begin();
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(0x6B); // Registro de Power Management
  Wire.write(0);    // Salir de modo de suspensión
  Wire.endTransmission(true);

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

  // Imprimir en el puerto serie para depuración
  Serial.print("Accel X: ");
  Serial.print(accelX);
  Serial.print(" | Mapped X: ");
  Serial.print(joyX);
  Serial.print(" | Accel Y: ");
  Serial.print(accelY);
  Serial.print(" | Mapped Y: ");
  Serial.println(joyY);

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

  Serial.print("Calibración completada: X Offset = ");
  Serial.print(xOffset);
  Serial.print(", Y Offset = ");
  Serial.println(yOffset);
}
