#pragma once
#include "comune.h"
#include "wav_reader.h"

// Amplitude below which the sound will be considered null.
#define MAXAMPTOL 0.05

typedef struct {
    long int offs_inizio;
    int durata_nota;
    int nota;
    int pd;
    int nd;
} event_t;

typedef struct
{
    int offset;
    int vis_len;
    double y_magn;
}
wav_visual_t;

typedef struct
{
    event_t *begin;
    int number;
    int alloc;
}
event_list_t;

/*  extern long int dur, offs, l_filedur; */
/*  extern int freq, lunw, lun_sam; */
/*  extern int stereo; */
/*  extern int canal_method; */
extern wav_visual_t wav_vis;
extern event_list_t ev_list;

extern double corr_arm;

extern double *cv, *fp, *lp;
extern int cvall, fpall;
extern double filedur;

extern void nome_nota(int nota, char *str);
