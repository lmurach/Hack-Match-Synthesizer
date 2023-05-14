# Hack-Match-Synthesizer

## Introduction
  The aim of this project was to build a synthesizer and menu display to emulate the title screen of the game Hack\*Match, using the Arduino Uno (ATMEGA328P-PU). The main focus was creating a system to play sound from the Arduino formulaically and allow for multiple types of sounds and instruments to overlap. As a fun bonus, a display was programmed to draw different shapes using any color in order to draw the menu text.

## Results
Below is the audio recording from my synthesizer

Below is the official audio from the real game Hack\*Match

https://github.com/Quillington/Hack-Match-Synthesizer/assets/66843400/3aa96e2a-16e6-4faf-affb-aeb4ebef89ed

Next is a side by side comparison of my emulated display (left) and the real menu (right). Some liberties were taken for time and aesthetic reasons.

<picture>
<img src="https://cdn.discordapp.com/attachments/269482905707872256/1107364315822051409/image.png">
</picture>

## Sound Implementation

### Creating Audio From Bits
  Sound waves are caused by the gradual differences in voltages over time. The Arduino is a digital device that can output a voltage that is either high (5V) or low (0V). Playing sound that only has two increments is 2 bit sound, which sounds very limited. Therefore, to achieve a higher quality sound, a system is needed that can send out a voltage at different values within a range. The more different values (or steps) within a range, the higher quality audio. For this project, the MCP4822 digital to analog converter (DAC) was used. It allows 12 bits to be sent to it for each voltage, giving it 4096 steps. This is relatively good, as CD audio is 16 bit.
  
// diagram

  These exact pins on the arduino must be used for SPI (including the chip select pin), as this is what the Arduino SPI library requires. 16 bits of information are sent over SPI. The first 4 bits are the header information specifying the output pin and gain, with the other 12 bits being a binary representation of a number 0-4095.

  Now that the gradual voltage differences were handled, the timing of when these samples were played needed to be handled. According to the Nyquist Theorem, the sampling rate, or number of samples generated and played each sample, must be more than double the highest frequency. The human range of hearing is between 20Hz and 20kHz, although a piano’s highest note is a little less than 8kHz. Most notes are much lower than this, middle C is 260Hz. This is why the sampling rate of 8000 was chosen, with a plan to change it to 16000 if the chip proved fast enough.
In order to ensure that a sample is always played exactly on time, a timer interrupt is used to play a prepared voltage value every 125us (1/8000 s).

### Reason for Synthesis
  The Arduino only has 32kBs of flash memory. If a sound sample was played 8000 times a second (a relatively modest sampling rate, CD audio plays a sample over 16000 times a second), then every second would require 12kB of hard coded data. In under 3 seconds of playing a song, the arduino would run out of memory.

### Karplus-Strong
// mention the paper cited here

### FM-Synthesis
// mention Bruce Land’s video and lab code here.
### System for Playing notes
  In this project there are three channels, two guitars and one synth. Channels are made from a struct that contains information about the sample to play, whether the channel is on or off, and any data that is reused for each new sample. For Karplus-Strong synthesis, the array is reused for each sample by overwriting the same channel array with new random noise data. Allocating a new array for every sample would be extremely memory intensive. FM-Synthesis reuses the variables that affect the amplitude of the signal during the attack and delay of the signal. These values are the same for every sample, but are changed over time, so they cannot be constants.
  
  Samples are stored in a separate array of different structs that contain information about the note frequency, decay and modulation parameters (FM only), duration, volume, and whether the note loops to a certain location. 
  
  Volume is limited to full volume, half volume, or quarter volume. This is so that the values can be bit shifted instead of divided with floating point values to greatly increase the speed. Unfortunately, this limits the sound capabilities and can make the sound feel monotonous. This tradeoff was made since the increase in speed was almost 20us per sample.

## Display Implementation

