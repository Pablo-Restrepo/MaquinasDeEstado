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
#define DEBUG(a)            \
    Serial.print(millis()); \
    Serial.print(": ");     \
    Serial.println(a);

const int photocellPin = A0; // Pin analógico conectado al fotocélula
const int ledPin = 13;       // Pin del LED
int outputValue = 0;         // Valor de salida del fotocélula

void readTemperatureAndHumedity(); // Declaracion de los metodos para las multitareas
void readLight();

AsyncTask asyncTaskReadTemperatureAndHumedity(2000, true, readTemperatureAndHumedity); // Tarea asíncrona para leer temperatura y humedad cada 2 segundos
AsyncTask asyncTaskReadLight(4000, true, readLight);                                   // Tarea asíncrona para leer la luz cada 4 segundos

#define NOTE_B0 31
#define NOTE_C1 33
#define NOTE_CS1 35
#define NOTE_D1 37
#define NOTE_DS1 39
#define NOTE_E1 41
#define NOTE_F1 44
#define NOTE_FS1 46
#define NOTE_G1 49
#define NOTE_GS1 52
#define NOTE_A1 55
#define NOTE_AS1 58
#define NOTE_B1 62
#define NOTE_C2 65
#define NOTE_CS2 69
#define NOTE_D2 73
#define NOTE_DS2 78
#define NOTE_E2 82
#define NOTE_F2 87
#define NOTE_FS2 93
#define NOTE_G2 98
#define NOTE_GS2 104
#define NOTE_A2 110
#define NOTE_AS2 117
#define NOTE_B2 123
#define NOTE_C3 131
#define NOTE_CS3 139
#define NOTE_D3 147
#define NOTE_DS3 156
#define NOTE_E3 165
#define NOTE_F3 175
#define NOTE_FS3 185
#define NOTE_G3 196
#define NOTE_GS3 208
#define NOTE_A3 220
#define NOTE_AS3 233
#define NOTE_B3 247
#define NOTE_C4 262
#define NOTE_CS4 277
#define NOTE_D4 294
#define NOTE_DS4 311
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_FS4 370
#define NOTE_G4 392
#define NOTE_GS4 415
#define NOTE_A4 440
#define NOTE_AS4 466
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_CS5 554
#define NOTE_D5 587
#define NOTE_DS5 622
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_FS5 740
#define NOTE_G5 784
#define NOTE_GS5 831
#define NOTE_A5 880
#define NOTE_AS5 932
#define NOTE_B5 988
#define NOTE_C6 1047
#define NOTE_CS6 1109
#define NOTE_D6 1175
#define NOTE_DS6 1245
#define NOTE_E6 1319
#define NOTE_F6 1397
#define NOTE_FS6 1480
#define NOTE_G6 1568
#define NOTE_GS6 1661
#define NOTE_A6 1760
#define NOTE_AS6 1865
#define NOTE_B6 1976
#define NOTE_C7 2093
#define NOTE_CS7 2217
#define NOTE_D7 2349
#define NOTE_DS7 2489
#define NOTE_E7 2637
#define NOTE_F7 2794
#define NOTE_FS7 2960
#define NOTE_G7 3136
#define NOTE_GS7 3322
#define NOTE_A7 3520
#define NOTE_AS7 3729
#define NOTE_B7 3951
#define NOTE_C8 4186
#define NOTE_CS8 4435
#define NOTE_D8 4699
#define NOTE_DS8 4978
#define REST 0

// change this to make the song slower or faster
int tempo = 85;

// change this to whichever pin you want to use
int buzzer = 7;

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

    // Jigglypuff's Song
    // Score available at https://musescore.com/user/28109683/scores/5044153

    NOTE_D5,
    -4,
    NOTE_A5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_E5,
    -4,
    NOTE_FS5,
    8,
    NOTE_G5,
    4,
    NOTE_FS5,
    -4,
    NOTE_E5,
    8,
    NOTE_FS5,
    4,
    NOTE_D5,
    -2,
    NOTE_D5,
    -4,
    NOTE_A5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_E5,
    -4,
    NOTE_FS5,
    8,
    NOTE_G5,
    4,
    NOTE_FS5,
    -1,
    NOTE_D5,
    -4,
    NOTE_A5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_E5,
    -4,
    NOTE_FS5,
    8,
    NOTE_G5,
    4,

    NOTE_FS5,
    -4,
    NOTE_E5,
    8,
    NOTE_FS5,
    4,
    NOTE_D5,
    -2,
    NOTE_D5,
    -4,
    NOTE_A5,
    8,
    NOTE_FS5,
    8,
    NOTE_D5,
    8,
    NOTE_E5,
    -4,
    NOTE_FS5,
    8,
    NOTE_G5,
    4,
    NOTE_FS5,
    -1,

};

// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;
int divider = 0, noteDuration = 0;

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
    lcd.setCursor(0, 1);
    lcd.print(DHT.getHumidity());
    delay(2000);
    lcd.clear();

    if (DHT.getTemperature() > 32)
    {
        sonido();
    }

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

void sonido()
{
    // iterate over the notes of the melody.
    // Remember, the array is twice the number of notes (notes + durations)
    for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2)
    {

        // calculates the duration of each note
        divider = melody[thisNote + 1];
        if (divider > 0)
        {
            // regular note, just proceed
            noteDuration = (wholenote) / divider;
        }
        else if (divider < 0)
        {
            // dotted notes are represented with negative durations!!
            noteDuration = (wholenote) / abs(divider);
            noteDuration *= 1.5; // increases the duration in half for dotted notes
        }

        // we only play the note for 90% of the duration, leaving 10% as a pause
        tone(buzzer, melody[thisNote], noteDuration * 0.9);

        // Wait for the specief duration before playing the next note.
        delay(noteDuration);

        // stop the waveform generation before the next note.
        noTone(buzzer);
    }
}
