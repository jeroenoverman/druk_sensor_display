#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

#include <stdint.h>

#define NUMBER_OF_CALLBACKS 5

class Measurement
{
private:
    uint16_t _n_samples;
    uint16_t _measurement_value;
    uint16_t _size;
    volatile bool _new_data;
    uint16_t _avaraged_measurement_value;
    void (*_callback_measurement_complete[NUMBER_OF_CALLBACKS])(uint16_t);
    void (*_callback_isr_measurement_complete)(uint16_t);
    void ExecuteCallback();
    void ExecuteISRCallback();
    void IntializeTimer1(uint16_t ticks);
    void InitializeADC(uint8_t adcChannel);
public:
    Measurement(uint16_t size);
    ~Measurement();
    void Initialize(uint16_t ticks, uint8_t adcChannel);
    void Reset();
    void SetCallback(void(*method)(uint16_t));
    void ClearCallback(void(*method)(uint16_t));
    void SetISRCallback(void(*method)(uint16_t));
    void ClearISRCallback(void(*method)(uint16_t));
    void AddSample(uint16_t value);
    void Loop();
    uint16_t LastMeasurementValue();
};


#endif