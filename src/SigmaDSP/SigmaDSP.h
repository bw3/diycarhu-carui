#ifndef SIGMADSP_H
#define SIGMADSP_H

#include <cstdint>
#include <linux/i2c-dev.h>
#include <array>
#include <map>
#include <string>
#include "parameters.h"
#include "tinyxml2.h"

// Only ADAU1701 is supported at the moment
#define ADAU1701 0

// Hardware register constants
typedef enum
{
  InterfaceRegister0       = 0x0800,
  InterfaceRegister1       = 0x0801,
  InterfaceRegister2       = 0x0802,
  InterfaceRegister3       = 0x0803,
  InterfaceRegister4       = 0x0804,
  InterfaceRegister5       = 0x0805,
  InterfaceRegister6       = 0x0806,
  InterfaceRegister7       = 0x0807,
  GpioAllRegister          = 0x0808,
  Adc0                     = 0x0809,
  Adc1                     = 0x080A,
  Adc2                     = 0x080B,
  Adc3                     = 0x080C,
  SafeloadData0            = 0x0810,
  SafeloadData1            = 0x0811,
  SafeloadData2            = 0x0812,
  SafeloadData3            = 0x0813,
  SafeloadData4            = 0x0814,
  SafeloadAddress0         = 0x0815,
  SafeloadAddress1         = 0x0816,
  SafeloadAddress2         = 0x0816,
  SafeloadAddress3         = 0x0818,
  SafeloadAddress4         = 0x0819,
  DataCapture0             = 0x081A,
  DataCpature1             = 0x081B,
  CoreRegister             = 0x081C,
  RAMRegister              = 0x081D,
  SerialOutRegister1       = 0x081E,
  SerialInputRegister      = 0x081F,
  MpCfg0                   = 0x0820,
  MpCfg1                   = 0x0821,
  AnalogPowerDownRegister  = 0x0822,
  AnalogInterfaceRegister0 = 0x0824
} dspRegister;

class SigmaDSP
{
  public:
    // Store passed device type and i2c address to private constants
    SigmaDSP(uint8_t i2cAddress, uint8_t device, int8_t resetPin = -1);

    // Init and setup
    void begin(int adapter_nr=1);
    void loadXml(const char* filename);
    void i2cClock(uint32_t clock);
    void reset();
    uint8_t ping();

    // DSP functions
    bool mux(std::string name,        uint8_t index,    uint8_t numberOfIndexes = 0);
    bool demux(std::string name,      uint8_t index,    uint8_t numberOfIndexes);
    bool gain(std::string name,          float gainVal, uint8_t channels = 1);
    bool gain(std::string name,        int32_t gainVal, uint8_t channels = 1);
    bool gain(std::string name,         double gainVal, uint8_t channels = 1) { return gain(name,   (float)gainVal, channels); }
    bool gain(std::string name,       uint32_t gainVal, uint8_t channels = 1) { return gain(name, (int32_t)gainVal, channels); }
    bool gain(std::string name,       uint16_t gainVal, uint8_t channels = 1) { return gain(name, (int32_t)gainVal, channels); }
    bool gain(std::string name,        int16_t gainVal, uint8_t channels = 1) { return gain(name, (int32_t)gainVal, channels); }
    bool gain(std::string name,        uint8_t gainVal, uint8_t channels = 1) { return gain(name, (int32_t)gainVal, channels); }
    bool gain(std::string name,         int8_t gainVal, uint8_t channels = 1) { return gain(name, (int32_t)gainVal, channels); }

    bool volume_slew(std::string name           , float db,        uint8_t slew = 12);
    bool dynamicBass(std::string name, float dB);
    bool hardClip(std::string name,     float highThreshold, float lowThreshold);
    bool softClip(std::string name,     float alpha);
    bool dcSource(std::string name,     float level);

    bool sineSource(std::string name,    float frequency);
    bool sineSource(std::string name,  int32_t frequency);
    bool sineSource(std::string name,   double frequency) { return sineSource(name,   (float)frequency); }
    bool sineSource(std::string name, uint32_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool sineSource(std::string name, uint16_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool sineSource(std::string name,  int16_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool sineSource(std::string name,  uint8_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool sineSource(std::string name,   int8_t frequency) { return sineSource(name, (int32_t)frequency); }

    bool squareSource(std::string name,    float frequency);
    bool squareSource(std::string name,  int32_t frequency);
    bool squareSource(std::string name,   double frequency) { return sineSource(name,   (float)frequency); }
    bool squareSource(std::string name, uint32_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool squareSource(std::string name, uint16_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool squareSource(std::string name,  int16_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool squareSource(std::string name,  uint8_t frequency) { return sineSource(name, (int32_t)frequency); }
    bool squareSource(std::string name,   int8_t frequency) { return sineSource(name, (int32_t)frequency); }

    bool sawtoothSource(std::string name,    float frequency);
    bool sawtoothSource(std::string name,  int32_t frequency);
    bool sawtoothSource(std::string name,   double frequency) { return sawtoothSource(name,   (float)frequency); }
    bool sawtoothSource(std::string name, uint32_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    bool sawtoothSource(std::string name, uint16_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    bool sawtoothSource(std::string name,  int16_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    bool sawtoothSource(std::string name,  uint8_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    bool sawtoothSource(std::string name,   int8_t frequency) { return sawtoothSource(name, (int32_t)frequency); }

    bool triangleSource(std::string name,    float frequency);
    bool triangleSource(std::string name,  int32_t frequency);
    bool triangleSource(std::string name,   double frequency) { return triangleSource(name,   (float)frequency); }
    bool triangleSource(std::string name, uint32_t frequency) { return triangleSource(name, (int32_t)frequency); }
    bool triangleSource(std::string name, uint16_t frequency) { return triangleSource(name, (int32_t)frequency); }
    bool triangleSource(std::string name,  int16_t frequency) { return triangleSource(name, (int32_t)frequency); }
    bool triangleSource(std::string name,  uint8_t frequency) { return triangleSource(name, (int32_t)frequency); }
    bool triangleSource(std::string name,   int8_t frequency) { return triangleSource(name, (int32_t)frequency); }

    bool audioDelay(std::string name,     float delayMs);
    bool EQfirstOrder(std::string name,   firstOrderEQ_t &equalizer);
    bool EQsecondOrder(std::string name,  secondOrderEQ_t &equalizer);
    bool toneControl(std::string name,    toneCtrl_t &toneCtrl);
    bool stateVariable(std::string name,  float freq, float q);
    bool compressorRMS(std::string name,  compressor_t &compressor);
    bool compressorPeak(std::string name, compressor_t &compressor);

    // Hardware functions
    void muteADC(bool mute);
    void muteDAC(bool mute);

    // Data conversion
    void floatToFixed(float value, uint8_t *buffer);
    void intToFixed(int32_t value, uint8_t *buffer);
    int32_t floatToInt(float value);

    // DSP data write methods

    // Template wrapper for safeload_writeRegister
    // Handles both int 28.0 and float 5.23 formatted parameters
    // Usage: safeload_write(uint16_t startMemoryAddress, data1, data2, ..., dataN);
    template <typename Address, typename Data1, typename... DataN>
    void safeload_write(const Address &address, const Data1 &data1, const DataN &...dataN);

    void safeload_writeRegister(uint16_t memoryAddress, uint8_t *data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress,  int32_t data, bool finished);
    void safeload_writeRegister(uint16_t memoryAddress,    float data, bool finished);
    #if defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32) || 1
    void safeload_writeRegister(uint16_t memoryAddress,  int16_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
    #else
    void safeload_writeRegister(uint16_t memoryAddress,      int data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
    #endif
    void safeload_writeRegister(uint16_t memoryAddress, uint16_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
    void safeload_writeRegister(uint16_t memoryAddress,  uint8_t data, bool finished) { safeload_writeRegister(memoryAddress, (int32_t)data, finished); }
    void safeload_writeRegister(uint16_t memoryAddress,   double data, bool finished) { safeload_writeRegister(memoryAddress,   (float)data, finished); }
    void writeRegister(uint16_t memoryAddress, uint16_t length, uint8_t *data);
    void writeRegister(uint16_t memoryAddress, uint16_t length, const uint8_t *data);
    void writeRegisterBlock(uint16_t memoryAddress, uint16_t length, const uint8_t *data, uint8_t registerSize);
    int32_t readBack(uint16_t address, uint16_t value, uint8_t length);
    uint32_t readRegister(dspRegister hwReg, uint8_t numberOfBytes);


  private:
    // Wrapper template functions for safeload template
    template <typename Data1, typename... DataN>
    void safeload_write_wrapper(const Data1 &data1, const DataN &...dataN)
    {
      safeload_writeRegister(_dspRegAddr, data1, false);
       _dspRegAddr++;
      safeload_write_wrapper(dataN...);  // Recursive call using pack expansion syntax
    }
    // Handles last argument
    template <typename Data1>
    void safeload_write_wrapper(const Data1& data1)
    {
      safeload_writeRegister(_dspRegAddr, data1, true);
    }

    // Math
    void linspace(float x1, float x2, float n, float *vect);

    // Objects
    int _i2c_fd;
    
    // Private constants
    const uint8_t _dspAddress; // Passed device i2c address
    const uint8_t _deviceType; // Passed device type
    const int8_t  _resetPin;   // Digital pin to reset the DSP
    
    // Private variables
    uint16_t _dspRegAddr;      // Used by template safeload functions

    tinyxml2::XMLDocument _xmlDoc;
    std::array<uint8_t, 2> _coreRegStart;
    std::array<uint8_t, 5120> _progData;
    std::array<uint8_t, 4096> _paramReg;
    std::array<uint8_t, 24> _hwConfigReg;
    std::array<uint8_t, 2> _coreRegEnd;
    class Param {
      public:
        enum ParamType {Gain,Volume,Delay,Mux, Demux, DynamicBass, HardClip, SoftClip, DcSource, SineSource, SawtoothSource, TriangleSource, EqFirstOrder, EqSecondOrder, ToneControl, StateVariable, CompressorRMS, CompressorMax};
        Param() {}
        Param(enum ParamType type,uint16_t addr, int qty) {this->type = type; this->addr = addr; this->qty = qty;}
        uint16_t addr;
        ParamType type;
        int qty;
    };
    std::map<std::string, SigmaDSP::Param> _paramMap;
};


#endif //SIGMADSP_H
