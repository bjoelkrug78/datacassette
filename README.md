# datacassette

ABOUT
-----
This project aims to store as much data as possible onto a standard audio cassette. As seen from Philips DCC it must be possible to increase the datarate per channel to somewhere around 32kbps I would expect. This would give me 64kbps in total and using HE-AAC would be enough for good quality digital audio on an analog tape. The idea would be to have a small microcontroller added to an analog tapedeck which would provide digital audio from it without any internal modifications.

CURRENT STATE
-------------
Currently I am encoding a random bitstream at 9600 bps to only one channel of the stereo tape.
Experiments have shown that there was no biterror for about 10 minutes of recording and the only corruption seen was due to a system sound that corrupted about 3000 bits of data in between.

NEXT STEPS
----------
- get the algorithm to lock onto frames so that it can get back in sync after a long burst error
- use both channels of stereo to increase data rate to 19200 bps
- use amplitude modulation to increase even further
- use training sequences in between to adjust to current conditions
- add forward error correction coding to be able to correct biterrors and increase bitrate even furth

HOW TO USE
----------
Currently there are two C source files. Compile them using gcc -o datacassettereader datacassettereader.c

datacassettewriter.c:
This will generate a random sequence of bits. A total of 8192 frames of 1024 bits data each will be generated along with an 8 bit header and 8 bit trailer. The sequence will be stored to input.txt


The datacassettewriter program will create an input.txt which will look like the following:
Frame 0                                                                        
Header: 11010010                                                               
1011110011010110000010110001111000111010111101001010100100011101010111010101001010000011010000100001100
0111010001001110101111111111011010110010111000101010101111000101000001011111001101101011111001101001110
0000001110010101010011001000111001001100001011100100000100000100000110011110100111101110010000000001101
0100011000101101111101101101000000101000000110100010000111001001110010010111101100100000100010000001011
1000100100011011111001001011000100010000011110010110111010110110111110011110011010001100100111110101100
0010010000000000100101011110001000101101101100000010011111011101111101011010010011110011111001110000111
1101111110000110001100000100011100011110111000011101101110101010001010010110011101010001011111101100101
1000011111101100110111010110110010010001100010010001100101010010001000010011010000111111010111111001000
0010010111000011001110010111110011010001110111010111010001100101101101100111001010011010010000010110000
1100101011011001011101111001000110010111110011100010100000101100101110101101011110100100000110110      
Trailer: 00101101                                                                                      
...

Alongside of it a WAV file will be created (with 96kHz sample rate, stereo, 16 bit resolution) which should be used to play from your laptop (using Audacity on Windows for example) and be recorded by your tapedeck. Make sure you have set the record level correctly on the tape deck.
Once you have the WAV file recorded, then use your tape deck to play back the recorded sounds and sample them with 96kHz sample rate, stereo, 16 bit resolution (using Audacity for example).

datacassettereader.c:
This will take in a WAV file recorded previously and will decode it and create output.txt.



