#ifndef __MEMORYBUFFER_H__
#define __MEMORYBUFFER_H__

#include <stdint.h>

template<unsigned N>
class MemoryBuffer
{
private:
    uint16_t _values[N];
    static const uint16_t _size = N;
    int16_t _index;
    uint16_t _maxValue;
    int16_t _maxIndex;
    uint16_t _minValue;
    int16_t _minIndex;
    bool _firstEntry;
    void _recalculateMinMax();
    int16_t _getArrayIndex(int16_t index);
public:
    MemoryBuffer();
    ~MemoryBuffer();
    void AddMeasurement(uint16_t value);
    uint16_t GetMaxMeasurement();
    int16_t GetMaxMeasurementIndex();
    uint16_t GetMinMeasurement();
    int16_t GetMinMeasurementIndex();
};

template<unsigned N>
MemoryBuffer<N>::MemoryBuffer():_index(0), _maxValue(0), _maxIndex(0), _minValue(0), _minIndex(0), _firstEntry(true)
{
}

template<unsigned N>
MemoryBuffer<N>::~MemoryBuffer()
{
}

template<unsigned N>
void MemoryBuffer<N>::AddMeasurement(uint16_t value)
{
    int16_t valueIndex = _index;
    _index = (_index + 1) % _size;
    _values[valueIndex] = value;
    if (_firstEntry)
    {
        _firstEntry = false;
        _maxValue = _minValue = value;
        _maxIndex = _minIndex = valueIndex;
        return;
    }
    
    if (value >= _maxValue)
    {
        _maxValue = value;
        _maxIndex = valueIndex;
    } else if (_maxIndex == valueIndex)
    {
        _recalculateMinMax();
    }
    
    if (value <= _minValue)
    {
        _minValue = value;
        _minIndex = valueIndex;
    } else if (_minIndex == valueIndex)
    {
        _recalculateMinMax();
    }
    
}

template<unsigned N>
uint16_t MemoryBuffer<N>::GetMaxMeasurement()
{
    return _maxValue;
}

template<unsigned N>
int16_t MemoryBuffer<N>::GetMaxMeasurementIndex()
{
    return (_index - _maxIndex + _size) % _size;
}

template<unsigned N>
uint16_t MemoryBuffer<N>::GetMinMeasurement()
{
    return _minValue;
}

template<unsigned N>
int16_t MemoryBuffer<N>::GetMinMeasurementIndex()
{
    return (_index - _minIndex + _size) % _size;
}

template<unsigned N>
void MemoryBuffer<N>::_recalculateMinMax()
{
    _maxValue = _minValue = _values[_getArrayIndex(-1)];
    _maxIndex = _minIndex = _getArrayIndex(-1);
    for (uint8_t i = 0; i < (_size - 1); i++)
    {
        int16_t currentIndex = _getArrayIndex(i);
        uint16_t currentValue = _values[currentIndex];
        if (currentValue >= _maxValue)
        {
            _maxValue = currentValue;
            _maxIndex = currentIndex;
        }
        if (currentValue <= _minValue)
        {
            _minValue = currentValue;
            _minIndex = currentIndex;
        }
    }
    
}

template<unsigned N>
int16_t MemoryBuffer<N>::_getArrayIndex(int16_t index)
{
    return (_index + index) % _size;
}

#endif