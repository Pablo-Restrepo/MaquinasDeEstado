/**
   Integrantes:

   Fabian Ome Peña
   Carlos Mario Perdomo Ramos
   Pablo Jose Restrepo Ruiz

   Arquitectura Computacional - Universidad del Cauca

   Fuentes:
    Photoresistor: https://learn.sunfounder.com/lesson-21-photoresistor-sensor/
    Humiture Sensor: https://learn.sunfounder.com/lesson-22-humiture-sensor/
*/

// Incluir las bibliotecas necesarias:
#include "AsyncTaskLib.h"  // Biblioteca para tareas asíncronas
#include "DHTStable.h"     // Biblioteca para el sensor DHT
#include <LiquidCrystal.h> // Biblioteca para la pantalla LCD
#include <Keypad.h>
#include "StateMachineLib.h"

// Definir el tamaño del teclado
const byte ROWS = 4;
const byte COLS = 4;

// Definir la disposición de las teclas en el teclado
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

// Definir los pines del Arduino conectados a las filas y columnas del teclado
byte rowPins[ROWS] = {39, 41, 43, 45};
byte colPins[COLS] = {47, 49, 51, 53};

// Crear una instancia de Keypad con los parámetros anteriores
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Contraseña esperada: 1234
char password[8] = {'1', '2', '3', '4'};

// Contraseña ingresada por el usuario
char inputPassword[8];

// Índice actual para ingresar la contraseña
unsigned char idx = 0;

// Número de intentos de contraseña fallidos
int attempts = 0;

int varAlar = 0;

// Bandera para indicar si se han realizado tres intentos fallidos consecutivos
boolean threeAttempts = false;

// Definir los pines del Arduino conectados a la pantalla LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// Crear una instancia de LiquidCrystal para la pantalla LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

DHTStable DHT;
#define DHT11_PIN 6 // Pin al que esta conectado el SDA del sensor DHT11 al Arduino (pin 6)
#define DEBUG(a)            \
    Serial.print(millis()); \
    Serial.print(": ");     \
    Serial.println(a);

const int photocellPin = A0; // Pin analógico conectado al fotocélula
const int ledPin = 13;       // Pin del LED
int outputValue = 0;         // Valor de salida del fotocélula

void readTemperatureAndHumedity(); // Declaracion de los metodos para las multitareas
void readLight();
void readPassword();
void systemLocked();
void readHall();
void readMetalTouch();
void readTracking();

AsyncTask asyncTaskReadTemperatureAndHumedity(2000, true, readTemperatureAndHumedity); // Tarea asíncrona para leer temperatura y humedad cada 2 segundos
AsyncTask asyncTaskReadLight(4000, true, readLight);                                   // Tarea asíncrona para leer la luz cada 4 segundos
AsyncTask asyncTaskReadPassword(100, true, readPassword);
AsyncTask asyncTaskSystemLocked(100, true, systemLocked);
AsyncTask asyncTaskReadHall(100, true, readHall);
AsyncTask asyncTaskReadMetalTouch(100, true, readMetalTouch);
AsyncTask asyncTaskReadTracking(100, true, readTracking);

enum State
{
    Inicio = 0,
    Monitoreo = 1,
    MonitoreoPuertasVentanas = 2,
    Alarma = 3,
    SistemaBloqueado = 4
};

enum Input
{
    Reset = 0,
    Forward = 1,
    Backward = 2,
    Unknown = 3,
};

StateMachine stateMachine(4, 9);
Input input;

const int buzzerPin = 7; // the buzzer pin attach to
int fre;                 // set the variable to store the frequence value

/**
  @brief Configura la configuración inicial del programa.
  @details Este metodo inicializa la pantalla LCD, inicia la comunicación serial y ejecuta tareas asíncronas para leer la temperatura y humedad, así como para leer la luz.
  @details La pantalla LCD se configura con un tamaño de 16x2 caracteres. La comunicación serial se establece a una velocidad de baudios de 9600.
*/
void setup()
{
    pinMode(buzzerPin, OUTPUT);
    lcd.begin(16, 2);
    Serial.begin(9600);
    asyncTaskReadPassword.Start();
    asyncTaskReadTemperatureAndHumedity.Start();
    asyncTaskReadLight.Start();
    asyncTaskSystemLocked.Start();
    asyncTaskReadHall.Start();
    asyncTaskReadMetalTouch.Start();
    asyncTaskReadTracking.Start();
}

/**
  @brief Metodo principal del programa que se ejecuta en un ciclo infinito.
  @details Este metodo se ejecuta en un bucle infinito y actualiza las tareas asíncronas de lectura de temperatura y humedad, así como de lectura de luz.
*/
void loop()
{
    asyncTaskReadPassword.Update();
    asyncTaskReadTemperatureAndHumedity.Update();
    asyncTaskReadLight.Update();
}

/**
  @brief Lee la temperatura y humedad del sensor DHT11.
  @details Este metodo lee la temperatura y humedad del sensor DHT11 y muestra los valores por el puerto serial y en la pantalla LCD.
*/
void readTemperatureAndHumedity()
{
    int chk = DHT.read22(DHT11_PIN);
    // int chk = DHT.read11(DHT11_PIN);

    Serial.print("Estado:\tHumedad (%):\tTemperatura (C):");
    Serial.print("\n");

    switch (chk)
    {
    case DHTLIB_OK:
        Serial.print("OK\t");
        break;
    case DHTLIB_ERROR_CHECKSUM:
        Serial.print("Checksum error\t");
        break;
    case DHTLIB_ERROR_TIMEOUT:
        Serial.print("Time out error\t");
        break;
    default:
        Serial.print("Unknown error\t");
        break;
    }

    Serial.print(DHT.getHumidity());
    Serial.print("\t\t");
    Serial.print(DHT.getTemperature());
    Serial.print("\t\t");
    Serial.print("\n");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Humedad:");
    lcd.setCursor(9, 0);
    lcd.print(DHT.getHumidity());
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.setCursor(6, 1);
    lcd.print(DHT.getTemperature());
    delay(2000);

    if (DHT.getTemperature() > 32)
    {
        alarma();
    }
    else
    {
        varAlar = 0;
    }
}

/**
  @brief Lee la intensidad de luz del sensor de luz.
  @details Este metodo lee la intensidad de luz del sensor de luz y muestra el valor por el puerto serial y en la pantalla LCD.
*/
void readLight()
{
    outputValue = analogRead(photocellPin);

    Serial.println("Luminosidad:");
    Serial.println(outputValue);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Luminosidad:");
    lcd.setCursor(0, 1);
    lcd.print(outputValue);
}

void alarma()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alarma:");
    lcd.setCursor(0, 1);
    lcd.print("Temp mayor a 32C");
    for (int j = 0; j <= 4; j++)
    {
        for (int i = 200; i <= 800; i += 10) // Incremento de 10 en lugar de 1 para un cambio más rápido de frecuencia
        {
            tone(7, i); // Generar tono en el pin 7 con frecuencia i
            delay(9);   // Pequeña pausa para cada frecuencia
        }
        for (int i = 800; i >= 200; i -= 10) // Decremento de 10 en lugar de 1 para un cambio más rápido de frecuencia
        {
            tone(7, i); // Generar tono en el pin 7 con frecuencia i
            delay(9);   // Pequeña pausa para cada frecuencia
        }
        noTone(7); // Detener el tono al final de la alarma
    }
    varAlar++;
    if (varAlar >= 4)
    {
        asyncTaskSystemLocked.Update();
    }
}

void readPassword()
{
    char key = keypad.getKey();
    boolean bandera = true;
    reset();
    while (true)
    {
        if (attempts >= 3)
        {
            asyncTaskSystemLocked.Update();
        }
        if (key)
        {
            if (key == '#')
            {
                if (isPasswordCorrect())
                {
                    print("Clave Correcta!");
                    attempts = 0;
                    return;
                }
                print("Clave Incorrecta!");
                attempts++;
                bandera = false;
            }
            if (idx >= 8)
            {
                print("Fuera de Rango!");
                bandera = false;
            }
            if (bandera)
            {
                inputPassword[idx] = key;
                lcd.print('*');
                idx++;
                (idx >= 8) ? idx : 0;
            }
            bandera = true;
        }
        key = keypad.getKey(); // Obtener una nueva tecla en cada iteración
    }
}

/**
   Imprime un mensaje en el LCD y restablece los valores.

   @param Message El mensaje a imprimir en el LCD.
*/
void print(String Message)
{
    lcd.clear();
    lcd.print(Message);
    delay(2000);
    lcd.clear();
    reset();
}

/**
   Verifica si la contraseña ingresada es correcta.

   @return true si la contraseña es correcta, false de lo contrario.
*/
bool isPasswordCorrect()
{
    return strcmp(inputPassword, password) == 0;
}

/**
   Restablece los valores y muestra un mensaje en el LCD.
*/
void reset()
{
    memset(inputPassword, 0, sizeof(inputPassword));
    idx = 0;
    lcd.setCursor(0, 0);
    lcd.print("Ingrese clave:");
    lcd.setCursor(0, 1);
}

void systemLocked()
{
    lcd.clear();
    lcd.print("Siste. Bloqueado");
    exit(0);
}

void readHall()
{
}

void readMetalTouch()
{
}

void readTracking()
{
}