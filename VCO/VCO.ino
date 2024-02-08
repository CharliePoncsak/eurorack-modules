// Custom ESP32 C3 implementation of a VCO
#include "audio_data.h"

// Pin numbers
#define V_OCT_PIN 0
#define FREQ_PIN 1
#define MOD_CV_PIN 2
#define MOD_PIN 3
#define WAVEFORM_PIN 4
// Pin 5 is empty
#define OCT_1_PIN 6
#define OCT_2_PIN 7
#define AUDIO_OUT_PIN 8
#define MODE_1_PIN 9
#define MODE_2_PIN 10

int mode = 0; // Base wavefold mode
int waveform = 1; // Base sine waveform
int f0 = 35; // base osc frequency

int wavetable[256]; //  1024 resolution , 256 rate
float freq_table[2048];

void setup() {
  Serial.begin(9600); // for debugging

  table_set(); // set wavetable
  
  // Select octave
  for (int i = 0; i < 1230; i++) {
    freq_table[i] = f0 * pow(2, (V_OCT_POW[i]));
  }
  for (int i = 0; i < 2048 - 1230; i++) {
    freq_table[i + 1230] = 6;
  }
}

void loop() {
    // TODO
}



void table_set() {//make wavetable

  if (mode == 0) { //wavefold
    switch  (waveform) {
      case 0:
        for (int i = 0; i < 256; i++) {  //saw
          wavetable[i] = i * 4 - 512;
        }
        break;

      case 1:
        for (int i = 0; i < 256; i++) {  //sin
          wavetable[i] = (sin(2 * M_PI * i  / 256)) * 511;
        }
        break;

      case 2:
        for (int i = 0; i < 128; i++) {  //squ
          wavetable[i] = 511;
          wavetable[i + 128] = -511;
        }
        break;

      case 3:
        for (int i = 0; i < 128; i++) {  //tri
          wavetable[i] = i * 8 - 511;
          wavetable[i + 128] = 511 - i * 8;
        }
        break;

      case 4:
        for (int i = 0; i < 128; i++) {  //oct saw
          wavetable[i] = i * 4 - 512 + i * 2;
          wavetable[i + 128] = i * 2 - 256 + i * 4;
        }
        break;

      case 5:
        for (int i = 0; i < 256; i++) {  //FM1
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 3 * i  / 256)) ) * 511;
        }
        break;

      case 6:
        for (int i = 0; i < 256; i++) {  //FM2
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 7 * i  / 256))) * 511;
        }
        break;

      case 7:
        for (int i = 0; i < 256; i++) {  //FM3
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 4 * i  / 256 + sin(2 * M_PI * 11 * i  / 256)))) * 511;
        }
        break;
    }
  }
  else if (mode == 2) { //AM
    switch  (waveform) {
      case 0:
        for (int i = 0; i < 256; i++) {  //saw
          wavetable[i] = (sin(2 * M_PI * i  / 256)) * 511;
        }
        break;

      case 1:
        for (int i = 0; i < 256; i++) {  //fm1
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 3 * i  / 256)) ) * 511;
        }
        break;

      case 2:
        for (int i = 0; i < 256; i++) {  //fm2
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 5 * i  / 256)) ) * 511;
        }
        break;

      case 3:
        for (int i = 0; i < 256; i++) {  //fm3
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 4 * i  / 256 + sin(2 * M_PI * 11 * i  / 256)))) * 511;
        }
        break;

      case 4:
        for (int i = 0; i < 256; i++) {  //non-integer multiplets fm1
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 1.28 * i  / 256)) ) * 511;
        }
        break;

      case 5:
        for (int i = 0; i < 256; i++) {  //non-integer multiplets fm2
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 3.19 * i  / 256)) ) * 511;
        }
        break;

      case 6:
        for (int i = 0; i < 256; i++) {  //non-integer multiplets fm3
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 2.3 * i  / 256 + sin(2 * M_PI * 7.3 * i  / 256)))) * 511;
        }
        break;

      case 7:
        for (int i = 0; i < 256; i++) {  //non-integer multiplets fm3
          wavetable[i] = (sin(2 * M_PI * i  / 256 + sin(2 * M_PI * 6.3 * i  / 256 + sin(2 * M_PI * 11.3 * i  / 256)))) * 511;
        }
        break;
    }
  }
}