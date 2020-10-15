#include "SigmaDSP.h"
#include <cmath>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <linux/i2c.h>

/*******************************************************************************************
**                                      PUBLIC METHODS                                    **
*******************************************************************************************/

/***************************************
Function: SigmaDSP()
Purpose:  Constructor of the class.
Inputs:   uint8_t i2cAddress; DSP i2c address
          uint8_t device;     The DSP type, e.g ADAU1701
          uint8_t resetPin;   pin to reset DSP (optional parameter)
Returns:  None
***************************************/
SigmaDSP::SigmaDSP(int adapter_nr, uint8_t i2cAddress)
  : _dspAddress(i2cAddress)
{
  int file;
  char filename[20];

  snprintf(filename, 19, "/dev/i2c-%d", adapter_nr);
  _i2c_fd = open(filename, O_RDWR);
  if (_i2c_fd < 0) {
    exit(1);
  }
  if (ioctl(_i2c_fd, I2C_SLAVE, _dspAddress) < 0) {
    exit(1);
  }
}


/***************************************
Function: begin()
Purpose:  Starts the i2c interface
Inputs:   TwoWire Wire;     Wire object (optional parameter)
Returns:  None
***************************************/
void SigmaDSP::begin(int adapter_nr)
{
  #if 1
  writeRegister(2076, _coreRegStart.size(), _coreRegStart.data());
  writeRegister(1024, _progData.size(),     _progData.data());
  writeRegister(   0, _paramReg.size(),     _paramReg.data());
  writeRegister(2076, _hwConfigReg.size(),  _hwConfigReg.data());
  writeRegister(2076, _coreRegEnd.size(),   _coreRegEnd.data());
  #endif
}

#define READ_XML_DATA(XMLNODE, ARR) \
    { \
        const char* str = XMLNODE->FirstChildElement("Data")->GetText(); \
        for(int i=0;i<(int)ARR.size();i++) { \
            ARR[i] = std::stoul(str,NULL,0); \
            str+=6; \
        } \
    }

#define NULL_CHECK(PTR, ERR_MSG) \
    if(!PTR) { \
        printf(ERR_MSG); \
        return; \
    }

void SigmaDSP::loadXml(const char* filename)
{
    using namespace tinyxml2;
    _xmlDoc.LoadFile(filename);
    XMLNode* schematic = _xmlDoc.FirstChildElement("Schematic");
    XMLNode* ic = schematic->FirstChildElement("IC");
    XMLNode* coreRegStartNode = ic->FirstChildElement("Register");
    READ_XML_DATA(coreRegStartNode, _coreRegStart);
    XMLNode* progDataNode = ic->FirstChildElement("Program");
    READ_XML_DATA(progDataNode, _progData);
    XMLNode* paramRegNode = coreRegStartNode->NextSiblingElement("Register");
    READ_XML_DATA(paramRegNode, _paramReg);
    XMLNode* hwConfigNode = paramRegNode->NextSiblingElement("Register");
    READ_XML_DATA(hwConfigNode, _hwConfigReg);
    XMLNode* coreRegEndNode = hwConfigNode->NextSiblingElement("Register");
    READ_XML_DATA(coreRegEndNode, _coreRegEnd);
    for(XMLNode* mod = ic->FirstChildElement("Module"); mod != NULL; mod = mod->NextSiblingElement("Module")) {
        QString cellName = mod->FirstChildElement("CellName")->GetText();
        XMLNode* algo = mod->FirstChildElement("Algorithm"); 
        std::string algoName = algo->FirstChildElement("DetailedName")->GetText();
        int param_count=0;
        uint16_t addr=0xFFFF;
        for(XMLNode* param = algo->FirstChildElement("ModuleParameter"); param !=NULL; param = param->NextSiblingElement("ModuleParameter")) {
            std::string paramName = param->FirstChildElement("Name")->GetText();
            uint16_t addr_tmp = std::stoul(param->FirstChildElement("Address")->GetText());
            if(addr_tmp < addr) {
                addr = addr_tmp;
            }
            param_count++;
        }
        if( algoName.rfind("SWGain1940DB",0) != std::string::npos ) {
            _paramMap[cellName] = SigmaDSP::Param(SigmaDSP::Param::ParamType::Volume, addr, param_count/2);
            printf("Volume : %s\n",cellName.toStdString().c_str());
        } else if(algoName.rfind("Gain1940",0) != std::string::npos || algoName.rfind("StereoMixer",0) != std::string::npos || algoName.rfind("MultCtrlMixer",0) != std::string::npos ) {
            _paramMap[cellName] = SigmaDSP::Param(SigmaDSP::Param::ParamType::Gain, addr, param_count);
            printf("Gain (%d): %s\n",param_count, cellName.toStdString().c_str());
        } else if(algoName.rfind("EQ1940",0) != std::string::npos ) {
            _paramMap[cellName] = SigmaDSP::Param(SigmaDSP::Param::ParamType::EqSecondOrder, addr, param_count/5);
            printf("EqSecondOrder (%d) : %s\n",param_count/5,cellName.toStdString().c_str());
        } else if(algoName.rfind("MultCtrlDelGrow",0) != std::string::npos ) {
            _paramMap[cellName] = SigmaDSP::Param(SigmaDSP::Param::ParamType::Delay, addr, param_count/1);
            printf("Delay  : %s\n",cellName.toStdString().c_str());
        } else if(algoName.rfind("stereomux1940",0) != std::string::npos || algoName.rfind("monomux1940",0) != std::string::npos) {
            _paramMap[cellName] = SigmaDSP::Param(SigmaDSP::Param::ParamType::Mux, addr, param_count);
            printf("Mux  : %s\n",cellName.toStdString().c_str());
        } else {
            printf("Unknown  : %s : %s\n",algoName.c_str(),cellName.toStdString().c_str());
        }
    }
}

/***************************************
Function: mux()
Purpose:  Controls a mux. The mux can only be a single address block, which
          is the clickless SW slew mux
Inputs:   QString name;  DSP memory address
          uint8_t index;                Index (the signal) you want to switch to (0, 1, 2, ...)
          uint8_t numberOfIndexes;      The total number of indexes (2 or more)
Returns:  None
***************************************/
bool SigmaDSP::demux(QString name, int index)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::Demux ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  // Index number is actually not needed, but kept for compatibility with demux

  safeload_write(startMemoryAddress, index);
  return true;
}


/***************************************
Function: demux()
Purpose:  Controls a demux.
Inputs:   QString name;  DSP memory address
          uint8_t index;                Index (the signal) you want to switch to (0, 1, 2, ...)
          uint8_t numberOfIndexes;      The total number of indexes (2 or more)
Returns:  None
***************************************/
bool SigmaDSP::mux(QString name, int index)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::Mux ) {
    return false;
  }
  int numberOfIndexes = it->second.qty;
  if(index < 0 || index >= numberOfIndexes || numberOfIndexes > 5) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  int32_t arr[5] = {0,0,0,0,0};
  arr[index] = 0x00800000;
  switch(numberOfIndexes) {
    case 2:
        safeload_write(startMemoryAddress, arr[0], arr[1]);
        break;
    case 3:
        safeload_write(startMemoryAddress,  arr[0], arr[1], arr[2]);
        break;
    case 4:
        safeload_write(startMemoryAddress, arr[0], arr[1], arr[2], arr[3]);
        break;
    case 5:
        safeload_write(startMemoryAddress, arr[0], arr[1], arr[2], arr[3], arr[4] );
        break;
  }
  return true;
}


/***************************************
Function: gain()
Purpose:  Adjusts the gain of a channel
Inputs:   QString name; DSP memory address
          gain;                        Actual gain value (1 = 0dB gain)
          uint8_t channels;            Number of channels this gain cell has (default 1)
Returns:  None
***************************************/
bool SigmaDSP::gain(QString name, int idx, float gain, uint8_t channels)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::Gain ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr + idx;
  for(uint8_t i = 0; i < channels - 1; i++)
    safeload_writeRegister(startMemoryAddress++, gain, false);

  safeload_writeRegister(startMemoryAddress, gain, true);
  return true;
}

bool SigmaDSP::gain(QString name, int idx, int32_t gain, uint8_t channels)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::Gain ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr + idx;
  int32_t value = (gain * ((int32_t)1 << 23));

  for(uint8_t i = 0; i < channels - 1; i++)
    safeload_writeRegister(startMemoryAddress++, value, false);

  safeload_writeRegister(startMemoryAddress, value, true);
  return true;
}


/***************************************
Function: volume_slew()
Purpose:  Volume control with slew
Inputs:   QString name; DSP memory address
          float dB;                    Volume to set in dB
          uint8_t slew;                slew rate (optional, default 12)
Returns:  None
***************************************/
bool SigmaDSP::volume_slew(QString name, float dB, uint8_t slew)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::Volume ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float volume = pow(10, dB / 20); // 10^(dB / 20)
  int32_t slewrate = 0x400000 / (1 << (slew - 1)); // 0x400000/2^(slew - 1))

  safeload_write(startMemoryAddress, volume, slewrate);
  return true;
}


/***************************************
Function: dynamicBass()
Purpose:  Controls the boost level of the dynamic bass block
Inputs:   QString name; DSP memory address
          float dB;                    Bass boost level in dB (0 - +20dB)
Returns:  None
***************************************/
bool SigmaDSP::dynamicBass(QString name, float dB)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::DynamicBass ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float boost = pow(10, -dB / 20); // 10^(-dB / 20)

  safeload_write(startMemoryAddress, boost);
  return true;
}


/***************************************
Function: hardClip()
Purpose:  Hard clip with separate negative
          and positive threshold
Inputs:   QString name; DSP memory address
          float highThreshold;         High threshold 0 -> 1.0
          float lowThreshold;          Low threshold -1.0 -> 0
Returns:  None
***************************************/
bool SigmaDSP::hardClip(QString name, float highThreshold, float lowThreshold)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::HardClip ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  safeload_write(startMemoryAddress, highThreshold, lowThreshold);
  return true;
}


 /***************************************
Function: softClip()
Purpose:  Soft clip with adjustable curve.
          Higher alpha -> smoother clipping curve
Inputs:   QString name; DSP memory address
          float alpha;                 Clipping coefficient (0.1 -> 10.0)
Returns:  None
***************************************/
bool SigmaDSP::softClip(QString name, float alpha)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::SoftClip ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  const float oneThird = 0.333;
  const float twoThird = 0.666;

  safeload_write(startMemoryAddress, alpha, (float)1/alpha, oneThird, twoThird);
  return true;
}


/***************************************
Function: dcSource()
Purpose:  This function controls a DC source cell
Inputs:   QString name; DSP memory address
          float level;                 DC value level range +/-1.0
Returns:  None
***************************************/
bool SigmaDSP::dcSource(QString name, float level)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::DcSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  safeload_write(startMemoryAddress, level);
  return true;
}


/***************************************
Function: sineSource()
Purpose:  This function controls a sine source cell
          Use float/double for accuracy, and integer for speed
Inputs:   QString name;   DSP memory address
          frequency;                     Frequency in Hz
Returns:  None
***************************************/
bool SigmaDSP::sineSource(QString name, float frequency)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::SineSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float value = (1.00/24000.00)*frequency;

  safeload_write(startMemoryAddress, 0xff, value, 0x800000);
  return true;
}

bool SigmaDSP::sineSource(QString name, int32_t frequency)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::SineSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  // Multiply frequency by 349.5234375 (ideally 349.525333333)
  int32_t value = (frequency << 9) - (frequency << 7) - (frequency << 5) - (frequency << 1) - (frequency >> 1) + (frequency >> 5) - (frequency >> 7);

  safeload_write(startMemoryAddress, 0xff, value, 0x800000);
  return true;
}


/***************************************
Function: squareSource()
Purpose:  This function controls a square source cell
Inputs:   QString name;   DSP memory address
          frequency;                     Frequency in Hz
Returns:  None
***************************************/
bool SigmaDSP::squareSource(QString name,    float frequency) { return sineSource(name, frequency); }
bool SigmaDSP::squareSource(QString name,  int32_t frequency) { return sineSource(name, frequency); }


/***************************************
Function: sawtoothSource()
Purpose:  This function controls a sawtooth source cell
          Use float/double for accuracy, and integer for speed
Inputs:   QString name;   DSP memory address
          frequency;                     Frequency in Hz
Returns:  None
***************************************/
bool SigmaDSP::sawtoothSource(QString name, float frequency)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::SawtoothSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float value = (0.50/24000.00)*frequency;

  safeload_write(startMemoryAddress, value, 0x800000);
  return true;
}

bool SigmaDSP::sawtoothSource(QString name, int32_t frequency)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::SawtoothSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  // Multiply frequency by 174.75 (ideally 174.762666667)
  int32_t value = (frequency << 8) - (frequency << 6) - (frequency << 4) - frequency - (frequency >> 1) + (frequency >> 2);

  safeload_write(startMemoryAddress, value, 0x800000);
  return true;
}


/***************************************
Function: triangleSource()
Purpose:  This function controls a triangle source cell
          Use float/double for accuracy, and integer for speed
Inputs:   QString name;   DSP memory address
          frequency;                     Frequency in Hz
Returns:  None
***************************************/
bool SigmaDSP::triangleSource(QString name, float frequency)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::TriangleSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float value = (0.50/24000.00)*frequency;

  safeload_write(startMemoryAddress, 0x00, 0x800000, 0x00, -0x800000, 0x03, value, 0x800000);
  return true;
}

bool SigmaDSP::triangleSource(QString name, int32_t frequency)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::TriangleSource ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  // Multiply frequency by 174.75 (ideally 174.762666667)
  int32_t value = (frequency << 8) - (frequency << 6) - (frequency << 4) - frequency - (frequency >> 1) + (frequency >> 2);

  safeload_write(startMemoryAddress, 0x00, 0x800000, 0x00, -0x800000, 0x03, value, 0x800000);
  return true;
}


/***************************************
Function: audioDelay()
Purpose:  This function controls a delay cell
Inputs:   QString name;   DSP memory address
          float delay_ms;                Delay value
Returns:  None

Delay ranges:
0.0-42.6ms  (ADAU170x) @ 48kHz
0.0-21.3ms  (ADAU170x) @ 96kHz
0.0-10.6ms  (ADAU170x) @ 192kHz

WARNING!!! Delays calculated are theoretical assuming you
have 100% data memory available in your Sigma Studio design.
Data memory is shared among other blocks in Sigma Studio so
in practice, this much data memory is not available to the user
because every block in a design uses a few data memory locations
for its processing. The SigmaStudio compiler manages the data
RAM and indicates if the number of addresses needed in the
design exceeds the maximum available.
***************************************/
bool SigmaDSP::audioDelay(QString name, float delayMs)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::Delay ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  int32_t ticks = (int32_t)(delayMs*0.001/(1/FS));

  if(ticks > 2048)
    ticks = 2048;

  safeload_write(startMemoryAddress, ticks);
  return true;
}


/***************************************
Function: EQfirstOrder()
Purpose:  Adjusts a first order EQ
Inputs:   QString name;   DSP memory address
          firstOrderEQ_t &equalizer;     Equalizer param struct
Returns:  None
***************************************/
bool SigmaDSP::EQfirstOrder(QString name, firstOrderEQ_t &equalizer)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::EqFirstOrder ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float w0, gainLinear;
  float b0, b1, a1;
  float coefficients[3];

  w0=2*M_PI*equalizer.freq/FS; //2*PI*freq/FS
  gainLinear = pow(10,(equalizer.gain/20)); //10^(gain/20)

  switch(equalizer.filterType)
  {
// Lowpass
    case parameters::filterType::lowpass:
    default:
      a1 = pow(2.7,-w0);
      b0 = gainLinear * (1.0 - a1);
      b1 = 0;
      break;

// Highpass
    case parameters::filterType::highpass:
      a1 = pow(2.7,-w0);
      b0 = gainLinear * a1;
      b1 = -a1 * gainLinear;
      break;
  }

  if(equalizer.state == parameters::state::on)
  {
    if(equalizer.phase == parameters::phase::nonInverted) // 0 deg
    {
      coefficients[0] = b0;
      coefficients[1] = b1;
      coefficients[2] = a1;
    }
    else //if(equalizer.phase == parameters::phase::inverted) // 180 deg
    {
      coefficients[0] = -1*b0;
      coefficients[1] = -1*b1;
      coefficients[2] = a1; // This coefficient does not change sign
    }
  }
  else //if(equalizer.state == parameters::state::off)
  {
    coefficients[0] = 1.00;
    coefficients[1] = 0.00;
    coefficients[2] = 0.00;
  }

  safeload_write(startMemoryAddress, coefficients[0], coefficients[1], coefficients[2]);
  return true;
}


/***************************************
Function: EQsecondOrder()
Purpose:  Adjusts a second order EQ
Inputs:   QString name;   DSP memory address
          secondOrderEQ_t &equalizer;    Equalizer param struct
Returns:  None
***************************************/
bool SigmaDSP::EQsecondOrder(QString name, int idx, enum filterType ft, float Q, float S, float bandwidth, float boost, float freq, float gain, bool invert, bool state)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::EqSecondOrder ) {
    printf("Invalid name: %s", name.toStdString().c_str());
    return false;
  }
  if(idx < 0 || idx >= it->second.qty) {
    printf("Index %d out of bounds for: %s", idx, name.toStdString().c_str());
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr + idx*5;
  float A, w0, alpha, gainLinear;
  float b0, b1, b2, a0, a1, a2;
  float coefficients[5];

  A=pow(10,(boost/40));           // 10^(boost/40)
  w0=2*M_PI*freq/FS;                // 2*PI*freq/FS
  gainLinear = pow(10,(gain/20)); // 10^(gain/20)

  switch(ft)
  {
// Parametric
    case parameters::filterType::parametric:
// Peaking
    case parameters::filterType::peaking:
    default:
      alpha = sin(w0)/(2*Q);
      a0 =  1 + alpha/A;
      a1 = -2 * cos(w0);
      a2 =  1 - alpha/A;
      b0 = (1 + alpha*A) * gainLinear;
      b1 = -(2 * cos(w0)) * gainLinear;
      b2 = (1 - alpha*A) * gainLinear;
      break;

// Low shelf
    case parameters::filterType::lowShelf:
      alpha=sin(w0)/2*sqrt((A+1/A)*(1/S-1)+2);
      a0 = (A+1)+(A-1)*cos(w0)+2*sqrt(A)*alpha;
      a1 = -2*((A-1)+(A+1)*cos(w0));
      a2 = (A+1)+(A-1)*cos(w0)-2*sqrt(A)*alpha;
      b0 = A*((A+1)-(A-1)*cos(w0)+2*sqrt(A)*alpha)*gainLinear;
      b1 = 2*A*((A-1)-(A+1)*cos(w0)) * gainLinear;
      b2 = A*((A+1)-(A-1)*cos(w0)-2*sqrt(A)*alpha)*gainLinear;
      break;

// High shelf
    case parameters::filterType::highShelf:
      alpha = sin(w0)/2 * sqrt((A + 1/A)*(1/S - 1) + 2);
      a0 = (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha;
      a1 = 2*((A-1) - (A+1)*cos(w0));
      a2 = (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha;
      b0 = A*((A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha) * gainLinear;
      b1 = -2*A*((A-1) + (A+1)*cos(w0)) * gainLinear;
      b2 = A*((A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha) * gainLinear;
      break;

// Lowpass
    case parameters::filterType::lowpass:
      alpha = sin(w0)/(2*Q);
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = (1 - cos(w0)) * (gainLinear/2);
      b1 = (1 - cos(w0))  * gainLinear;
      b2 = (1 - cos(w0)) * (gainLinear/2);
      break;

// Highpass
    case parameters::filterType::highpass:
      alpha = sin(w0)/(2*Q);
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = (1 + cos(w0)) * (gainLinear/2);
      b1 = -(1 + cos(w0)) * gainLinear;
      b2 = (1 + cos(w0)) * (gainLinear/2);
      break;

// Bandpass
    case parameters::filterType::bandpass:
      alpha = sin(w0) * sinh(log(2)/(2 * bandwidth * w0/sin(w0)));
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = alpha * gainLinear;
      b1 = 0;
      b2 = -alpha * gainLinear;
      break;

// Bandstop
    case parameters::filterType::bandstop:
      alpha = sin(w0) * sinh(log(2)/(2 * bandwidth * w0/sin(w0)));
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = 1 * gainLinear;
      b1 = -2*cos(w0) * gainLinear;
      b2 = 1 * gainLinear;
      break;

// Butterworth lowpass
    case parameters::filterType::butterworthLowpass:
      alpha = sin(w0) / 2.0 * 1/sqrt(2);
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = (1 - cos(w0)) * gainLinear / 2;
      b1 = 1 - cos(w0) * gainLinear;
      b2 = (1 - cos(w0)) * gainLinear / 2;
      break;

// Butterworth highpass
    case parameters::filterType::butterworthHighpass:
      alpha = sin(w0) / 2.0 * 1/sqrt(2);
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = (1 + cos(w0)) * gainLinear / 2;
      b1 = -(1 + cos(w0)) * gainLinear;
      b2 = (1 + cos(w0)) * gainLinear / 2;
      break;

// Bessel lowpass
    case parameters::filterType::besselLowpass:
      alpha = sin(w0) / 2.0 * 1/sqrt(3) ;
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = (1 - cos(w0)) * gainLinear / 2;
      b1 = 1 - cos(w0) * gainLinear;
      b2 = (1 - cos(w0)) * gainLinear / 2;
      break;

// Bessel highpass
    case parameters::filterType::besselHighpass:
      alpha = sin(w0) / 2.0 * 1/sqrt(3) ;
      a0 = 1 + alpha;
      a1 = -2*cos(w0);
      a2 = 1 - alpha;
      b0 = (1 + cos(w0)) * gainLinear / 2;
      b1 = -(1 + cos(w0)) * gainLinear;
      b2 = (1 + cos(w0)) * gainLinear / 2;
      break;
  }

  // For Sigma DSP implementation we need to normalize all the coefficients respect to a0
  // and inverting by sign a1 and a2
  if(a0 != 0.00 && state)
  {
    if(!invert) // 0 deg
    {
      coefficients[0] = b0/a0;
      coefficients[1] = b1/a0;
      coefficients[2] = b2/a0;
      coefficients[3] = -1*a1/a0;
      coefficients[4] = -1*a2/a0;
    }
    else //if(phase == parameters::phase::inverted) // 180 deg
    {
      coefficients[0] = -1*b0/a0;
      coefficients[1] = -1*b1/a0;
      coefficients[2] = -1*b2/a0;
      coefficients[3] = -1*a1/a0; // This coefficient does not change sign!
      coefficients[4] = -1*a2/a0; // This coefficient does not change sign!
    }
  }
  else //if(state == parameters::state::off)
  {
    coefficients[0] = 1.00;
    coefficients[1] = 0;
    coefficients[2] = 0;
    coefficients[3] = 0;
    coefficients[4] = 0;
  }

  safeload_write(startMemoryAddress, coefficients[0], coefficients[1], coefficients[2], coefficients[3], coefficients[4]);
  return true;
}


/***************************************
Function: toneControl()
Purpose:  This function manages a baxandall low-high dual tone control
Inputs:   QString name;   DSP memory address
          toneCtrl_t &toneCtrl;          Tone control param struct
Returns:  None
***************************************/
bool SigmaDSP::toneControl(QString name, toneCtrl_t &toneCtrl)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::ToneControl ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float tb,bb,wT,wB,Knum_T,Kden_T,Knum_B,Kden_B,alpha0,beta1,alpha1,beta2,alpha2,beta3,alpha3,beta4;
  float b0,b1,b2,a0,a1,a2;
  float coefficients[5];

  tb = pow(10, toneCtrl.boost_Treble_dB / 20.0);
  bb = pow(10, toneCtrl.boost_Bass_dB / 20.0);

  wT = tan(M_PI * toneCtrl.freq_Treble / FS);
  wB = tan(M_PI * toneCtrl.freq_Bass / FS);

  Knum_T = 2 / (1 + (1.0 / tb));
  Kden_T = 2 / (1 + tb);
  Knum_B = 2.0 / (1.0 + (1.0 / bb));
  Kden_B = 2.0 / (1.0 + bb);

  alpha0 = wT + Kden_T;
  beta1 = wT + Knum_T;
  alpha1 = wT - Kden_T;
  beta2 = wT - Knum_T;

  alpha2 = (wB*Kden_B) + 1;
  beta3 = (wB*Knum_B) - 1;
  alpha3 = (wB*Kden_B) - 1;
  beta4 = (wB*Knum_B) + 1;

  a0 = alpha0 * alpha2;
  a1 = (alpha0 * alpha3) + (alpha1 * alpha2);
  a2 = alpha1 * alpha3;
  b0 = beta1 * beta3;
  b1 = (beta1 * beta4) + (beta2 * beta3);
  b2 = beta2 * beta4;

  // For Sigma DSP implementation we need to normalize all the coefficients respect to a0
  // and inverting a1 and a2 by sign
  if(a0 != 0.00 && toneCtrl.state == parameters::state::on)
  {
    if(toneCtrl.phase == parameters::phase::nonInverted) // 0 deg
    {
      coefficients[0] = b0/a0;
      coefficients[1] = b1/a0;
      coefficients[2] = b2/a0;
      coefficients[3] = -1*a1/a0;
      coefficients[4] = -1*a2/a0;
    }
    else //if(toneCtrl.phase == parameters::phase::inverted) // 180 deg
    {
      coefficients[0] = -1*b0/a0;
      coefficients[1] = -1*b1/a0;
      coefficients[2] = -1*b2/a0;
      coefficients[3] = a1/a0;
      coefficients[4] = a2/a0;
    }
  }
  else //if(toneCtrl.state == parameters::state::off)
  {
    coefficients[0] = 1.00;
    coefficients[1] = 0;
    coefficients[2] = 0;
    coefficients[3] = 0;
    coefficients[4] = 0;
  }

  safeload_write(startMemoryAddress, coefficients[0], coefficients[1], coefficients[2], coefficients[3], coefficients[4]);
  return true;
}


/**
 * This function control a state variable filter block
 * @param dspAddress - the physical I2C address (7-bit format)
 * @param address - the param address of the cell
 * @param frequency - frequency range: 1-19148 Hz
 * @param q - q range: 1.28:10
 */
bool SigmaDSP::stateVariable(QString name, float freq, float q)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::StateVariable ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  float param1 = 2*sin(M_PI*freq/FS);
  float param2 = 1/q;

  safeload_write(startMemoryAddress, param1, param2);
  return true;
}


/***************************************
Function: CompressorRMS()
Purpose:  This function calculates the curve and the other parameters of a rms compressor block
          Set ratio = 1 to disable compressor
Inputs:   QString name;   DSP memory address
          compressor_t &compressor;      Compressor param struct
Returns:  None
***************************************/
bool SigmaDSP::compressorRMS(QString name, compressor_t &compressor)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::CompressorRMS ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  uint8_t i,count;

  float curve[34];
  float x[34];
  float y[34];
  float delta = 0.00;
  float coeff = 0.00;

  float dbps = 0.00;
  float attack_par = 0.00;
  float hold_par = 0.00;
  float decay_par = 0.00;
  float postgain_par = 0.00;

  linspace(-90, 6, 34, x);

  count = 0;

  coeff = 1/compressor.ratio;
  for(i = 0; i < 34; i++) // This algorithm creates the curve with 1 1 1 1 1 1 1 0.9 0.8 0.72 0.64 and so on coefficients
  {
    if(x[i] >= compressor.threshold)
    {
      count++;
      if(count == 1)
        delta = x[i]*coeff-x[i]*1;
      y[i] = (x[i] * coeff)-delta;
    }
    else
      y[i] = x[i] * 1;
  }

  for(i = 0; i < 34; i++)  // Coefficients of the curve calculation
    curve[i] = (pow(10, y[i]/20)) / (pow(10, x[i]/20));  // Coefficients are the ratio between the linearized values of vect. y and x

  // Create buffer to store converted data
  uint8_t storeData[5];

  // Parameter load into Sigma DSP
  for(i = 0; i < 34; i++)
  {
    floatToFixed(curve[i], storeData);
    safeload_writeRegister(startMemoryAddress++, storeData, false);
  }

  // Conversion dBps -> ms
  // dBps = 121;
  // TCms = (20/(dBps*2.3))*1000

  // RMS TC (dB/s)
  dbps = (20/(compressor.attack*2.3))*1e3;
  attack_par = abs(1.0 - pow(10,(dbps/(10*FS))));
  floatToFixed(attack_par, storeData);
  safeload_writeRegister(startMemoryAddress++, storeData, true);

  #ifdef COMPRESSORWITHPOSTGAIN
    postgain_par = pow(10, compressor.postgain/40);
    floatToFixed(postgain_par, storeData);
    safeload_writeRegister(startMemoryAddress++, storeData, true);
  #endif

  // Hold
  hold_par = compressor.hold*FS/1000;
  floatToFixed(hold_par, storeData);
  safeload_writeRegister(startMemoryAddress++, storeData, true);

  // Decay (dB/s)
  dbps = (20/(compressor.decayMs*2.3))*1e3;
  decay_par = dbps/(96*FS);
  floatToFixed(decay_par, storeData);
  safeload_writeRegister(startMemoryAddress, storeData, true);
  return true;
}


/***************************************
Function: CompressorPeak()
Purpose:  This function calculates the curve and the other parameters of a peak compressor block
          Set ratio = 1 to disable compressor
Inputs:   QString name;   DSP memory address
          compressor_t &compressor;      Compressor param struct
Returns:  None
***************************************/
bool SigmaDSP::compressorPeak(QString name, compressor_t &compressor)
{
  auto it = _paramMap.find(name);
  if( it == _paramMap.end() || it->second.type != SigmaDSP::Param::ParamType::CompressorMax ) {
    return false;
  }
  uint16_t startMemoryAddress = it->second.addr;
  uint8_t i,count;

  float curve[33];
  float x[33];
  float y[33];
  float delta = 0.00;
  float coeff = 0.00;

  float dbps = 0.00;
  //float attack_par = 0.00;
  float hold_par = 0.00;
  float decay_par = 0.00;
  float postgain_par = 0.00;

  linspace(-90, 6, 33, x);

  count = 0;

  coeff = 1/compressor.ratio;
  for(i = 0; i < 33; i++) // This algorithm creates the curve with 1 1 1 1 1 1 1 0.9 0.8 0.72 0.64 and so on coefficients
  {
    if(x[i] >= compressor.threshold)
    {
      count++;
      if(count == 1)
        delta = x[i]*coeff-x[i]*1;
      y[i] = (x[i] * coeff)-delta;
    }
    else
      y[i] = x[i] * 1;
  }

  for(i = 0; i < 33; i++)  // Coefficients of the curve calculation
    curve[i] = (pow(10, y[i]/20)) / (pow(10, x[i]/20));  // Coefficients are the ratio between the linearized values of vect. y and x

  // Create buffer to store converted data
  uint8_t storeData[5];

  // Parameter load into Sigma DSP
  for(i = 0; i < 33; i++)
  {
    floatToFixed(curve[i], storeData);
    safeload_writeRegister(startMemoryAddress++, storeData, false);
  }

  #ifdef COMPRESSORWITHPOSTGAIN
    postgain_par = pow(10, compressor.postgain/40);
    floatToFixed(postgain_par, storeData);
    safeload_writeRegister(startMemoryAddress++, storeData, true);
  #endif

  // Hold
  hold_par = compressor.hold*FS/1000;
  floatToFixed(hold_par, storeData);
  safeload_writeRegister(startMemoryAddress++, storeData, true);

  // Decay (dB/s)
  dbps = (20/(compressor.decayMs*2.3))*1e3;
  decay_par = dbps/(96*FS);
  floatToFixed(decay_par, storeData);
  safeload_writeRegister(startMemoryAddress, storeData, true);
  return true;
}


/***************************************
Function: muteADC()
Purpose:  Mutes the internal ADC in the DSP
Inputs:   bool mute;   True if muted, false if unmuted
Returns:  None
***************************************/
void SigmaDSP::muteADC(bool mute)
{
  // Read two bytes from the DSP core register
  uint16_t coreRegister = readRegister(dspRegister::CoreRegister, 2);

  uint8_t muteADCdata[2];

  muteADCdata[0] = (coreRegister >> 8) & 0xFF;

  // Set or clear only the DAC mute/unmute bit
  if(mute == true)
    muteADCdata[1] = coreRegister & 0xEF;
  else
    muteADCdata[1] = coreRegister | 0x10;

  writeRegister(dspRegister::CoreRegister, sizeof(muteADCdata), muteADCdata);
}


/***************************************
Function: muteDAC()
Purpose:  Mutes the internal DAC in the DSP
Inputs:   bool mute;   True if muted, false if unmuted
Returns:  None
***************************************/
void SigmaDSP::muteDAC(bool mute)
{
  // Read two bytes from the DSP core register
  uint16_t coreRegister = readRegister(dspRegister::CoreRegister, 2);

  uint8_t muteDACdata[2];

  muteDACdata[0] = (coreRegister >> 8) & 0xFF;

  // Set or clear only the DAC mute/unmute bit
  if(mute == true)
    muteDACdata[1] = coreRegister & 0xF7;
  else
    muteDACdata[1] = coreRegister | 0x08;

  writeRegister(dspRegister::CoreRegister, sizeof(muteDACdata), muteDACdata);
}


template <typename Address, typename Data1, typename... DataN>
void SigmaDSP::safeload_write(const Address &address, const Data1 &data1, const DataN &...dataN)
{
  // Store passed address
  _dspRegAddr = address;

  safeload_write_wrapper(data1, dataN...);
}


/***************************************
Function: safeload_writeRegister()
Purpose:  Writes 5 bytes of data to the parameter memory of the DSP, the first byte is 0x00
Inputs:   uint16_t startMemoryAddress;   DSP memory address
          uint8_t *data;                 Data array to write
          bool finished;                 Indicates if this is the last packet or not
Returns:  None
***************************************/
void SigmaDSP::safeload_writeRegister(uint16_t memoryAddress, uint8_t *data, bool finished)
{
  static uint8_t _safeload_count = 0; // Keeps track of the safeload count
  
  uint8_t addr[2]; // Address array

  addr[0] = (memoryAddress >> 8) & 0xFF;
  addr[1] = memoryAddress & 0xFF;

  writeRegister(dspRegister::SafeloadAddress0 + _safeload_count, sizeof(addr), addr); // Place passed 16-bit memory address in safeload address area

  // Q: Why is the safeload registers five bytes long, while I'm loading four-byte parameters into the RAM using these registers?
  // A: The safeload registers are also used to load the slew RAM data, which is five bytes long. For parameter RAM writes using safeload,
  // the first byte of the safeload register can be set to 0x00.

  // Needs 5 bytes of data
  writeRegister(dspRegister::SafeloadData0 + _safeload_count, 5, data); // Placed passed data (5 bytes) in the next safeload data space

  _safeload_count++; // Increase counter

  if(finished == true || _safeload_count >= 5) // Max 5 safeload memory registers
  {
    addr[0] = 0x00;
    addr[1] = 0x3C; // Set the IST bit (initiate safeload transfer bit)
    writeRegister(dspRegister::CoreRegister, sizeof(addr), addr); // Load content from the safeload registers
    _safeload_count = 0;
  }
}

void SigmaDSP::safeload_writeRegister(uint16_t memoryAddress, int32_t data, bool finished)
{
  uint8_t dataArray[5];
  intToFixed(data, dataArray);
  safeload_writeRegister(memoryAddress, dataArray, finished);
}

void SigmaDSP::safeload_writeRegister(uint16_t memoryAddress, float data, bool finished)
{
  uint8_t dataArray[5];
  floatToFixed(data, dataArray);
  safeload_writeRegister(memoryAddress, dataArray, finished);
}


/***************************************
Function: writeRegister()
Purpose:  Writes data to the DSP
          (max 32 bytes due to i2c buffer size)
Inputs:   uint16_t startMemoryAddress;   DSP memory address
          uint8_t length;                Number of bytes to write
          uint8_t *data;                 Data array to write
Returns:  None
***************************************/
void SigmaDSP::writeRegister(uint16_t memoryAddress, uint16_t length, uint8_t *data)
{
  /*uint8_t LSByte = (uint8_t)memoryAddress & 0xFF;
  uint8_t MSByte = memoryAddress >> 8;

  _WireObject->beginTransmission(_dspAddress); // Begin write

  _WireObject->write(MSByte); // Send high address
  _WireObject->write(LSByte); // Send low address

  for(uint8_t i = 0; i < length; i++)
    _WireObject->write(data[i]); // Send all bytes in passed array

  _WireObject->endTransmission(); // Write out data to I2C and stop transmitting*/
  writeRegisterBlock(memoryAddress, length, data, 32);
}


/***************************************
Function: writeRegister()
Purpose:  Sends data to the DSP from PROGMEM
          (max 32 bytes due to i2c buffer size)
Inputs:   uint16_t startMemoryAddress;   DSP memory address
          uint8_t length;                Number of bytes to write
          const uint8_t *data;           Data array to write
Returns:  None
***************************************/
void SigmaDSP::writeRegister(uint16_t memoryAddress, uint16_t length, const uint8_t *data)
{
  /*uint8_t LSByte = (uint8_t)memoryAddress & 0xFF;
  uint8_t MSByte = memoryAddress >> 8;

  _WireObject->beginTransmission(_dspAddress); // Begin write

  _WireObject->write(MSByte); // Send high address
  _WireObject->write(LSByte); // Send low address

  for(uint8_t i = 0; i < length; i++)
    _WireObject->write(pgm_read_byte(&data[i])); // Send all bytes in passed array

  _WireObject->endTransmission(); // Write out data to I2C and stop transmitting*/
  writeRegisterBlock(memoryAddress, length, data, length);
}


/***************************************
Function: writeRegisterBlock()
Purpose:  Writes data to the DSP from PROGMEM
Inputs:   uint16_t startMemoryAddress;   DSP memory address
          uint16_t length;               Number of bytes to write
          const uint8_t *data;           Data array to write
          uint8_t registerSize;          Number of bytes each register can hold
Returns:  None
***************************************/
void SigmaDSP::writeRegisterBlock(uint16_t memoryAddress, uint16_t length, const uint8_t *data, uint8_t registerSize)
{
/*  uint8_t MSByte = 0;
  uint8_t LSByte = 0;
  uint16_t bytesSent = 0;

  // Run until all bytes are sent
  while(bytesSent < length)
  {
    // Convert address to 8-bit
    MSByte = memoryAddress >> 8;
    LSByte = (uint8_t)memoryAddress & 0xFF;

    _WireObject->beginTransmission(_dspAddress);
    _WireObject->write(MSByte); // Send high address
    _WireObject->write(LSByte); // Send low address
    for(uint8_t i = 0; i < registerSize; i++) // Send n bytes
    {
      _WireObject->write(pgm_read_byte(&data[bytesSent]));
      bytesSent++;
    }
    _WireObject->endTransmission();

    memoryAddress++; // Increase address
  }*/
  uint16_t bytesSent = 0;
  std::vector<uint8_t> writeData(length+2);
  while(bytesSent < length) {
    writeData[0] = memoryAddress >> 8;
    writeData[1] = (uint8_t)memoryAddress & 0xFF;
    for(int i=0;i<length;i++) {
      writeData[i+2] = data[bytesSent];
      bytesSent++;
    }
    write(_i2c_fd, writeData.data(), length+2);
    memoryAddress++;
  }
}


/***************************************
Function: readBack()
Purpose:  reads a DSP block
Inputs:   uint16_t tMemoryAddress;       DSP memory address
          uint16_t readout;              what register to read
          uint8_t numberOfBytes;         How manu bytes to read
Returns:  int32_t value where bytes are concatenated
***************************************/
int32_t SigmaDSP::readBack(uint16_t memoryAddress, uint16_t readout, uint8_t numberOfBytes)
{
  /*uint8_t LSByte = (uint8_t)memoryAddress & 0xFF;
  uint8_t MSByte = memoryAddress >> 8;

  _WireObject->beginTransmission(_dspAddress); // Begin write
  _WireObject->write(MSByte); // Send high address
  _WireObject->write(LSByte); // Send low address
  LSByte = (uint8_t)readout & 0xFF;
  MSByte = readout >> 8;
  _WireObject->write(MSByte); // Send high register to read
  _WireObject->write(LSByte); // Send low register to read
  _WireObject->endTransmission();

  _WireObject->beginTransmission(_dspAddress);
  LSByte = (uint8_t)memoryAddress & 0xFF;
  MSByte = memoryAddress >> 8;
  _WireObject->write(MSByte);
  _WireObject->write(LSByte);
  _WireObject->endTransmission(false);

  int32_t returnVal = 0;
  _WireObject->requestFrom(_dspAddress, numberOfBytes);
  for(uint8_t i = 0; i < numberOfBytes; i++)
    returnVal = returnVal << 8 | _WireObject->read();

  return returnVal;*/
  return -1;
}


/***************************************
Function: readRegster()
Purpose:  reads a DSP hardware register
Inputs:   dspRegister reg;       Register address
          uint8_t numberOfBytes; Number of bytes to read from the register
Returns:  uint16_t value;        Value of the register
***************************************/
uint32_t SigmaDSP::readRegister(dspRegister reg, uint8_t numberOfBytes)
{
/*  uint8_t LSByte = (uint8_t)reg & 0xFF;
  uint8_t MSByte = reg >> 8;

  _WireObject->beginTransmission(_dspAddress); // Begin write
  _WireObject->write(MSByte); // Send high address
  _WireObject->write(LSByte); // Send low address
  _WireObject->endTransmission(false);

  uint32_t returnVal = 0;
  _WireObject->requestFrom(_dspAddress, numberOfBytes);
  for(uint8_t i = 0; i < numberOfBytes; i++)
    returnVal = returnVal << 8 | _WireObject->read();

  return returnVal;*/

  uint8_t MSByte = reg >> 8;
  uint8_t LSByte = (uint8_t)reg & 0xFF;
  std::vector<uint8_t> writeData(2);
  std::vector<uint8_t> readData(numberOfBytes);
  std::vector<struct i2c_msg> msgs;
  writeData[0] = MSByte;
  writeData[1] = MSByte;
  msgs[0].addr = _dspAddress;
  msgs[0].flags = 0x00;
  msgs[0].len = 2;
  msgs[0].buf = writeData.data();
  msgs[1].addr = _dspAddress;
  msgs[1].flags = 0x01;
  msgs[1].len = numberOfBytes;
  msgs[1].buf = readData.data();
  struct i2c_rdwr_ioctl_data data;
  data.msgs = msgs.data();
  data.nmsgs = 2;
  ioctl(_i2c_fd, I2C_RDWR, data);
  uint32_t returnVal = 0;
  for(uint8_t i = 0; i < numberOfBytes; i++) {
    returnVal = returnVal << 8 | readData[i];
  }
  return returnVal;
}


/***************************************
Function: floatTofixed()
Purpose:  Converts a 5.23 float value to 5-byte HEX and stores it to a buffer
Inputs:   float value;      Value to convert
          uint8_t *buffer;  Buffer to store the converted data to
Returns:  None
***************************************/
void SigmaDSP::floatToFixed(float value, uint8_t *buffer)
{
  // Convert float to 4 byte hex
  int32_t fixedval = (value * ((int32_t)1 << 23));

  // Store the 4 bytes in the passed buffer
  buffer[0] = 0x00; // First must be empty
  buffer[1] = (fixedval >> 24) & 0xFF;
  buffer[2] = (fixedval >> 16) & 0xFF;
  buffer[3] = (fixedval >>  8) & 0xFF;
  buffer[4] = fixedval & 0xFF;
}


/***************************************
Function: intToFixed()
Purpose:  Converts a 28.0 integer value to 5-byte HEX and stores it to a buffer
Inputs:   int32_t value;      Value to convert
          uint8_t *buffer;    Buffer to store the converted data to
Returns:  None
***************************************/
void SigmaDSP::intToFixed(int32_t value, uint8_t *buffer)
{
  // Store the 4 bytes in the passed buffer
  buffer[0] = 0x00; // First must be empty
  buffer[1] = (value >> 24) & 0xFF;
  buffer[2] = (value >> 16) & 0xFF;
  buffer[3] = (value >>  8) & 0xFF;
  buffer[4] = value & 0xFF;
}


/***************************************
Function: floatToInt()
Purpose:  Converts a 5.23 float value to int 28.0
Inputs:   float value;    Value to convert
Returns:  int32_t;        Converted value
***************************************/
int32_t SigmaDSP::floatToInt(float value)
{
  // Convert float 5.23 to int 28.0
  return (value * ((int32_t)1 << 23));
}


/*******************************************************************************************
**                                     PRIVATE METHODS                                    **
*******************************************************************************************/


/***************************************
Function: linspace()
Purpose:  Calculates the points of a linearly spaced vector
Inputs:   float x1;     Start point
          float x2;     End point
          float n;      number of points to calculate
          float *vect;  Array to store calculated data
Returns:  None
***************************************/
void SigmaDSP::linspace(float x1, float x2, float n, float *vect)
{
  float k = (abs(x1) + abs(x2))/(n);

  for(uint8_t i = 0; i < n; i++)
    vect[i] = x1+(k*i);
}
