#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

#define FRAMELENGTH 1024
#define FRAMES 8*1024
#define DATALENGTH FRAMELENGTH*FRAMES
#define INTROLENGTH 8
#define OUTROLENGTH 8
#define BITLENGTH FRAMES*(INTROLENGTH+OUTROLENGTH+FRAMELENGTH)
#define SAMPLERATE 96000
#define ONEFREQ 9600
#define ZEROFREQ 4800
#define DATARATE 9600

#define PI 3.14159265
#define PAUSETIME 10

#define SUBCHUNK1SIZE   (16)
#define AUDIO_FORMAT    (1) /* For PCM */
#define NUM_CHANNELS    (2)

#define BITS_PER_SAMPLE (16)

#define BYTE_RATE       (SAMPLERATE * NUM_CHANNELS * BITS_PER_SAMPLE/8)
#define BLOCK_ALIGN     (NUM_CHANNELS * BITS_PER_SAMPLE/8)

/*
The header of a wav file Based on:
https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
*/
typedef struct wavfile_header_s
{
    char    ChunkID[4];     /*  4   */
    int32_t ChunkSize;      /*  4   */
    char    Format[4];      /*  4   */

    char    Subchunk1ID[4]; /*  4   */
    int32_t Subchunk1Size;  /*  4   */
    int16_t AudioFormat;    /*  2   */
    int16_t NumChannels;    /*  2   */
    int32_t SampleRate;     /*  4   */
    int32_t ByteRate;       /*  4   */
    int16_t BlockAlign;     /*  2   */
    int16_t BitsPerSample;  /*  2   */

    char    Subchunk2ID[4];
    int32_t Subchunk2Size;
} wavfile_header_t;



void write_PCM16_stereo_header(FILE*   file_p,
                               int32_t SampleRate,
                               int32_t FrameCount)
{
  wavfile_header_t wav_header;
  int32_t subchunk2_size;
  int32_t chunk_size;
  size_t write_count;

  subchunk2_size  = FrameCount * NUM_CHANNELS * BITS_PER_SAMPLE/8;
  chunk_size      = 4 + (8 + SUBCHUNK1SIZE) + (8 + subchunk2_size);

  wav_header.ChunkID[0] = 'R';
  wav_header.ChunkID[1] = 'I';
  wav_header.ChunkID[2] = 'F';
  wav_header.ChunkID[3] = 'F';

  wav_header.ChunkSize = chunk_size;

  wav_header.Format[0] = 'W';
  wav_header.Format[1] = 'A';
  wav_header.Format[2] = 'V';
  wav_header.Format[3] = 'E';

  wav_header.Subchunk1ID[0] = 'f';
  wav_header.Subchunk1ID[1] = 'm';
  wav_header.Subchunk1ID[2] = 't';
  wav_header.Subchunk1ID[3] = ' ';

  wav_header.Subchunk1Size = SUBCHUNK1SIZE;
  wav_header.AudioFormat = AUDIO_FORMAT;
  wav_header.NumChannels = NUM_CHANNELS;
  wav_header.SampleRate = SampleRate;
  wav_header.ByteRate = BYTE_RATE;
  wav_header.BlockAlign = BLOCK_ALIGN;
  wav_header.BitsPerSample = BITS_PER_SAMPLE;

  wav_header.Subchunk2ID[0] = 'd';
  wav_header.Subchunk2ID[1] = 'a';
  wav_header.Subchunk2ID[2] = 't';
  wav_header.Subchunk2ID[3] = 'a';
  wav_header.Subchunk2Size = subchunk2_size;

  fwrite(&wav_header, 
         sizeof(wavfile_header_t), 1,
         file_p
        );
}



/*Data structure to hold a single frame with two channels*/
typedef struct PCM16_stereo_s
{
  int16_t left;
  int16_t right;
} PCM16_stereo_t;



int main(void)
{
  FILE* file_p;
  FILE* inputdata_p;
  int frame[INTROLENGTH+FRAMELENGTH+OUTROLENGTH];

  int f, i, j, k, leftbit, rightbit, polarity_left=1, polarity_right=1;
  int32_t FrameCount;

  PCM16_stereo_t *sample_p=NULL;
  sample_p = (PCM16_stereo_t *)malloc(sizeof(PCM16_stereo_t) * 1);
 
  /* Open the wav file*/
  file_p = fopen("input.wav", "w");

  FrameCount = 2*PAUSETIME*SAMPLERATE+(BITLENGTH/DATARATE)*SAMPLERATE;
  printf("Framecount is %d\n", FrameCount);

  /* write the wav file header*/
  write_PCM16_stereo_header(file_p,
                            SAMPLERATE,
                            FrameCount);

  /* write pause */
  for(i=0; i<SAMPLERATE*PAUSETIME; i++)
  {
    sample_p[0].left  = 0;
    sample_p[0].right = 0;
    fwrite(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
  };

  printf("Samplerate is %d Hz\n", SAMPLERATE);
  printf("Datarate is %d bps\n", DATARATE);
  printf("Creating %d bits of data\n", DATALENGTH);
  inputdata_p = fopen("input.txt", "w");

  /* Generate frame data */
  for(f=0; f<FRAMES; f++) 
  {
    printf("Writing frame %d (consisting of %d intro bits, %d data bits, %d outro bits)\n", f, INTROLENGTH, FRAMELENGTH, OUTROLENGTH);

    fprintf(inputdata_p, "Frame %d\n", f);

    /* write intro */
    frame[0]=1;
    frame[1]=1;
    frame[2]=0;
    frame[3]=1;
    frame[4]=0;
    frame[5]=0;
    frame[6]=1;
    frame[7]=0;
    fprintf(inputdata_p, "Header: 11010010\n");

    /* randomly generated data */
    for(i=0; i<FRAMELENGTH; i++)
    {
      frame[INTROLENGTH+i]=rand() & 1;
      fprintf(inputdata_p, "%d", frame[INTROLENGTH+i]);
    };

    /* write outro */
    frame[FRAMELENGTH+INTROLENGTH]=0;
    frame[FRAMELENGTH+INTROLENGTH+1]=0;
    frame[FRAMELENGTH+INTROLENGTH+2]=1;
    frame[FRAMELENGTH+INTROLENGTH+3]=0;
    frame[FRAMELENGTH+INTROLENGTH+4]=1;
    frame[FRAMELENGTH+INTROLENGTH+5]=1;
    frame[FRAMELENGTH+INTROLENGTH+6]=0;
    frame[FRAMELENGTH+INTROLENGTH+7]=1;
    fprintf(inputdata_p, "\nTrailer: 00101101\n");

    /* write data */
    for(i=0; i<FRAMELENGTH+INTROLENGTH+OUTROLENGTH; i++)
    {
      leftbit = frame[i];
      for(j=0; j<SAMPLERATE/DATARATE; j++)
      { 
        if(leftbit == 1)
        {
          sample_p[0].left = (int16_t) polarity_left*32767.0 * sin(2*PI*j*ONEFREQ/SAMPLERATE);
        } else {
          sample_p[0].left = (int16_t) polarity_left*32767.0 * sin(2*PI*j*ZEROFREQ/SAMPLERATE);
        };
        /* right channel is silent for now */
        sample_p[0].right = 0;
        fwrite(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
      };
      if(leftbit == 0) polarity_left=polarity_left*(-1);
    };
   
  };

  /* write pause */
  for(i=0; i<SAMPLERATE*PAUSETIME; i++)
  {
    sample_p[0].left  = 0;
    sample_p[0].right = 0;
    fwrite(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
  };

  fclose(file_p);
  fclose(inputdata_p);
  free(sample_p);
  return 0;    
}

