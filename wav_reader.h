#pragma once
#include "comune.h"

typedef struct {
    int freq;
    int num_can;
    int lun_sample;
    int lun_word;
    int num_sample;
} wav_spec_t;

typedef struct {
    FILE *file;
    wav_spec_t spec;
    int canal_method;
} wav_reader_t;

enum {
    WAV_READER_SUCCESS = 0,
    WAV_READER_NO_FILE,
    WAV_READER_UNKNOWN_FORMAT,
    WAV_READER_ILL_FORMED,
};

extern long wav_reader_offset(wav_reader_t *wav, int pos);
extern int wav_reader_seek(wav_reader_t *wav, int pos);
extern int wav_reader_init(wav_reader_t *wav, const char *filename);
extern void wav_reader_set_zero(wav_reader_t *wav);

extern int getone(wav_reader_t *wav, double x[]);
/* [NOV-2018] The parameter "ascomp" specify if the data should be loaded as complex number.
   If true data will be loaded interleaved with zeroes for the imaginary part of each
   sample. */
extern void getdata(wav_reader_t *wav, double x[], long int loc_offs, long int *noc, int ascomp);
