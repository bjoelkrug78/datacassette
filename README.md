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
Alongside of it a WAV file will be created (with 96kHz sample rate, stereo, 16 bit resolution) which should be used to play from your laptop (using Audacity on Windows for example) and be recorded by your tapedeck. Make sure you have set the record level correctly on the tape deck.

Once you have the WAV file recorded, then use your tape deck to play back the recorded sounds and sample them with 96kHz sample rate, stereo, 16 bit resolution (using Audacity for example).

datacassettereader.c:
This will take in a WAV file recorded previously and will decode it and create output.txt.

