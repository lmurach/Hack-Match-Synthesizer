#include <SPI.h>

#define MAX_AMP 32767

uint32_t xorShift = 0;

// tables for DDS      
signed char sineTable[256], fm1, fm2 ;
int indexFM = 0;

// sound
const int CS = 10;
const uint8_t SAMP_SIZE = 100;
volatile bool readyToPlay = false;
volatile int playCount = 0;

typedef struct samp {
  uint8_t index;
  uint32_t duration;
  uint8_t volume;
  uint8_t stringLength;
  bool isLast;
  bool isLoop;
  int indexToLoop;
}Samp;
   
typedef struct synthSamp {
  int index;
  uint32_t duration;
  int volume;
  int fc;
  int fm;
  int decayFC;
  int riseFC;
  int depthFM;
  int decayFM;
  bool isLast;
  bool isLoop;
  int indexToLoop;
}SynthSamp;

Samp sampArray[] = {
  {0, 8000, 3, 97, 0, 0, 0},
  {1, 8000, 3, 97, 0, 0, 0},
  {2, 2000, 3, 97, 0, 0, 0},
  {3, 6000, 3, 97, 0, 0, 0},
  {4, 8000, 3, 97, 0, 1, 0},

  {5, 8000, 1, 41, 0, 0, 5},
  {6, 2000, 1, 36, 0, 0, 6},
  {7, 14000, 1, 32, 0, 0, 5},
  {8, 2000, 1, 36, 0, 0, 5},
  {9, 6000, 1, 41, 0, 0, 5},
  {10, 2000, 1, 32, 0, 0, 5},
  {11, 30000, 1, 36, 0, 0, 5},

  {12, 8000, 1, 41, 0, 0, 5},
  {13, 2000, 1, 36, 0, 0, 6},
  {14, 14000, 1, 32, 0, 0, 5},
  {15, 2000, 1, 27, 0, 0, 5},
  {16, 6000, 1, 32, 0, 0, 5},
  {17, 2000, 1, 36, 0, 0, 5},
  {18, 30000, 1, 32, 0, 1, -1},
};

SynthSamp sampArraySynth[] = {
  {0, 8000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {1, 2000, 0, (int)(8.192 * 146.83), (int)(8.192 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {2, 14000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {3, 6000, 0, (int)(8.192 * 174.61), (int)(8.192 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {4, 2000, 0, (int)(8.192 * 174.61), (int)(8.192 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {5, 6000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {6, 2000, 0, (int)(8.192 * 146.83), (int)(8.192 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {7, 24000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},

  {8, 8000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {9, 2000, 0, (int)(8.192 * 146.83), (int)(8.192 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {10, 14000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {11, 6000, 0, (int)(8.192 * 174.61), (int)(8.192 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {12, 2000, 0, (int)(8.192 * 174.61), (int)(8.192 * (174.61)), 5, 4, 8, 4, 0, 0, 0},
  {13, 6000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 0, 0},
  {14, 2000, 0, (int)(8.192 * 146.83), (int)(8.192 * (146.83)), 5, 4, 8, 4, 0, 0, 0},
  {15, 24000, 0, (int)(8.192 * 164.81), (int)(8.192 * (164.81)), 5, 4, 8, 4, 0, 1, -1},
};

typedef struct stringChannel {
  bool isOn;
  uint16_t index;
  int channelArray[SAMP_SIZE];
  Samp sample;
}StringChannel;

typedef struct SynthChannel {
  bool isOn;
  uint16_t index;
  uint16_t mainAmpFall;
  uint16_t mainRisePhase;
  uint16_t mainAmpRise;
  uint16_t FMamp;
  uint16_t FMAcc;
  uint16_t mainAcc;
  SynthSamp sample;
}SynthChannel;

StringChannel channelArray[] = {
  {1, 0, {0}, sampArray[0]},
  {1, 0, {0}, sampArray[5]},
};

SynthChannel channelArraySynth[] = {
  {0, 0, MAX_AMP, MAX_AMP, 0, MAX_AMP, 0, 0, sampArraySynth[0]}
};

void setup() {
  Serial.begin(9600);
  xorShift = analogRead(0); 
  // gives xorShift a random seed from the white on analog pin 0
  // so the sounds are slightly different every time on startup

  // Enable PCIE2 Bit3 = 1 (Port D)
  PCICR |= B00000100;
  // Select pin 6 and 7
  PCMSK2 |= B11000000;

  //set timer2 interrupt at 8kHz
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 8 prescaler
  TCCR2B |= (1 << CS21);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  for (int i = 0; i < 256; i++) {
    sineTable[i] = (char)(127.0 * sin(6.283*((float)i)/256.0)) ;
    // Serial.println(sineTable[i]);
  }

  sei();
  SPI.begin(); 
}


void loop() {
  KSAlgMaster();
}

//char serial_buf[64];
//#define DEBUG(fmt, ...) \
//  do { \
//  snprintf(serial_buf, sizeof(serial_buf), fmt, __VA_ARGS__); \
//  Serial.print(serial_buf); \
//  } while (0)
//
//#define DEBUGF(msg, f) \
//  DEBUG(msg "%d.%04d\n", (int)f, ((int)(f*10000)) % 10000);

//void fillBuffer(Samp sample) {
//  for (int i = 0; i < sizeof(sample.sampBuffer); i++) {
//    sample.sampBuffer[i] = random(-V, V);
//  }
//}

void KSAlgMaster() {
  long output = 0;
  for (int i = 0; i < 2; i++){
    if (channelArray[i].isOn) {
      output += (KSAlg(&channelArray[i]) + (1024 >> channelArray[i].sample.volume));
    }
  }
  if (channelArraySynth[0].isOn) {
    output += (FMSynth(&channelArraySynth[0]) * 2);
  }
  while (!readyToPlay) {
    // noop
  }
  SPIData(output);
  readyToPlay = false;
}

int FMSynth(SynthChannel *chan) {
  SynthSamp *sample = &chan->sample;
  // compute exponential attack and decay of amplitude
  // the (time & 0x0ff) slows down the decay computation by 256 times   
  if ((chan->index & 0x0ff) == 0) {
    chan->mainAmpFall = chan->mainAmpFall - (chan->mainAmpFall >> sample->decayFC) ;
    chan->mainRisePhase = chan->mainRisePhase - (chan->mainRisePhase >> sample->riseFC);
    // compute exponential decay of FM depth of modulation
    chan->FMamp = chan->FMamp - (chan->FMamp >> sample->decayFM) ;
  }

  // form (1-exp(-t/tau)) for the attack phase
  chan->mainAmpRise =  MAX_AMP - chan->mainRisePhase;
  // product of rise and fall exponentials is the amplitude envelope
  uint32_t FCamp = (chan->mainAmpRise >> 8) * (chan->mainAmpFall >> 8) ;

  //the first FM DDR -- feeds into final DDR
  chan->FMAcc = chan->FMAcc + sample->fm ;
  uint8_t high_fm = (char)(chan->FMAcc >> 8) ;
  int8_t fm1 = sineTable[high_fm] ;

  //the final output DDR 
  // phase accum = main_DDR_freq + FM_DDR * (FM amplitude)
  chan->mainAcc = chan->mainAcc + (sample->fc + (fm1*(chan->FMamp >> sample->depthFM)));
  uint8_t high_main = (char)(chan->mainAcc >> 8) ;
  
  // output the wavefrom sample
  // scale amplitude to use only high byte and shift into range
  // 0 to 255    
  int num = 0;
  if ((int)sineTable[high_main] > 0) {
    num = 1;
  }
  if ((int)sineTable[high_main] < 0) {
    num = -1;
  }
  signed int output = 128 + ((FCamp >> 8) * (num)) ;
  
  chan->index = chan->index + 1;
  if (chan->index == sample->duration) {
    chan->index = 0;
    chan->mainAmpFall = MAX_AMP;
    chan->mainRisePhase = MAX_AMP;
    chan->mainAmpRise = 0;
    chan->FMamp = MAX_AMP;
    chan->FMAcc = 0;
    chan->mainAcc = 0;
    if (sample->isLoop) {
      if (sample->indexToLoop == -1) {
        chan->isOn = false;
        channelArray[1].isOn = true;
        channelArray[1].sample = sampArray[5];
      }
      else{
        chan->sample = sampArraySynth[sample->indexToLoop];
      }
    }
    else {
      if (!sample->isLast) {
        chan->sample = sampArraySynth[sample->index + 1];
      }
      else {
        chan->isOn = false;
      }
    }
  }
  return output;
}

int KSAlg(StringChannel *chan) {
  int output;
  Samp *sample = &chan->sample;
  int firstIndex = chan->index % sample->stringLength;
  int secondIndex;
  if (chan->index == sample->stringLength - 1) {
    secondIndex = 0;
  }
  else {
    secondIndex = firstIndex + 1;
  }
  if (chan->index >= sample->stringLength) {
    output = ((chan->channelArray[firstIndex] + chan->channelArray[secondIndex]) >> 1);
    chan->channelArray[firstIndex] = output;
  }
  else {
    output = xorShift32() >> sample->volume;
    chan->channelArray[firstIndex] = output;
  }
  chan->index = chan->index + 1;
  if (chan->index == sample->duration) {
    chan->index = 0;
    if (sample->isLoop) {
      if (sample->indexToLoop == -1) {
        // code to jump to the other main instrument
        chan->isOn = false;
        channelArraySynth[0].isOn = true;
        channelArraySynth[0].sample = sampArraySynth[0];
      }
      else {
        // code to jump within the instrument space
        chan->sample = sampArray[sample->indexToLoop];
      }
    }
    else {
      // if it is the last sample, stop, otherwise grab the next sample
      if (!sample->isLast) {
        chan->sample = sampArray[sample->index + 1];
      }
      else {
        chan->isOn = false;
      }
    }
  }
  return output;
}

ISR(TIMER2_COMPA_vect){  
  readyToPlay = true;
  playCount++;
}

void SPIData(int data) {
  // output to pin 6 with gain and no shutdown 1X01
  long entireCommandToSend = 0x9000 + data;
  int lowerByte = entireCommandToSend & 0x00FF;
  int upperByte = (entireCommandToSend & 0xFF00) >> 8;
 
  digitalWrite(CS, LOW);
  SPI.transfer(upperByte);
  SPI.transfer(lowerByte);
  digitalWrite(CS, HIGH);
}

int xorShift32()
{
  // shift register linear number generator
  // used to replace random() for speed
  xorShift ^= xorShift << 13;
  xorShift ^= xorShift >> 17;
  xorShift ^= xorShift << 5;
  int result = xorShift & 0xFFF;
  if (result & 0x800) {
    result += 0xF001;
  }
  return result;
}