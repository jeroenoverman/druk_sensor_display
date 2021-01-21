#ifndef __PRESSURESENSOR_H__
#define __PRESSURESENSOR_H__

#include <stdint.h>

class PressureSensor
{
private:
    float _voltageScalar;
    float _voltageOffset;
    float _kpaScalar;
    float _kpaOffset;
    float _barScalar;
    float _barOffset;

    float _convertAdcValueToVoltage(uint16_t adcValue);
    float _convertVoltageToKpa(float voltage);
    float _convertKpaToBar(float kpa);

public:
    PressureSensor(/* args */);
    ~PressureSensor();
    float ConvertADCValueToBar(uint16_t adcValue);
    void CalibrateAtmosphericPressure(uint16_t adcValue);
    float GetCalibrationValue();
    void SetCalibrationValue(float calibrationValue);
};

#endif