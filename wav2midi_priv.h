#pragma once
#include "comune.h"
#include "wav_reader.h"

// Amplitude below which the sound will be considered null.
#define MAXAMPTOL 0.05

/* [NOV-2018] This struct represent an musical note that is played at some moment.
   - offs_inizio, start time of the note, in samples number (not used when writing midi file)
   - nota, the musical note, I guess is a integer encoding counting the semitones
   - pd, duration time of the silence following the note (in some units)
   - nd, duration time of the note (in some units, the same of "pd")
*/
typedef struct {
    long int offs_inizio;
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
