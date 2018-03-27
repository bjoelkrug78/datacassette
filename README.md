# datacassette

This project aims to store as much data as possible onto a standard audio cassette.
Currently there are two C source files. Compile them using gcc -o datacassettereader datacassettereader.c

datacassettewriter.c:
This will generate a random sequence of bits. A total of 8192 frames of 1024 bits data each will be generated along with an 8 bit header and 8 bit trailer. The sequence will be stored to input.txt
Alongside of it a WAV file will be created (with 96kHz sample rate, stereo, 16 bit resolution) which should be used to play from your laptop (using Audacity on Windows for example) and be recorded by your tapedeck. Make sure you have set the record level correctly on the tape deck.

Once you have the WAV file recorded, then use your tape deck to play back the recorded sounds and sample them with 96kHz sample rate, stereo, 16 bit resolution (using Audacity for example).

datacassettereader.c:
This will take in a WAV file recorded previously and will decode it and create output.txt.

