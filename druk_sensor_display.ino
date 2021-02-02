
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

#include "Measurement.h"
#include "MemoryBuffer.h"
#include "PressureSensor.h"

#define UC_FREQUENCY 16000000UL
#define TIMER1_PRESCALER 64UL

#define DESIRED_SAMPLE_FREQUENCY 5000UL // The desired sample frequency of the analog value in Hertz

#define DESIRED_SAMPLES_PER_MEASUREMENT_POINT 500

#define DESIRED_MEASUREMENTS_PER_SCREEN_UPDATE 3

#define DESIRED_BUFFER_LENGTH 30 //Buffer length in seconds

#define TIMER1_CLOCK_TICKS_ADC_TRIGGER (UC_FREQUENCY / (TIMER1_PRESCALER * DESIRED_SAMPLE_FREQUENCY)) - 1

#define MEMORY_BUFFER_LENGTH (DESIRED_SAMPLE_FREQUENCY / DESIRED_SAMPLES_PER_MEASUREMENT_POINT * DESIRED_BUFFER_LENGTH)

#define ANALOG_INPUT 0

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define EEPROM_IS_CALIBRATED_ADDRESS 0
#define EEPROM_CALIBRATION_VALUE_ADDRESS 1

int OLED_RESET = 4;
Adafruit_SSD1306 display(OLED_RESET);

Measurement measurements((uint16_t)DESIRED_SAMPLES_PER_MEASUREMENT_POINT);
MemoryBuffer<MEMORY_BUFFER_LENGTH> memoryBuffer;
PressureSensor sensor;

ISR(ADC_vect) // ADC conversion complete
{
    cli();
    measurements.AddSample(ADC);

    TIFR1 = (1 << OCF1B); // clear Compare Match B Flag
    sei();

} // ISR

void updateScreen()
{
    char currentPressure[10];
    char maxPressure[10];
    dtostrf(sensor.ConvertADCValueToBar(measurements.LastMeasurementValue()), 5, 2, currentPressure);
    dtostrf(sensor.ConvertADCValueToBar(memoryBuffer.GetMaxMeasurement()), 5, 2, maxPressure);

    int16_t maxIndex = memoryBuffer.GetMaxMeasurementIndex();
    int32_t xRectangle = ((int32_t)maxIndex * 118) / (int32_t)MEMORY_BUFFER_LENGTH;
    xRectangle = 118 - xRectangle;

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 14);
    display.println(currentPressure);
    display.setCursor(65, 14);
    display.println(maxPressure);
    display.drawRect(62, 10, 66, 22, WHITE);
    display.drawRect(63, 11, 64, 20, WHITE);

    display.fillRect(xRectangle, 0, 10, 6, WHITE);
    display.display();
}

void AddMeasurementCallBack(uint16_t measurement)
{
    memoryBuffer.AddMeasurement(measurement);
}

void UpdateScreenCallback(uint16_t measurement)
{
    static uint16_t _nMeasurements = 0;
    _nMeasurements++;
    if (_nMeasurements == DESIRED_MEASUREMENTS_PER_SCREEN_UPDATE)
    {
        _nMeasurements = 0;
        updateScreen();
    }
}

void WriteToSerialCallback(uint16_t measurement)
{
    Serial.println(sensor.ConvertADCValueToBar(measurements.LastMeasurementValue()));
}

void CalibrateSensorCallback(uint16_t measurement)
{
    sensor.CalibrateAtmosphericPressure(measurement);
}

void WriteCalibratingSensor()
{
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.print("Calibrating sensor...");
}

void ShowStartScreen(uint8_t calibration)
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0); //positie ''boostgauge''
    display.print("BOOSTGAUGE");
    if (calibration)
    {
        WriteCalibratingSensor();
    }

    display.display();
    delay(1000); //vertraging tussen weergeven Hallo en Volvo 940 TURBO
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0, 0); //positie Volvo 940 TURBO
    display.print("940 TURBO");
    if (calibration)
    {
        WriteCalibratingSensor();
    }

    display.display();
    delay(1000);
}

uint8_t ReadIsCalibratedFlagFromEeprom()
{
    uint8_t value = EEPROM.read(EEPROM_IS_CALIBRATED_ADDRESS);
    return value;
}

void WriteIsCalbratedFlagToEeprom(uint8_t calibrated)
{
    EEPROM.write(EEPROM_IS_CALIBRATED_ADDRESS, calibrated);
}

void WriteCalibrationValueToEeprom()
{
    float value = sensor.GetCalibrationValue();
    EEPROM.put(EEPROM_CALIBRATION_VALUE_ADDRESS, value);
}

void RestoreCalibrationValueFromEeprom()
{
    float value;
    EEPROM.get(EEPROM_CALIBRATION_VALUE_ADDRESS, value);
    sensor.SetCalibrationValue(value);
}

void setup()
{
    Serial.begin(115200);

    uint8_t calibratePressure = (ReadIsCalibratedFlagFromEeprom() + 1) % 2;
    measurements.Initialize(TIMER1_CLOCK_TICKS_ADC_TRIGGER, ANALOG_INPUT);

    if (calibratePressure)
    {
        measurements.SetISRCallback(&CalibrateSensorCallback);
    }
    else
    {
        RestoreCalibrationValueFromEeprom();
    }

    ShowStartScreen(calibratePressure);

    if (calibratePressure)
    {
        measurements.ClearISRCallback(&CalibrateSensorCallback);
        WriteIsCalbratedFlagToEeprom(1);
        WriteCalibrationValueToEeprom();
    }

    measurements.SetCallback(&AddMeasurementCallBack);
    measurements.SetCallback(&WriteToSerialCallback);
    measurements.SetCallback(&UpdateScreenCallback);
}

uint8_t clearedCalibratedFlag = 0;

void loop()
{

    measurements.Loop();

    if (millis() > 45000 && clearedCalibratedFlag == 0) 
    // 1 minutes || Millis on arduino nano can count to about a minute before they rollover
    {
        clearedCalibratedFlag = 1;
        WriteIsCalbratedFlagToEeprom(0);
    }
}