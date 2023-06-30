/**
 * @file MaquinasDeEstado.ino
 *
 * @brief Descripción general del programa.
 *
 * Este programa realiza diversas funciones como leer sensores de temperatura, humedad y luz,
 * controlar una alarma, y administrar un sistema de seguridad basado en una contraseña.
 *
 * Integrantes:
 * - Fabian Ome Peña
 * - Carlos Mario Perdomo Ramos
 * - Pablo Jose Restrepo Ruiz
 *
 * Arquitectura Computacional - Universidad del Cauca
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

// Leds
const int redPin = 21;
const int bluePin = 20;
const int greenPin = 19;

// Contrasenia
char password[5] = "1234";
char contraIngre[5];
int longiCadena = 0;
int attempts = 0;
boolean passwordCorrect = false;

// Variables varias
int varAlar = 0;
int tempInicio = 0;
int tempFinal = 0;
int outputValue = 0;
int contadorAlarma = 0;

// el pin del zumbador se conecta aquí
const int buzzerPin = 7;
int fre;

// Definir los pines del Arduino conectados a la pantalla LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
// Crear una instancia de LiquidCrystal para la pantalla LCD
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

DHTStable DHT;
#define DHT11_PIN 6 // Pin al que está conectado el SDA del sensor DHT11 al Arduino (pin 6)
#define DEBUG(a)            \
    Serial.print(millis()); \
    Serial.print(": ");     \
    Serial.println(a);

const int photocellPin = A0; // Pin analógico conectado al fotocélula
const int ledPin = 13;       // Pin del LED

// Declaración de funciones
void temporizador();
void readTemperatureAndHumedity();
void timeoutg();
void time20s();

// Tareas asíncronas
AsyncTask asyncTaskTemporizador(5000, false, temporizador);
AsyncTask asyncTasktime20s(10000, false, time20s);
AsyncTask asyncTasktime2I5s(2500, false, timeoutg);
AsyncTask asyncTasktime1I5s(1500, false, timeoutg);
AsyncTask asyncTaskTemp(1000, true, readTemperatureAndHumedity);
AsyncTask asyncTasktime5s(5000, true, timeoutg);

// Definición de los estados posibles
enum State
{
    Inicio = 0,
    Monitoreo = 1,
    MonitoreoPuertasVentanas = 2,
    Alarma = 3,
    SistemaBloqueado = 4
};

// Definición de las entradas posibles
enum Input
{
    Unknown = 0,
    contraseCorrecta = 1,
    sisBloqueado = 2,
    tempover = 3,
    timeout = 4,
    tempoverandto = 5,
    asterisco = 6,
    evento = 7
};

// Configuración de stateMachine e input value
StateMachine stateMachine(5, 11);
Input input;
Input currentInput = Input::Unknown;

/**
 * @brief Configura la configuración inicial del programa.
 *
 * Este metodo inicializa la pantalla LCD, inicia la comunicación serial y ejecuta tareas asíncronas para
 * leer la temperatura y humedad, así como para leer la luz.
 *
 * La pantalla LCD se configura con un tamaño de 16x2 caracteres. La comunicación serial se establece a
 * una velocidad de baudios de 9600.
 */
void setup()
{
    pinMode(buzzerPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    lcd.begin(16, 2);
    Serial.begin(9600);

    Serial.println("Starting State Machine...");
    setupStateMachine();
    Serial.println("Start Machine Started");

    stateMachine.SetState(Inicio, false, true);
}

/**
 * @brief Metodo principal del programa que se ejecuta en un ciclo infinito.
 *
 * Este metodo se ejecuta en un bucle infinito y actualiza las tareas asíncronas de lectura de temperatura
 * y humedad, así como de lectura de luz.
 */
void loop()
{
    input = static_cast<Input>(readInput());
    asyncTaskTemp.Update();
    asyncTasktime5s.Update();
    asyncTasktime20s.Update();
    asyncTasktime2I5s.Update();
    asyncTasktime1I5s.Update();
    asyncTaskTemporizador.Update();
    stateMachine.Update();
}

/**
 * @brief Aplicación de seguridad y monitoreo con Arduino.
 *
 * Este programa implementa un sistema de seguridad y monitoreo utilizando Arduino y varios componentes,
 * como un sensor DHT11 para medir temperatura y humedad, una pantalla LCD para mostrar información, un teclado
 * matricial para la entrada de contraseñas y un fotocélula para detectar cambios en la luminosidad.
 * El sistema consta de varios estados y se controla mediante una máquina de estados finitos.
 */
int readInput()
{
    Input currentInput = Input::Unknown;
    char key = keypad.getKey();

    if (stateMachine.GetState() == MonitoreoPuertasVentanas)
    {
        if (key == '*')
        {
            asyncTaskTemp.Stop();
            asyncTasktime5s.Stop();
            asyncTasktime20s.Stop();
            asyncTasktime1I5s.Stop();
            asyncTasktime2I5s.Stop();
            attempts = 0;
            currentInput = Input::asterisco;
        }
    }

    if (attempts >= 3)
    {
        currentInput = Input::sisBloqueado;
    }

    if (!passwordCorrect)
    {
        if (key)
        {
            asyncTaskTemporizador.Start();
            if (key == '#')
            {
                asyncTaskTemporizador.Stop();
                if (isPasswordCorrect())
                {
                    color(0, 255, 0);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Clave Correcta!");
                    delay(2000);
                    passwordCorrect = true;
                    currentInput = Input::contraseCorrecta;
                    lcd.clear();
                }
                else
                {
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("Clave Incorrecta!");
                    delay(2000);
                    attempts++;
                    inputSecuritySystem();
                }

                for (int i = 0; i < 5; i++)
                {
                    contraIngre[i] = NULL;
                }
            }
            else
            {
                contraIngre[longiCadena++] = key;
                lcd.print("*");
            }
        }
    }
    return static_cast<int>(currentInput);
}

/**
 * @brief Lee la temperatura y humedad del sensor DHT11.
 *
 * Este metodo lee la temperatura y humedad del sensor DHT11 y muestra los valores por el puerto serial
 * y en la pantalla LCD.
 */
void readTemperatureAndHumedity()
{
    // int chk = DHT.read22(DHT11_PIN);
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

    color(0, 0, 255);
    outputValue = analogRead(photocellPin);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hum:");
    lcd.setCursor(4, 0);
    lcd.print(DHT.getHumidity());
    lcd.setCursor(9, 0);
    lcd.print("Luz:");
    lcd.print(outputValue);
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.setCursor(5, 1);
    lcd.print(DHT.getTemperature());

    if (DHT.getTemperature() > 32)
    {
        tone(buzzerPin, 800);
        input = Input::tempover;
    }
    else
    {
        asyncTasktime20s.Stop();
        contadorAlarma = 0;
        noTone(buzzerPin);
    }
}

/**
 * @brief Configura la máquina de estados y define las transiciones y acciones correspondientes a cada estado.
 */
void setupStateMachine()
{
    stateMachine.AddTransition(Inicio, Monitoreo, []()
                               { return input == contraseCorrecta; });
    stateMachine.AddTransition(Inicio, SistemaBloqueado, []()
                               { return input == sisBloqueado; });

    stateMachine.AddTransition(Monitoreo, Alarma, []()
                               { return input == tempover; });
    stateMachine.AddTransition(Monitoreo, MonitoreoPuertasVentanas, []()
                               { return input == timeout; });
    stateMachine.AddTransition(Monitoreo, SistemaBloqueado, []()
                               { return input == tempoverandto; });

    stateMachine.AddTransition(Alarma, SistemaBloqueado, []()
                               { return input == tempoverandto; });
    stateMachine.AddTransition(Alarma, Monitoreo, []()
                               { return input == timeout; });

    stateMachine.AddTransition(SistemaBloqueado, MonitoreoPuertasVentanas, []()
                               { return input == timeout; });

    stateMachine.AddTransition(MonitoreoPuertasVentanas, Inicio, []()
                               { return input == asterisco; });
    stateMachine.AddTransition(MonitoreoPuertasVentanas, Monitoreo, []()
                               { return input == timeout; });
    stateMachine.AddTransition(MonitoreoPuertasVentanas, SistemaBloqueado, []()
                               { return input == evento; });

    stateMachine.SetOnEntering(Inicio, inputSecuritySystem);
    stateMachine.SetOnEntering(Monitoreo, inputDHT);
    stateMachine.SetOnEntering(Alarma, inputAlarma);
    stateMachine.SetOnEntering(SistemaBloqueado, inputSistemaBloqueado);
    stateMachine.SetOnEntering(MonitoreoPuertasVentanas, inputMonitoreoPuertasVentanas);

    stateMachine.SetOnLeaving(Inicio, []()
                              { Serial.println("Leaving Inicio"); });
    stateMachine.SetOnLeaving(Monitoreo, []()
                              { Serial.println("Leaving Monitoreo"); });
    stateMachine.SetOnLeaving(Alarma, []()
                              { Serial.println("Leaving Alarma"); });
    stateMachine.SetOnLeaving(MonitoreoPuertasVentanas, []()
                              { Serial.println("Leaving MonitoreoPuertasVentanas"); });
    stateMachine.SetOnLeaving(SistemaBloqueado, []()
                              { Serial.println("Leaving SistemaBloqueado"); });
}

/**
 * @brief Cambia el color de los LEDs RGB.
 *
 * @param red   Valor de intensidad para el LED rojo (0-255).
 * @param green Valor de intensidad para el LED verde (0-255).
 * @param blue  Valor de intensidad para el LED azul (0-255).
 */
void color(unsigned char red, unsigned char green, unsigned char blue)
{
    analogWrite(redPin, red);
    analogWrite(bluePin, blue);
    analogWrite(greenPin, green);
}

/**
 * @brief Realiza una acción cuando se alcanza el tiempo límite de 20 segundos.
 */
void time20s()
{
    input = Input::tempoverandto;
    if (stateMachine.GetState() == SistemaBloqueado)
    {
        input = Input::timeout;
    }
}

/**
 * @brief Realiza una acción cuando se alcanza el tiempo límite de timeout.
 */
void timeoutg()
{
    input = Input::timeout;
}

/**
 * @brief Función de temporizador que se ejecuta cuando se detecta una clave incorrecta.
 */
void temporizador()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Clave Incorrecta!");
    delay(2000);
    inputSecuritySystem();
    tempInicio = 0;
    tempFinal = 0;
    attempts++;
}

/**
 * @brief Realiza una acción al ingresar al estado de Security System.
 */
void inputSecuritySystem()
{
    Serial.println("Entering SecuritySystem");
    passwordCorrect = false;
    longiCadena = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ingrese Clave:");
    lcd.setCursor(0, 1);
    color(0, 0, 0);
}

/**
 * @brief Realiza una acción al ingresar al estado de Alarma.
 */
void inputAlarma()
{
    Serial.println("Entering Alarma");
    color(255, 0, 0);
    asyncTasktime5s.Start();
    if (contadorAlarma == 0)
    {
        asyncTasktime20s.Start();
    }
    contadorAlarma++;
    tone(buzzerPin, 800);
    color(255, 0, 0);
}

/**
 * @brief Realiza una acción al ingresar al estado de Monitoreo DHT.
 */
void inputDHT()
{
    Serial.println("Entering MonitoreoDHT");
    asyncTaskTemp.Start();
    asyncTasktime2I5s.Start();
}

/**
 * @brief Realiza una acción al ingresar al estado de Sistema Bloqueado.
 */
void inputSistemaBloqueado()
{
    Serial.println("Entering SistemaBloqueado");
    color(255, 0, 0);
    asyncTaskTemp.Stop();
    asyncTasktime5s.Stop();
    asyncTasktime1I5s.Stop();
    asyncTasktime2I5s.Stop();
    contadorAlarma = 0;
    noTone(buzzerPin);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SistemaBloqueado");
    asyncTasktime20s.Stop();
    asyncTasktime20s.Start();
}

/**
 * @brief Realiza una acción al ingresar al estado de Monitoreo de Puertas y Ventanas.
 */
void inputMonitoreoPuertasVentanas()
{
    Serial.println("Entering MonitoreoPuertasVentanas");
    color(0, 0, 255);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PuertasYVentanas");
    delay(2000);
}

/**
 * @brief Verifica si la contraseña ingresada es correcta.
 *
 * @return true si la contraseña es correcta, false de lo contrario.
 */
bool isPasswordCorrect()
{
    return strcmp(contraIngre, password) == 0;
}

/**
 * @brief Bloquea el sistema y muestra un mensaje en la pantalla LCD.
 */
void systemLocked()
{
    lcd.clear();
    lcd.print("Sis. Bloqueado");
    exit(0);
}
