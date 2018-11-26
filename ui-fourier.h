#pragma once
#include <ui.h>
#include "wav_reader.h"

extern void open_fourier_window(wav_reader_t *wav, int sample_start, int sample_size);
extern void update_fourier_window(wav_reader_t *wav, int sample_start, int sample_size);
