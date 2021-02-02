#include "PressureSensor.h"

PressureSensor::PressureSensor(/* args */)
{
    _voltageScalar = 5.0 / 1024;
    _voltageOffset = 0;
    _kpaScalar = (250 - 20) / (4.65 - 0.4);
    _kpaScalar /= 1.18; // calibrated for sensor from aliExpress
    _kpaOffset = 20 - _kpaScalar * 0.4;
    _barScalar = 0.01;
    _barOffset = -1;
}

PressureSensor::~PressureSensor()
{
}

float PressureSensor::_convertAdcValueToVoltage(uint16_t adcValue)
{
    return _voltageScalar * (float)adcValue + _voltageOffset;
}
float PressureSensor::_convertVoltageToKpa(float voltage)
{
    return _kpaScalar * voltage + _kpaOffset;
}
float PressureSensor::_convertKpaToBar(float kpa)
{
    return _barScalar * kpa + _barOffset;
}

float PressureSensor::ConvertADCValueToBar(uint16_t adcValue)
{
    return _convertKpaToBar(_convertVoltageToKpa(_convertAdcValueToVoltage(adcValue)));
}

void PressureSensor::CalibrateAtmosphericPressure(uint16_t adcValue)
{
    static uint16_t value = 0;
    static uint16_t samples = 0;
    uint32_t tempValue = ((uint32_t)samples * (uint32_t)value) + (uint32_t)adcValue;
    samples++;
    value = (uint16_t)(tempValue / (uint32_t)samples);
    _barOffset = 0;
    _barOffset = 0 - ConvertADCValueToBar(value);
}

float PressureSensor::GetCalibrationValue()
{
    return _barOffset;
}

void PressureSensor::SetCalibrationValue(float calibrationValue)
{
    _barOffset = calibrationValue;
}