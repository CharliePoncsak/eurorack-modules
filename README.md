# drum-sampler

Arduino drum sampler.

Code and Arduino circuit from Hagiwo :

https://note.com/solder_state/n/n0209d16d0d08
https://www.youtube.com/watch?v=7Vcwk8KYZM8

## Hardware

### Board: Seeeduino xiao

https://wiki.seeedstudio.com/Seeeduino-XIAO/

#### CPU: ATSAMD21G18 (SAM D21 Family)

https://www.microchip.com/en-us/product/ATsamd21g18

[Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/DataSheets/SAM-D21-DA1-Family-Data-Sheet-DS40001882H.pdf)

## Coverting .wav files to raw audio files

Use either ffmpeg or Audiacity to convert a .wav file to use it with this program:

### Ffmpeg Method

simply use the following command:

`ffmpeg -i file.wav -f u8 -acodec pcm_u8 file.raw`

### Audacity Method

1. Load the file in audacity, then make sure the track is mono by going to Mix > Mix Stereo Down to Mono

2. Make sure the sampling rate is 32kHz by going to Tracks > Resample...

3. Export the file to raw by first selecting the portion of the clip to export then going to Export > Export Selected Audio

In the dialog that pops up, select these options
|||
|--|--|
| File type | Other uncompressed file |
| Header | RAW (header-less) |
| Encoding | Unsigned 8-bit PCM |

## Converting raw audio files to C array definition

Start the GUI file converter by typing the following command:

`start_converter_ui.cmd`

Then open a new browser tab to http://localhost:3000/.

### Converter settings

The **Input file type** box is used to select between signed ans unsigned raw audio.

The **Input glob pattern** field contains a glob pattern. All files matching this pattern relative to the root of this repository, will be converted.

In the **Output type** box, select whether the generated C array should be 1 dimensional (and use pointer arithmetic to retrieve the samples) or 2 dimensional (and use regular array indexing).
