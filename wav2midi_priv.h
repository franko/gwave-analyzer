#pragma once
#include "comune.h"
#include "wav_reader.h"

// Amplitude below which the sound will be considered null.
#define MAXAMPTOL 0.05

/* [NOV-2018] This struct represent an musical note that is played at some moment.
   - offs_inizio, start time of the note, in samples number (not used when writing midi file)
   - dominant_freq, frequency on the dominant in Hz
   - nota, the musical note, in semitone, given by 12 * (log2(freq) - 8.2)) + 62
   - pd, duration time of the silence following the note (in 1/200 of seconds)
   - nd, duration time of the note (in the same units of "pd")
*/
typedef struct {
    long int offs_inizio;
    float dominant_freq;
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
extern void event_list_init(event_list_t *list, const int init_alloc);
extern void event_list_check_size(event_list_t *list, const int size);
