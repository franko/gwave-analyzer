#pragma once

#include "wav_reader.h"

extern void wav2midi_set_corr_arm(double corr_arm);
extern void wav2midi_set_wavvis_offset(int offset);
extern void wav2midi_do_conversion(wav_reader_t *wav);
extern int wav2midi_init();
