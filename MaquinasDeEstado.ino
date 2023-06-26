/**
 * Integrantes:
 *
 * Fabian Ome Peña
 * Carlos Mario Perdomo Ramos
 * Pablo Jose Restrepo Ruiz
 *
 * Arquitectura Computacional - Universidad del Cauca
 *
 * Fuentes:
 *  Photoresistor: https://learn.sunfounder.com/lesson-21-photoresistor-sensor/
 *  Humiture Sensor: https://learn.sunfounder.com/lesson-22-humiture-sensor/
 */

// Incluir las bibliotecas necesarias:
#include "AsyncTaskLib.h"  // Biblioteca para tareas asíncronas
#include "DHTStable.h"     // Biblioteca para el sensor DHT
#include <LiquidCrystal.h> // Biblioteca para la pantalla LCD

// Definir los pines del Arduino conectados a la pantalla LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// Crear una instancia de LiquidCrystal para la pantalla LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

DHTStable DHT;
#define DHT11_PIN 6 // Pin al que esta conectado el SDA del sensor DHT11 al Arduino (pin 6)

const int photocellPin = A0; // Pin analógico conectado al fotocélula
const int ledPin = 13;       // Pin del LED
int outputValue = 0;         // Valor de salida del fotocélula

void readTemperatureAndHumedity(); // Declaracion de los metodos para las multitareas
void readLight();

AsyncTask asyncTaskReadTemperatureAndHumedity(2000, true, readTemperatureAndHumedity); // Tarea asíncrona para leer temperatura y humedad cada 2 segundos
AsyncTask asyncTaskReadLight(4000, true, readLight);                                   // Tarea asíncrona para leer la luz cada 4 segundos

/**
 *@brief Configura la configuración inicial del programa.
 *@details Este metodo inicializa la pantalla LCD, inicia la comunicación serial y ejecuta tareas asíncronas para leer la temperatura y humedad, así como para leer la luz.
 *@details La pantalla LCD se configura con un tamaño de 16x2 caracteres. La comunicación serial se establece a una velocidad de baudios de 9600.
 */
void setup()
{
    lcd.begin(16, 2);
    Serial.begin(9600);

    asyncTaskReadTemperatureAndHumedity.Start();
    asyncTaskReadLight.Start();
}

/**
 *@brief Metodo principal del programa que se ejecuta en un ciclo infinito.
 *@details Este metodo se ejecuta en un bucle infinito y actualiza las tareas asíncronas de lectura de temperatura y humedad, así como de lectura de luz.
 */
void loop()
{
    asyncTaskReadTemperatureAndHumedity.Update();
    asyncTaskReadLight.Update();
}

/**
 *@brief Lee la temperatura y humedad del sensor DHT11.
 *@details Este metodo lee la temperatura y humedad del sensor DHT11 y muestra los valores por el puerto serial y en la pantalla LCD.
 */
void readTemperatureAndHumedity()
{
    int chk = DHT.read11(DHT11_PIN);

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
    lcd.setCursor(0, 1);
    lcd.print(DHT.getHumidity());
    delay(2000);
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Temperatura:");
    lcd.setCursor(0, 1);
    lcd.print(DHT.getTemperature());
    delay(2000);
    lcd.clear();
}

/**
 *@brief Lee la intensidad de luz del sensor de luz.
 *@details Este metodo lee la intensidad de luz del sensor de luz y muestra el valor por el puerto serial y en la pantalla LCD.
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
