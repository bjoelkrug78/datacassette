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

/* Data structure to hold a single frame with two channels */
typedef struct PCM16_stereo_s
{
    int16_t left;
    int16_t right;
} PCM16_stereo_t;


void flush_last_sequence(FILE* outputdata_p, int* onecount, int* zerocount, int* databits, uint8_t* headerdetect, int* frames, int* frameopen, int* payload, int noise)
{

  int onesequence, zerosequence, i, n;

  /* flush last sequence detected */
  /* detect sequence of ones or zeros */
  if((*onecount>=2 && *zerocount>0) || (*onecount>=1 && noise))
  {
    /* end of one sequence */
    onesequence = (int)((((double)*onecount)/2.0)+0.49);
    for(i=0; i<onesequence; i++) 
    {
      if(*frameopen==1) *payload=*payload+1;
      if(*frameopen==1 && *payload<=1024) fprintf(outputdata_p, "1");
   
      // header detection
      *headerdetect=(*headerdetect << 1) + 1;
      if(*headerdetect == 210 && *frameopen==0) 
      {
        printf("Detected header at %d, opened frame %d\n", *databits, *frames);
        fprintf(outputdata_p, "Frame %d\n", *frames);
        fprintf(outputdata_p, "Header: 11010010\n"); 
        *frames=*frames+1;
        *frameopen=1;
        *payload=0;
      };
      if(*frameopen==1 && *payload==1032)
      {
        *frameopen=0;
        fprintf(outputdata_p, "\nTrailer: 00101101\n");
      };

    };
    *databits=*databits+onesequence;
    *onecount=0;

  } else
  if((*zerocount>=1 && *onecount>0) || (*zerocount>=1 && noise))
  {
    /* end of zero sequence */
    zerosequence = (int)((((double)*zerocount)/1.0)+0.49);
    for(i=0; i<zerosequence; i++) 
    {
      if(*frameopen==1) *payload=*payload+1;
      if(*frameopen==1 && *payload<=1024) fprintf(outputdata_p, "0");

      // header detection
      *headerdetect=(*headerdetect << 1);
      if(*headerdetect == 210 && *frameopen==0) 
      {
        printf("Detected header at %d, opened frame %d\n", *databits, *frames);
        fprintf(outputdata_p, "Frame %d\n", *frames);
        fprintf(outputdata_p, "Header: 11010010\n");
        *frames=*frames+1;
        *frameopen=1;
        *payload=0;
      };
      if(*frameopen==1 && *payload==1032)
      {
        *frameopen=0;
        fprintf(outputdata_p, "\nTrailer: 00101101\n");
      };

    };
    *databits=*databits+zerosequence;
    *zerocount=0;

  };

}



int main(void)
{
  FILE* file_p;
  FILE* outputdata_p;

  int i, j, k, leftbit, rightbit;
  int leftlast, rightlast, leftnow, rightnow;
  int leftcount=0, rightcount=0;
  int onecount=0, zerocount=0;
  int onesequence=0, zerosequence=0;
  int32_t FrameCount;
  int samplenumber=0, databits=0;
  uint8_t headerdetect=0;
  int frames=0;
  int frameopen=0;
  int payload=0;

  /* performance measures */
  int leftmax=0;
  int leftmin=0;
  int leftmaxperiod=0;
  int leftmaxone=0;
  int leftminone=100000000;
  int leftmaxzero=0;
  int leftminzero=100000000;
  int betrag;
  int leftintegral=0;
  int leftmaxonearea=0;
  int leftminonearea=100000000;
  int leftmaxzeroarea=0;
  int leftminzeroarea=100000000;

  PCM16_stereo_t *sample_p = NULL;
  sample_p = (PCM16_stereo_t *)malloc(sizeof(PCM16_stereo_t) * 1);

  /* Open the wav file */
  file_p = fopen("output.wav", "r");

  /* Open the output file */
  outputdata_p = fopen("output.txt", "w");

  /* Skip the header */
  for(i=0; i<11; i++)
  {
    fread(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
  };

  /* Read the data */
  fread(sample_p, sizeof(PCM16_stereo_t), 1, file_p);
  leftlast=sample_p[0].left;
  samplenumber++;
  while(fread(sample_p, sizeof(PCM16_stereo_t), 1, file_p))
  {
    samplenumber++;
    leftnow=sample_p[0].left;
    leftintegral=leftintegral+leftnow;
    leftcount++;
  
    if(leftnow > leftmax) leftmax=leftnow; 
    if(leftnow < leftmin) leftmin=leftnow; 

    /* detect zero crossing */
    if((leftlast>=0 && leftnow<0) || (leftlast<=0 && leftnow>0)) 
    {
      /* rising or falling edge */
      /* detect data */
      if(leftmax > 700 || leftmin < -700)
      {
        if(leftcount<=7) 
        {
          if(leftmax > 700) betrag=leftmax;
          if(leftmin < -700) betrag=-leftmin;
          if(betrag>leftmaxone) leftmaxone=betrag;
          if(betrag<leftminone) leftminone=betrag;
          if(leftintegral<0) leftintegral=-leftintegral;
          if(leftintegral>leftmaxonearea) leftmaxonearea=leftintegral;
          if(leftintegral<leftminonearea) leftminonearea=leftintegral;
          onecount++;
          leftintegral=0;
        };
        if(leftcount>=8) 
        {
          /* first bit is known to be a 1 */
          if(databits==0 && onecount==0) 
          {
            onecount++;
          } else
          {
            if(leftmax > 700) betrag=leftmax;
            if(leftmin < -700) betrag=-leftmin;
            if(betrag>leftmaxzero) leftmaxzero=betrag;
            if(betrag<leftminzero) leftminzero=betrag;
            if(leftintegral<0) leftintegral=-leftintegral;
            if(leftintegral>leftmaxzeroarea) leftmaxzeroarea=leftintegral;
            if(leftintegral<leftminzeroarea) leftminzeroarea=leftintegral;
            zerocount++; 
            leftintegral=0;
          };
        };
      } else
      {
        /* Noise detected */
        flush_last_sequence(outputdata_p, &onecount, &zerocount, &databits, &headerdetect, &frames, &frameopen, &payload, 1);
        onecount=0;
        zerocount=0;
      };
      leftcount=0;
      leftmax=0;
      leftmin=0;
    } else if(leftlast==0 && leftnow==0)
    { 
      /* Silence detected */
      if(onecount!=0 || zerocount!=0)
      {
        // the last half of a one could be lost
        if(onecount==1) onecount++;
        flush_last_sequence(outputdata_p, &onecount, &zerocount, &databits, &headerdetect, &frames, &frameopen, &payload, 1);
      };
      leftcount=0;
      leftmax=0;
      leftmin=0;
    }; 
 
    flush_last_sequence(outputdata_p, &onecount, &zerocount, &databits, &headerdetect, &frames, &frameopen, &payload, 0);

    /* store the current value */
    leftlast=sample_p[0].left;
  };

  flush_last_sequence(outputdata_p, &onecount, &zerocount, &databits, &headerdetect, &frames, &frameopen, &payload, 1);

  printf("Number of samples processed: %d\n", samplenumber);
  printf("Number of data bits recovered: %d\n", databits);
  printf("Amplitude for 1s: max %d, min %d\n", leftmaxone, leftminone);
  printf("Amplitude for 0s: max %d, min %d\n", leftmaxzero, leftminzero);
  printf("Area for 1s: max %d, min %d\n", leftmaxonearea, leftminonearea);
  printf("Area for 0s: max %d, min %d\n", leftmaxzeroarea, leftminzeroarea);

  fclose(file_p);
  fclose(outputdata_p);
  free(sample_p);
  return 0;    
}

