#pragma once
#include "wav_reader.h"

extern int wav_read_fourier(wav_reader_t *wav, long int noc, int loc_offs);

extern double detfreq(wav_reader_t *wav, long int nod, char *puro, char *nullo, double *chiq, double *pintens, int loc_offs);
