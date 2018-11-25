#pragma once

#include <ui.h>
#include "wav_reader.h"

extern void open_fourier_window();
extern void compute_fourier(wav_reader_t *wav, int sample_start, int sample_size);
extern void refresh_fourier_window();
