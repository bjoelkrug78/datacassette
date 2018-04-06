# datacassette

ABOUT
-----
This project aims to store as much data as possible onto a standard audio cassette. As seen from Philips DCC it must be possible to increase the datarate per channel to somewhere around 32kbps I would expect. This would give me 64kbps in total and using HE-AAC would be enough for good quality digital audio on an analog tape. The idea would be to have a small microcontroller added to an analog tapedeck which would provide digital audio from it without any internal modifications.

CURRENT STATE
-------------
Currently I am encoding a random bitstream at 9600 bps to only one channel of the stereo tape.
Experiments have shown that there was no biterror for about 10 minutes of recording and the only corruption seen was due to a system sound that corrupted about 3000 bits of data in between. Unfortunately the algorithm didn't recover from this and the following frames were all corrupted.

HOW IS THE DATA ENCODED
-----------------------
In datacassettewriter.c a random sequence of zeros and ones is created. This random sequence is packaged into frames which include an 8 bit header, then 1024 bits of random data and followed by an 8 bit trailer. Each frame looks like this

11010010 xxxxxxxxxxxxxxxx 00101101
header   1024 random bits trailer  

The current default is 1024 random bits per frame and it creates a total of 8k (8192) frames, resulting in 8 Megabits or 512 Kilobits of data.

I am using two tones for encoding the binary ones and zeros. Ones are using a 9600 Hz tone, while zeros are using a 4800 Hz tone.
I am also using 9600 bits per second as the data rate. This means that I am actually writing out a half sine wave for a zero (one half-period) and a full sine wave for a one (two half-periods).
This makes it pretty easy to determine zeros and ones in the created .wav file when zooming into it in Audacity. Also I can easily see how my tape deck behaves when starting the recording and stopping it.

For the algorithm the detection is then as follows:
- it detects a zero-crossing and starts counting the samples until it detects another zero crossing
- in case of a zero there will theoretically be 10 samples between 2 zero crossings (because we are using a 96kHz sample rate and a 4.8kHz tone, so there are 20 samples in a full sine wave period and 10 in half of it which is the time between two zero crossings)
- in case of a one there will theoretically be 5 samples between 2 zero crossings (because we are using a 96kHz sample rate and a 9.6kHz tone for ones, so there are 10 samples in a full sine wave period and 5 in half of it which is the time between two zero crossings)
- to make detection more robust we count something that has less or equal than 7 samples between two zero crossings as a 1 and something with 8 or more samples between two zero crossings as a 0
- the algorithm keeps counting sequences of ones or zeros and only flushes them out once a zero follows a one sequence or a one follows a zero sequence

YOUTUBE
-------
I have uploaded some short video clips to youtube which show the following:

#1 Just showing the playback of my recorded data on my tape deck (left channel only) 
https://www.youtube.com/watch?v=IJ5VDKV9qbw 

#2 Showing the original .wav file that was created with datacassettewriter.c (https://github.com/bjoelkrug78/datacassette) and was used to record onto the ferro type audio tape on my tape deck
https://www.youtube.com/watch?v=9A1D3Q4va-w

#3 Showing the resulting .wav file that was recorded from my tape deck with my Windows laptop using the microphone input and audacity to sample
https://www.youtube.com/watch?v=FkQzJ6LRdTA

On both #2 and #3 you can easily decode the zeros and ones using your eyes (and so does the datacassettereader.c software)

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



