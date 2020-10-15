#ifndef SIGMADSP_H
#define SIGMADSP_H

#include <cstdint>
#include <linux/i2c-dev.h>
#include <array>
#include <map>
#include <QString>
#include <QObject>
#include "parameters.h"
#include "tinyxml2.h"

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

class SigmaDSP: public QObject
{
  Q_OBJECT

  public:
    // Store passed device type and i2c address to private constants
    SigmaDSP(int adapter_nr, uint8_t i2cAddress);

    // Init and setup
    void begin(int adapter_nr=1);
    void loadXml(const char* filename);

    // DSP functions
    Q_INVOKABLE bool mux(QString name,        int index);
    Q_INVOKABLE bool demux(QString name,      int index);
    Q_INVOKABLE bool gain(QString name,       int idx, float gainVal, uint8_t channels = 1);
    Q_INVOKABLE bool gain(QString name,       int idx, int32_t gainVal, uint8_t channels = 1);

    Q_INVOKABLE bool volume_slew(QString name           , float db,        uint8_t slew = 12);
    Q_INVOKABLE bool dynamicBass(QString name, float dB);
    Q_INVOKABLE bool hardClip(QString name,     float highThreshold, float lowThreshold);
    Q_INVOKABLE bool softClip(QString name,     float alpha);
    Q_INVOKABLE bool dcSource(QString name,     float level);

    Q_INVOKABLE bool sineSource(QString name,    float frequency);
    Q_INVOKABLE bool sineSource(QString name,  int32_t frequency);
    Q_INVOKABLE bool sineSource(QString name,   double frequency) { return sineSource(name,   (float)frequency); }
    Q_INVOKABLE bool sineSource(QString name, uint32_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sineSource(QString name, uint16_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sineSource(QString name,  int16_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sineSource(QString name,  uint8_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sineSource(QString name,   int8_t frequency) { return sineSource(name, (int32_t)frequency); }

    Q_INVOKABLE bool squareSource(QString name,    float frequency);
    Q_INVOKABLE bool squareSource(QString name,  int32_t frequency);
    Q_INVOKABLE bool squareSource(QString name,   double frequency) { return sineSource(name,   (float)frequency); }
    Q_INVOKABLE bool squareSource(QString name, uint32_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool squareSource(QString name, uint16_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool squareSource(QString name,  int16_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool squareSource(QString name,  uint8_t frequency) { return sineSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool squareSource(QString name,   int8_t frequency) { return sineSource(name, (int32_t)frequency); }

    Q_INVOKABLE bool sawtoothSource(QString name,    float frequency);
    Q_INVOKABLE bool sawtoothSource(QString name,  int32_t frequency);
    Q_INVOKABLE bool sawtoothSource(QString name,   double frequency) { return sawtoothSource(name,   (float)frequency); }
    Q_INVOKABLE bool sawtoothSource(QString name, uint32_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sawtoothSource(QString name, uint16_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sawtoothSource(QString name,  int16_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sawtoothSource(QString name,  uint8_t frequency) { return sawtoothSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool sawtoothSource(QString name,   int8_t frequency) { return sawtoothSource(name, (int32_t)frequency); }

    Q_INVOKABLE bool triangleSource(QString name,    float frequency);
    Q_INVOKABLE bool triangleSource(QString name,  int32_t frequency);
    Q_INVOKABLE bool triangleSource(QString name,   double frequency) { return triangleSource(name,   (float)frequency); }
    Q_INVOKABLE bool triangleSource(QString name, uint32_t frequency) { return triangleSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool triangleSource(QString name, uint16_t frequency) { return triangleSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool triangleSource(QString name,  int16_t frequency) { return triangleSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool triangleSource(QString name,  uint8_t frequency) { return triangleSource(name, (int32_t)frequency); }
    Q_INVOKABLE bool triangleSource(QString name,   int8_t frequency) { return triangleSource(name, (int32_t)frequency); }

    enum filterType {
        peaking,
        parametric,
        lowShelf,
        highShelf,
        lowpass,
        highpass,
        bandpass,
        bandstop,
        butterworthLowpass,
        butterworthHighpass,
        besselLowpass,
        besselHighpass,
    };
    Q_ENUM(filterType);
    enum phase {
        deg_0       = 0,
        nonInverted = 0,
        deg_180     = 1,
        inverted    = 1,
    };
    Q_ENUM(phase);

    Q_INVOKABLE bool audioDelay(QString name,     float delayMs);
    Q_INVOKABLE bool EQfirstOrder(QString name,   firstOrderEQ_t &equalizer);
    Q_INVOKABLE bool EQsecondOrder(QString name, int idx, enum filterType ft, float Q, float S, float bandwidth, float boost, float freq, float gain, bool invert, bool );
    Q_INVOKABLE bool EQlowpass(QString name, int idx, float gain, bool invert, float freq, float Q) { return EQsecondOrder(name, idx, lowpass, Q, 0, 0, 0, freq, gain, invert, true); };
    Q_INVOKABLE bool EQhighpass(QString name, int idx, float gain, bool invert, float freq, float Q) { return EQsecondOrder(name, idx, highpass, Q, 0, 0, 0, freq, gain, invert, true); };
    Q_INVOKABLE bool EQlowshelf(QString name, int idx, float gain, bool invert, float freq, float slope, float boost) { return EQsecondOrder(name, idx, lowShelf, 0, slope, 0, boost, freq, gain, invert, true); };
    Q_INVOKABLE bool EQhighshelf(QString name, int idx, float gain, bool invert, float freq, float slope, float boost) { return EQsecondOrder(name, idx, highShelf, 0, slope, 0, boost, freq, gain, invert, true); };
    Q_INVOKABLE bool EQbandstop(QString name, int idx, float gain, bool invert, float freq, float bandwidth) { return EQsecondOrder(name, idx, bandpass, 0, 0, bandwidth, 0, freq, gain, invert, true); };
    Q_INVOKABLE bool EQbandpass(QString name, int idx, float gain, bool invert, float freq, float bandwidth) { return EQsecondOrder(name, idx,bandstop , 0, 0, bandwidth, 0, freq, gain, invert, true); };
    Q_INVOKABLE bool EQparametric(QString name, int idx, float gain, bool invert, float freq, float Q, float boost) { return EQsecondOrder(name, idx, parametric, Q, 0, 0, boost, freq, gain, invert, true); }
    Q_INVOKABLE bool EQoff(QString name, int idx) { return EQsecondOrder(name, idx, parametric, 0, 0, 0, 0, 0, 0, phase::deg_0, false); } 
    Q_INVOKABLE bool toneControl(QString name,    toneCtrl_t &toneCtrl);
    Q_INVOKABLE bool stateVariable(QString name,  float freq, float q);
    Q_INVOKABLE bool compressorRMS(QString name,  compressor_t &compressor);
    Q_INVOKABLE bool compressorPeak(QString name, compressor_t &compressor);

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
    std::map<QString, SigmaDSP::Param> _paramMap;
};


#endif //SIGMADSP_H
