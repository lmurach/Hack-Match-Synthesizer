# Hack-Match-Synthesizer

## Introduction
  The aim of this project was to build a synthesizer and menu display to emulate the title screen of the mini-game Hack\*Match (within the game Exapunks), using the Arduino Uno. The main focus was creating a system to play sound from the Arduino formulaically and allow for multiple types of sounds and instruments to overlap. As a fun bonus, a display was programmed to draw different shapes in color which was used to draw the menu text.

## Results
The <a href="https://youtu.be/K3FyVlFzFVE" target="_blank">following</a> is the audio recording from my synthesizer. And <a href="https://www.youtube.com/watch?v=lHcRBpQOLEU" target="_blank">this video</a> is the audio from the official game.

<picture>
<img src=https://github.com/Quillington/Hack-Match-Synthesizer/assets/66843400/c6c5e999-061f-44af-b399-e9b00ddbd5bd>
</picture>


## Sound Implementation

### Creating Audio From Bits
  Sound waves are caused by the gradual differences in voltages over time. The Arduino is a digital device that can output a voltage that is either high (5V) or low (0V). Playing sound that only has two increments is 2 bit sound, which sounds very limited. Therefore, to achieve a higher quality sound, a system is needed that can send out a voltage at different values within a range. The more different values (or steps) within a range, the higher quality audio. For this project, the MCP4822 digital to analog converter (DAC) was used. It allows 12 bits to be sent to it for each voltage, giving it 4096 steps. This is relatively good, as CD audio is 16 bit.

  16 bits of information are sent over SPI. The first 4 bits are the header information specifying the output pin and gain, with the other 12 bits being a binary representation of a number 0-4095.

  Now that the gradual voltage differences were handled, the timing of when these samples were played needed to be handled. According to the Nyquist Theorem, the sampling rate, or number of samples generated and played each sample, must be more than double the highest frequency. The human range of hearing is between 20Hz and 20kHz, although a piano’s highest note is a little less than 8kHz. Most notes are much lower than this, middle C is 260Hz. This is why the sampling rate of 8000 was chosen, with a plan to change it to 16000 if the chip proved fast enough.
In order to ensure that a sample is always played exactly on time, a timer interrupt is used to play a prepared voltage value every 125us (1/8000 s).

### Reason for Synthesis
  The Arduino only has 32kBs of flash memory. If a sound sample was played 8000 times a second (a relatively modest sampling rate, CD audio plays a sample over 16000 times a second), then every second would require 12kB of hard coded data. In under 3 seconds of playing a song, the arduino would run out of memory.

## Karplus-Strong
  Kevin Karplus and Alex Strong published a paper titled ['Digital Synthesis of Plucked-String and Drum Timbres'](http://www.music.mcgill.ca/~gary/courses/papers/Karplus-Strong-CMJ-1983.pdf) in 1983 detailing an algorithm that can be used to synthesize a noise that closely resembles a guitar pluck. The main idea of the algorithm is to generate a circular array of random noise (the range of random numbers is the volume), and slowly average the array of data over time. Since the algorithm only uses random number generation, addition, and dividing by two (a bit shift), the algorithm is extremely fast in hardware. (The algorithm [XORshift32](https://www.jstatsoft.org/article/view/v008i14) was implemented for faster random number generation)
  
  The two values that are averaged together depend on the frequency of the note. The first value is always the previous array value. The second value is the value *p*, which is the sampling rate/not harmonic frequency. For example, a middle C with my synthesizer would be 8000/261.63 = 30.57. This number is rounded up to 31 so that it can be used as an array index. This rounding causes some error in the note accuracy. Increasing the size of the array by 2 and having the array represent 0.5 for each index would allow for greater accuracy in the note values. This was not done due to strict memory constraints. 

## FM-Synthesis
FM-Synthesis is the process of playing a sine wave (or other basic wave) where the frequency is modulated over time by another sine wave. Multiple modulations can be stacked to create increasingly more interesting sound. The main downfall is that each modulation requires more computation time. This method was invented by John Chowning in his paper ['The Synthesis of Complex Audio Spectra by Means of Frequency Modulation'](https://web.eecs.umich.edu/~fessler/course/100/misc/chowning-73-tso.pdf) in 1973.

Calculating sine waves, exponential functions, and the floating point arithmetic required for these calculations is expensive and difficult to do. For this reason, sine tables are used as well as integer multiplication approximations. Many of these ideas come from Professor Bruce Land’s 2012 ECE4760 [videos](https://www.youtube.com/playlist?list=PLD7F7ED1F3505D8D5) and [example code](https://people.ece.cornell.edu/land/courses/ece4760/Math/GCC644/FM_synth/FM_synth_3.c). 

## System for Playing notes
In this project there are three channels, two guitars and one synth. Channels are made from a struct that contains information about the sample to play, whether the channel is on or off, and any data that is reused for each new sample. For Karplus-Strong synthesis, the array is reused for each sample by overwriting the same channel array with new random noise data. Allocating a new array for every sample would be extremely memory intensive. FM-Synthesis reuses the variables that affect the amplitude of the signal during the attack and delay of the signal. These values are the same for every sample, but are changed over time, so they cannot be constants.

Samples are stored in a separate array of different structs that contain information about the note frequency, decay and modulation parameters (FM only), duration, volume, and whether the note loops to a certain location. 

Volume is limited to full volume, half volume, or quarter volume. This is so that the values can be bit shifted instead of divided with floating point values to greatly increase the speed. Unfortunately, this limits the sound capabilities and can make the sound feel monotonous. This tradeoff was made since the increase in speed was almost 20us per sample.


## Display Introduction
The NHD-1.69-160128ASC3 display (with the SEPS525 controller) was used to show the title screen.

### Starting Point and Modifications
The following [display example code](https://github.com/NewhavenDisplay/NHD-1.69-160128ASC3_Example/tree/master), published by the official display distributor was used as a starting point. This code was invaluable for the list of commands needed to turn on the display as well as a reference to how commands and data should be sent. However, a few modifications were made for speed and more utility.

### Bit-Banging to SPI
The example code set the settings so that each pixel required 18 bits of color data to be sent from the microcontroller to the display. The code sent these 18 bits by manually bit-banging an output pin high and low. This proved to be extremely slow. (About 14 seconds to turn the entire screen one color). The original plan was to send all 18 bits using SPI, however the Arduino only allows 8 bits to be sent over SPI. Sending 24 bits did not work, so instead 16 bits were sent with the last 2 sent without the SPI protocol. Below shows an oscilloscope reading of the bits, showing the speed difference between the first 16 and the last 2.
<picture>
<img src=https://github.com/Quillington/Hack-Match-Synthesizer/assets/66843400/db053716-8447-430f-813d-1e3b821a5b1e>
</picture>

Later this approach was redone so that the display was changed to the 16 bit color mode and no bit-banging was necessary. This further increased the speed to 0.5us per screen update. Unfortunately this speed is still quite slow for gameplay. For an interactive game to utilize this display, a different microcontroller must be used.


### Drawing symbols
By utilizing the command to change memory position, it is very easy to formulaically draw different shapes. Functions were added to one to draw rectangles with an option to slant the rectangles to create parallelograms. It starts at the starting index, draws for width pixels, then sets the y value to the new memory location. Any shape is able to be broken up into different squares and parallelograms to be drawn on the display.


## Further Implementation Ideas
The memory and SPI speed on the arduino proved to be extremely limiting. While the memory was enough for the menu, it is nowhere near enough for the full game. A separate external RAM chip could also be utilized, but the number of instruments and display speed would still be heavily limited by the SPI speed. It is likely that a different microcontroller would be utilized so that better performance could be achieved.


