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

extern long wav_reader_offset(wav_reader_t *wav, int pos);
extern int wav_reader_seek(wav_reader_t *wav, int pos);
extern void wav_reader_init(wav_reader_t *wav, const char *filename, char status_message[], const int status_message_len);
extern void wav_reader_set_zero(wav_reader_t *wav);

extern int getone(wav_reader_t *wav, double x[]);
extern void getdata(wav_reader_t *wav, double x[], long int loc_offs, long int *noc, int ascomp);
