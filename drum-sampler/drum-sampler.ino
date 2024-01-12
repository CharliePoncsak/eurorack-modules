#include <FlashAsEEPROM.h>//flash memory use as eeprom
#include <Adafruit_FreeTouch.h>//touch sensor

//OLED setting
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>//for I2C
#include <avr/pgmspace.h>

#include "audio_data.h"

#define OLED_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define byte uint8_t

#define NUMBER_OF_SAMPLES 18
#define NUMBER_OF_CHANNELS 4
#define NUMBER_OF_TOUCH_SENSORS 3
#define MAX_VOLUME 10
#define MIN_VOLUME 1
#define MAX_PITCH 52
#define MIN_PITCH 5

// Audio value to output for silence, to avoid clicks
#define SILENCE 0x7f

byte pitch = 13;  // 31usec = 1sec / 32kHz(sampling rate), Adjust by processing delay.

// Original labels "808 BD ", "808 SD ", "808 TOM", "808 RIM", "808 HC ",  "808 CB ", "808 CH ", "808 OH ", "909 BD1", "909 BD2", "909 SD1", "909 SD2", "909 TOM", "909 RIM", "909 HC1", "909 HC2", "909 CH ", "909 OH "
const static char* smpl_name[NUMBER_OF_SAMPLES] PROGMEM = {//display sample name
 "909 BD ",
 "BILL HH",
 "BRUH BD",
 "CRDO CB",
 "CLN SD ",
 "909 CP ",
 "CRUNCH ",
 "DRIP PC",
 "909 HH1",
 "909 HH2",
 "LOVE PC",
 "MTRO CP",
 "909 OH ",
 "909 RS ",
 "909 SN ",
 "SHAKEIT",
 "808 SN ",
 "TECK BD"
};

// Display variables
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
byte mode = 0;//display serect , sample 0-3 and amp 4-7 , pitch 8 , display 9 ,save 10
bool should_update_display = 0;//1=display change , 0 = no change , for reduce i2c frequency.

bool disp_sw = 1; //1=display update on , 0 = no update for play sound.counter measure of delay by update display and reduce audio noise from display power supply.
bool old_disp_sw = 0;
bool font_size = 1;//1=font size big but have noise , 0 = font size small due to reduce noise.

// Touch sensors
struct TouchSensor {
  char* pin_name;
  Adafruit_FreeTouch sensor;
  int touch_value;  
  byte touch_active;
};

TouchSensor sensors[NUMBER_OF_TOUCH_SENSORS] = {
  { .pin_name = "A7", .sensor = Adafruit_FreeTouch(A7, OVERSAMPLE_32, RESISTOR_0, FREQ_MODE_NONE), .touch_value = 0, .touch_active = 0 },  // 0 = select button
  { .pin_name = "A8", .sensor = Adafruit_FreeTouch(A8, OVERSAMPLE_32, RESISTOR_0, FREQ_MODE_NONE), .touch_value = 0, .touch_active = 0 },  // 1 = up button
  { .pin_name = "A9", .sensor = Adafruit_FreeTouch(A9, OVERSAMPLE_32, RESISTOR_0, FREQ_MODE_NONE), .touch_value = 0, .touch_active = 0 }   // 2 = down button
};

struct Channel {
  bool should_play;               // A boolean indicating that the channel should be playing
  int sample_id;                  // The id of the sample selected for each channel. This id is the index of the sample in audio_data.
  size_t current_index;           // Current index into AUDIO_DATA of the byte to be output as audio.
  byte current_output;            // Current byte to be output as audio.
  byte volume;                    // Volume for each channel
  int control_voltage;            // Control voltage comming from the inputs of each channel
  int previous_control_voltage;   // Previous value of the control voltage. Used to detect a rising or falling edge.
  int input_pin;                  // Id of the pin for the control voltage input
};

Channel channels[NUMBER_OF_CHANNELS] = {
  { .should_play = 0, .sample_id = 0, .current_index = 0, .current_output = 0, .volume = 0, .control_voltage = 8, .previous_control_voltage = 0, .input_pin = 1 },
  { .should_play = 0, .sample_id = 0, .current_index = 0, .current_output = 0, .volume = 0, .control_voltage = 8, .previous_control_voltage = 0, .input_pin = 2 },
  { .should_play = 0, .sample_id = 0, .current_index = 0, .current_output = 0, .volume = 0, .control_voltage = 8, .previous_control_voltage = 0, .input_pin = 3 },
  { .should_play = 0, .sample_id = 0, .current_index = 0, .current_output = 0, .volume = 0, .control_voltage = 8, .previous_control_voltage = 0, .input_pin = 6 }
};

void setup() {
  // Display initialization
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  OLED_display();

  analogWriteResolution(8);
  analogReadResolution(10);
  // Setup inputs for each channel
  for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    Channel c = channels[i];
    pinMode(c.input_pin, INPUT_PULLDOWN);
  }
  pinMode(10, INPUT_PULLUP); //for development

  //   Initialize touch sensors
  for (int i = 0; i < NUMBER_OF_TOUCH_SENSORS; i++) {
    TouchSensor s = sensors[i];
    if (!s.sensor.begin()) {
      Serial.print("Failed to begin qt on pin ");
      Serial.println(s.pin_name);
    }
  }
}

void loop() {
  read_controls();
  soundout();
  delayMicroseconds(pitch);
}

void read_controls() {
  old_disp_sw = disp_sw;
  disp_sw = digitalRead(10);
  OLED_display(); // Is it necessary??

  if (disp_sw == 1 && 
      channels[0].should_play == 0 && 
      channels[0].should_play == 0 && 
      channels[0].should_play == 0 && 
      channels[0].should_play == 0) {
    // Read touch sensor input
    for (int i = 0; i < NUMBER_OF_TOUCH_SENSORS; i++) {
      TouchSensor s = sensors[i];
      s.touch_value = s.sensor.measure();

      // Read touch sensors values and set touch_active_x accordingly
      if (s.touch_value > 930 && s.touch_active == 0) {
        s.touch_active = 1;
        should_update_display = 1;
      }
      if (s.touch_value <= 930) {
        s.touch_value = 0;
      }
    }

    // Update mode
    if (sensors[0].touch_active == 1) {
      sensors[0].touch_active = 0;
      mode++;
      if (mode > 10) {
        mode = 0;
      }
      else if (mode < 0) {
        mode = 10;
      }
    }

    switch (mode) {
      case 0:
      case 1:
      case 2:
      case 3:
        {
          // Select sample
          Channel c = channels[mode];
          if (sensors[1].touch_active == 1) { // Up
            sensors[1].touch_active = 0;
            c.should_play = 1; // One shot sample play
            c.sample_id++;
            if (c.sample_id > NUMBER_OF_SAMPLES - 1) {
              c.sample_id = 0;
            }
          } else if (sensors[2].touch_active == 1) { // Down
            sensors[2].touch_active = 0;
            c.should_play = 1; // One shot sample play
            c.sample_id--;
            if (c.sample_id < 0) {
              c.sample_id = NUMBER_OF_SAMPLES - 1;
            }
          }
          break;
        }
      
      case 4:
      case 5:
      case 6:
      case 7:
        {
          // Set volume
          Channel c = channels[mode - 4];
          if (sensors[1].touch_active == 1) { // Up
            sensors[1].touch_active = 0;
            c.should_play = 1; // One shot sample play
            c.volume++;
            if (c.volume > MAX_VOLUME) {
              c.volume = MAX_VOLUME;
            }
          } else if (sensors[2].touch_active == 1) { // Down
            sensors[2].touch_active = 0;
            c.should_play = 1; // One shot sample play
            c.volume--;
            if (c.volume < MIN_VOLUME) {
              c.volume = MIN_VOLUME;
            }
          }
          break;
        }
      case 8:
        {
          if (sensors[1].touch_active == 1) { // Up
            sensors[1].touch_active = 0;
            channels[0].should_play = 1; // One shot sample play on channel 1
            pitch++;
            if (pitch > MAX_PITCH) {
              pitch = MAX_PITCH;
            }
          } else if (sensors[2].touch_active == 1) { // Down
            sensors[2].touch_active = 0;
            channels[0].should_play = 1; // One shot sample play on channel 1
            pitch--;
            if (pitch < MIN_PITCH) {
              pitch = MIN_PITCH;
            }
          }
          break;
        }
      case 9:
        {
          if (sensors[1].touch_active == 1) { // Up
            sensors[1].touch_active = 0;
            font_size = 1;
          } else if (sensors[2].touch_active == 1) { // Down
            sensors[2].touch_active = 0;
            font_size = 0;
          }
          break;
        }
      case 10:
        {
          if (sensors[1].touch_active == 1) { // Up
            sensors[1].touch_active = 0;
            save_settings();
          } else if (sensors[2].touch_active == 1) { // Down
            sensors[2].touch_active = 0;
            save_settings();
          }
          break;
        }
      default:
        break;
    }

    if (should_update_display == 1) {
      should_update_display = 0;
      OLED_display();
      delay(100);//countermeasure of touch sensor continuous input
      // TODO remove delay once touch sensors changed to knobs
    }
  }

  // Read channel inputs
  for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    Channel c = channels[i];
    c.previous_control_voltage = c.control_voltage;
    c.control_voltage = digitalRead(c.input_pin);
  }
}

void soundout() {
  int master_output = 0;
  for (int i = 0; i < NUMBER_OF_CHANNELS; i++) {
    Channel c = channels[i];

    // Check if sample is done playing
    if (c.current_index >= AUDIO_DATA_OFFSETS[c.sample_id] + AUDIO_DATA_SIZES[c.sample_id]) {
      c.should_play = 0;
    }

    // Check for rising edge on control voltage
    if (c.previous_control_voltage == 0 && c.control_voltage == 1) {
      c.should_play = 1;
      // Set index to the start of the selected sample
      c.current_index = AUDIO_DATA_OFFSETS[c.sample_id];
    }

    if (c.should_play == 1) {
      // Read byte in AUDIO_DATA
      c.current_output = ((pgm_read_byte(&(AUDIO_DATA[c.current_index])) - 127) * c.volume / 6) + 127;
      // Increment index
      c.current_index++;
    } else {
      c.current_output = SILENCE;
    }

    master_output += c.current_output;
  }
  analogWrite(A0, master_output);
}

// Save setting data to flash memory
void save_settings() {
 delay(100);
 EEPROM.write(1, channels[0].sample_id);
 EEPROM.write(2, channels[1].sample_id);
 EEPROM.write(3, channels[2].sample_id);
 EEPROM.write(4, channels[3].sample_id);
 EEPROM.write(5, channels[0].volume);
 EEPROM.write(6, channels[1].volume);
 EEPROM.write(7, channels[2].volume);
 EEPROM.write(8, channels[3].volume);
 EEPROM.write(9, pitch);
 EEPROM.commit();

 display.clearDisplay();
 display.setTextSize(font_size + 1);  
 if (disp_sw == 1) {
   display.setTextSize(2);  
 }
 display.setTextColor(BLACK, WHITE);
 display.setCursor(0, 32);  
 display.print("SAVED ");
 display.display();
 delay(1000);
}

void load_settings() {
  if(EEPROM.isValid() == 1) {
    channels[0].sample_id = EEPROM.read(1);
    channels[1].sample_id = EEPROM.read(2);
    channels[2].sample_id = EEPROM.read(3);
    channels[3].sample_id = EEPROM.read(4);
    channels[0].volume = EEPROM.read(5);
    channels[1].volume = EEPROM.read(6);
    channels[2].volume = EEPROM.read(7);
    channels[3].volume = EEPROM.read(8);
    pitch = EEPROM.read(9);
  } else {
    // Assign default values
    channels[0].sample_id = 10;
    channels[1].sample_id = 5;
    channels[2].sample_id = 1;
    channels[3].sample_id = 2;
    channels[0].volume = 8;
    channels[1].volume = 8;
    channels[2].volume = 8;
    channels[3].volume = 8;
    pitch = 13;
  }
}

void OLED_display() {
  display.clearDisplay();  
  display.setTextSize(font_size + 1);  
  if (disp_sw == 1) {
    display.setTextSize(2);  
  }
  display.setTextColor(WHITE); 

  if (mode <= 7) {
    //sample display
    display.setCursor(0, 0);  
    display.print(smpl_name[channels[0].sample_id]);
    display.setCursor(0, 16);  
    display.print(smpl_name[channels[1].sample_id]);
    display.setCursor(0, 32);  
    display.print(smpl_name[channels[2].sample_id]);
    display.setCursor(0, 48);  
    display.print(smpl_name[channels[3].sample_id]);

    //amp display
    display.drawRect(95, 0,  channels[0].volume * 3, 15, WHITE);
    display.drawRect(95, 16, channels[1].volume * 3, 15, WHITE);
    display.drawRect(95, 32, channels[2].volume * 3, 15, WHITE);
    display.drawRect(95, 48, channels[3].volume * 3, 15, WHITE);

    //mode display
    if (mode < 4 && disp_sw == 1) {
      display.drawTriangle(92, 2 + mode * 16, 92, 10 + mode * 16, 86, 6 + mode * 16, WHITE);
    } else if (mode >= 4 && disp_sw == 1) {
      display.drawTriangle(86, 2 + (mode - 4) * 16, 86, 10 + (mode - 4) * 16, 92, 6 + (mode - 4) * 16, WHITE);
    }
  } else if (mode >= 8) {
    display.setCursor(0, 0);  
    display.print("PITCH");
    display.setCursor(70, 0);  
    display.print(pitch);
    display.setCursor(0, 16);  
    display.print("NOISE");
    display.setCursor(70, 16);  
    if (font_size == 1) {
      display.print("MID");
    } else if (font_size == 0) {
      display.print("LOW");
    }
    display.setCursor(0, 32);  
    display.print("SAVE");
    display.drawTriangle(114, 2 + (mode - 8) * 16, 114, 10 + (mode - 8) * 16, 108, 6 + (mode - 8) * 16, WHITE);
  }
  display.display();
}