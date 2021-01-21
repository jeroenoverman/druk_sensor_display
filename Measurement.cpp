#include "Measurement.h"
#include <avr/io.h>
#include <avr/interrupt.h>

Measurement::Measurement(uint16_t size) : _size(size)
{
    for (uint8_t i = 0; i < NUMBER_OF_CALLBACKS; i++)
    {
        _callback_measurement_complete[i] = nullptr;
    }
    _callback_isr_measurement_complete = nullptr;
    
    Reset();
    
}

Measurement::~Measurement()
{
}

void Measurement::Reset()
{
    _n_samples = 0;
    _measurement_value = 0;
}

void Measurement::SetCallback(void(*method)(uint16_t))
{
    for (uint8_t i = 0; i < NUMBER_OF_CALLBACKS; i++)
    {
        if(_callback_measurement_complete[i] == nullptr)
        {
            _callback_measurement_complete[i] = method;
            return;
        }
    }   
}

void Measurement::ClearCallback(void(*method)(uint16_t))
{
    for (uint8_t i = 0; i < NUMBER_OF_CALLBACKS; i++)
    {
        if(_callback_measurement_complete[i] == method)
        {
            _callback_measurement_complete[i] = nullptr;
            return;
        }
    }
}

void Measurement::SetISRCallback(void(*method)(uint16_t))
{
    _callback_isr_measurement_complete = method;
}

void Measurement::ClearISRCallback(void(*method)(uint16_t))
{
    _callback_isr_measurement_complete = nullptr;
}
    
void Measurement::AddSample(uint16_t value)
{
    // simple exponential moving average filter.
    uint32_t tempValue = ((uint32_t)_n_samples * (uint32_t)_measurement_value) + (uint32_t)value;
    _n_samples++;
    _measurement_value = (uint16_t)(tempValue/(uint32_t)_n_samples);

    if (_n_samples == _size)
    {
        _avaraged_measurement_value = _measurement_value;
        Reset();
        ExecuteISRCallback();
        _new_data = true;
    }
}

void Measurement::Loop()
{
    if (_new_data)
    {
        _new_data = false;
        ExecuteCallback();
    }
}

uint16_t Measurement::LastMeasurementValue()
{
    return _avaraged_measurement_value;
}

void Measurement::ExecuteCallback()
{
    for (uint8_t i = 0; i < NUMBER_OF_CALLBACKS; i++)
    {
        if (_callback_measurement_complete[i] != nullptr)
        {
            _callback_measurement_complete[i](_avaraged_measurement_value);
        }
    }
}

void Measurement::ExecuteISRCallback()
{
    if(_callback_isr_measurement_complete != nullptr)
    {
        _callback_isr_measurement_complete(_avaraged_measurement_value);
    }
}

void Measurement::Initialize(uint16_t ticks, uint8_t adcChannel)
{
    IntializeTimer1(ticks);
    InitializeADC(adcChannel);
}

void Measurement::IntializeTimer1(uint16_t ticks)
{

    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    TIMSK1 = 0;

    TCCR1B = (1 << WGM12); // Configure for CTC mode 4 OCR1A=TOP

    OCR1B = ticks; // Compare value
    OCR1A = ticks; // Set CTC TOP value, must be >= OCR1B

    // Set prescaler
    TCCR1B |= (1 << CS11)|(1 << CS10); // Fcpu/64
}

void Measurement::InitializeADC(uint8_t adcChannel)
{
    cli();
    // enable adc, auto trigger, interrupt enable, prescale=128
    ADCSRA = ((1 << ADEN) | (1 << ADATE) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));
    // auto trigger source to timer1 compareB
    ADCSRB = ((1 << ADTS2) | (1 << ADTS0));
    // Use AVcc as reference voltage and use given analog input
    ADMUX = ((1 << REFS0) | (adcChannel));
    sei();
}
