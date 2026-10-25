#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <math.h>

/* ==========================================================================
 *  datacassettewriter.c
 *
 *  Generates a WAV file containing a random bitstream encoded as audio tones
 *  suitable for recording onto a standard audio cassette.
 *
 *  Encoding scheme:
 *    - 1-bit  → 9600 Hz sine wave (full period)
 *    - 0-bit  → 4800 Hz sine wave (half period)
 *    - Data rate: 9600 bits per second
 *    - Sample rate: 96 kHz, 16-bit stereo
 *
 *  Frame format:
 *    8-bit header (11010010) + 1024 data bits + 8-bit trailer (00101101)
 * ========================================================================= */

#define FRAMELENGTH   1024          /* Number of data bits per frame */
#define FRAMES        (8*1024)      /* Total number of frames to generate */
#define DATALENGTH    (FRAMELENGTH*FRAMES)
#define INTROLENGTH   8             /* Header length in bits */
#define OUTROLENGTH   8             /* Trailer length in bits */
#define BITLENGTH     (FRAMES*(INTROLENGTH+OUTROLENGTH+FRAMELENGTH))
#define SAMPLERATE    96000         /* Audio sample rate in Hz */
#define ONEFREQ       9600          /* Frequency used to encode a '1' */
#define ZEROFREQ      4800          /* Frequency used to encode a '0' */
#define DATARATE      9600          /* Target data rate in bits per second */

#define PI            3.14159265
#define PAUSETIME     10            /* Seconds of silence at start and end */

/* WAV file constants */
#define SUBCHUNK1SIZE (16)
#define AUDIO_FORMAT  (1)           /* PCM */
#define NUM_CHANNELS  (2)
#define BITS_PER_SAMPLE (16)

#define BYTE_RATE     (SAMPLERATE * NUM_CHANNELS * BITS_PER_SAMPLE/8)
#define BLOCK_ALIGN   (NUM_CHANNELS * BITS_PER_SAMPLE/8)

/*
 * Standard WAV file header structure
 * Based on: https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */
typedef struct wavfile_header_s
{
    char    ChunkID[4];     /* "RIFF" */
    int32_t ChunkSize;
    char    Format[4];      /* "WAVE" */

    char    Subchunk1ID[4]; /* "fmt " */
    int32_t Subchunk1Size;
    int16_t AudioFormat;
    int16_t NumChannels;
    int32_t SampleRate;
    int32_t ByteRate;
    int16_t BlockAlign;
    int16_t BitsPerSample;

    char    Subchunk2ID[4]; /* "data" */
    int32_t Subchunk2Size;
} wavfile_header_t;


/* Write a complete PCM 16-bit stereo WAV header to the file */
void write_PCM16_stereo_header(FILE*   file_p,
                               int32_t SampleRate,
                               int32_t FrameCount)
{
  wavfile_header_t wav_header;
  int32_t subchunk2_size;
  int32_t chunk_size;

  subchunk2_size  = FrameCount * NUM_CHANNELS * BITS_PER_SAMPLE/8;
  chunk_size      = 4 + (8 + SUBCHUNK1SIZE) + (8 + subchunk2_size);

  /* RIFF chunk descriptor */
  wav_header.ChunkID[0] = 'R';
  wav_header.ChunkID[1] = 'I';
  wav_header.ChunkID[2] = 'F';
  wav_header.ChunkID[3] = 'F';
  wav_header.ChunkSize  = chunk_size;

  wav_header.Format[0] = 'W';
  wav_header.Format[1] = 'A';
  wav_header.Format[2] = 'V';
  wav_header.Format[3] = 'E';

  /* fmt sub-chunk */
  wav_header.Subchunk1ID[0] = 'f';
  wav_header.Subchunk1ID[1] = 'm';
  wav_header.Subchunk1ID[2] = 't';
  wav_header.Subchunk1ID[3] = ' ';
  wav_header.Subchunk1Size   = SUBCHUNK1SIZE;
  wav_header.AudioFormat    = AUDIO_FORMAT;
  wav_header.NumChannels    = NUM_CHANNELS;
  wav_header.SampleRate     = SampleRate;
  wav_header.ByteRate       = BYTE_RATE;
  wav_header.BlockAlign     = BLOCK_ALIGN;
  wav_header.BitsPerSample  = BITS_PER_SAMPLE;

  /* data sub-chunk */
  wav_header.Subchunk2ID[0] = 'd';
  wav_header.Subchunk2ID[1] = 'a';
  wav_header.Subchunk2ID[2] = 't';
  wav_header.Subchunk2ID[3] = 'a';
  wav_header.Subchunk2Size   = subchunk2_size;

  fwrite(&wav_header, sizeof(wavfile_header_t), 1, file_p);
  printf("WAV header written (SampleRate=%d Hz, FrameCount=%d)\n", SampleRate, FrameCount);
}


/* Single stereo sample (16-bit left + right) */
typedef struct PCM16_stereo_s
{
  int16_t left;
  int16_t right;
} PCM16_stereo_t;


int main(void)
{
  FILE* file_p;
  FILE* inputdata_p;
  int frame[INTROLENGTH + FRAMELENGTH + OUTROLENGTH];

  int f, i, j;
  int leftbit;
  int polarity_left = 1;   /* Used to keep phase continuous when switching frequencies */
  int32_t FrameCount;

  PCM16_stereo_t *sample_p = NULL;
  sample_p = (PCM16_stereo_t *)malloc(sizeof(PCM16_stereo_t) * 1);
  if (sample_p == NULL) {
    fprintf(stderr, "ERROR: Failed to allocate sample buffer\n");
    return 1;
  }

  printf("=== datacassettewriter starting ===\n");
  printf("Configuration:\n");
  printf("  Frames           : %d\n", FRAMES);
  printf("  Data bits/frame  : %d\n", FRAMELENGTH);
  printf("  Header length    : %d bits\n", INTROLENGTH);
  printf("  Trailer length   : %d bits\n", OUTROLENGTH);
  printf("  Total data bits  : %d\n", DATALENGTH);
  printf("  Sample rate      : %d Hz\n", SAMPLERATE);
  printf("  Data rate        : %d bps\n", DATARATE);
  printf("  One frequency    : %d Hz\n", ONEFREQ);
  printf("  Zero frequency   : %d Hz\n", ZEROFREQ);
  printf("  Pause duration   : %d seconds\n\n", PAUSETIME);

  /* Open output WAV file */
  file_p = fopen("input.wav", "w");
  if (file_p == NULL) {
    fprintf(stderr, "ERROR: Could not open input.wav for writing\n");
    free(sample_p);
    return 1;
  }

  /* Calculate total number of audio samples we will write */
  FrameCount = 2 * PAUSETIME * SAMPLERATE + (BITLENGTH / DATARATE) * SAMPLERATE;
  printf("Total audio frames to write: %d\n", FrameCount);

  /* Write WAV header */
  write_PCM16_stereo_header(file_p, SAMPLERATE, FrameCount);

  /* Write leading silence (pause) so the tape deck can settle */
  printf("Writing %d seconds of leading silence...\n", PAUSETIME);
  for (i = 0; i < SAMPLERATE * PAUSETIME; i++) {
    sample_p[0].left  = 0;
    sample_p[0].right = 0;
    fwrite(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
  }

  /* Also create a text log of the generated bits */
  inputdata_p = fopen("input.txt", "w");
  if (inputdata_p == NULL) {
    fprintf(stderr, "ERROR: Could not open input.txt for writing\n");
    fclose(file_p);
    free(sample_p);
    return 1;
  }

  printf("\nGenerating and encoding frames...\n");

  /* Generate and encode each frame */
  for (f = 0; f < FRAMES; f++) {
    /* Progress every 512 frames to avoid flooding the console */
    if (f % 512 == 0) {
      printf("  Processing frame %d / %d (%.1f%%)\n",
             f, FRAMES, 100.0 * f / FRAMES);
    }

    fprintf(inputdata_p, "Frame %d\n", f);

    /* ---- Header: 11010010 ---- */
    frame[0] = 1;
    frame[1] = 1;
    frame[2] = 0;
    frame[3] = 1;
    frame[4] = 0;
    frame[5] = 0;
    frame[6] = 1;
    frame[7] = 0;
    fprintf(inputdata_p, "Header: 11010010\n");

    /* ---- Random payload data ---- */
    for (i = 0; i < FRAMELENGTH; i++) {
      frame[INTROLENGTH + i] = rand() & 1;
      fprintf(inputdata_p, "%d", frame[INTROLENGTH + i]);
    }

    /* ---- Trailer: 00101101 ---- */
    frame[FRAMELENGTH + INTROLENGTH + 0] = 0;
    frame[FRAMELENGTH + INTROLENGTH + 1] = 0;
    frame[FRAMELENGTH + INTROLENGTH + 2] = 1;
    frame[FRAMELENGTH + INTROLENGTH + 3] = 0;
    frame[FRAMELENGTH + INTROLENGTH + 4] = 1;
    frame[FRAMELENGTH + INTROLENGTH + 5] = 1;
    frame[FRAMELENGTH + INTROLENGTH + 6] = 0;
    frame[FRAMELENGTH + INTROLENGTH + 7] = 1;
    fprintf(inputdata_p, "\nTrailer: 00101101\n");

    /* Encode every bit of the current frame into audio samples */
    for (i = 0; i < FRAMELENGTH + INTROLENGTH + OUTROLENGTH; i++) {
      leftbit = frame[i];

      /* Generate one bit period of audio (SAMPLERATE/DATARATE samples) */
      for (j = 0; j < SAMPLERATE / DATARATE; j++) {
        if (leftbit == 1) {
          /* Full sine wave at ONEFREQ for a '1' */
          sample_p[0].left = (int16_t)(polarity_left * 32767.0 *
                                       sin(2 * PI * j * ONEFREQ / SAMPLERATE));
        } else {
          /* Half sine wave at ZEROFREQ for a '0' */
          sample_p[0].left = (int16_t)(polarity_left * 32767.0 *
                                       sin(2 * PI * j * ZEROFREQ / SAMPLERATE));
        }
        /* Right channel currently left silent */
        sample_p[0].right = 0;
        fwrite(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
      }

      /* Invert polarity after a zero to keep the waveform continuous */
      if (leftbit == 0) {
        polarity_left = -polarity_left;
      }
    }
  }

  printf("  Finished all %d frames.\n", FRAMES);

  /* Write trailing silence */
  printf("Writing %d seconds of trailing silence...\n", PAUSETIME);
  for (i = 0; i < SAMPLERATE * PAUSETIME; i++) {
    sample_p[0].left  = 0;
    sample_p[0].right = 0;
    fwrite(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
  }

  fclose(file_p);
  fclose(inputdata_p);
  free(sample_p);

  printf("\n=== datacassettewriter finished successfully ===\n");
  printf("Created files: input.wav  and  input.txt\n");
  return 0;
}
