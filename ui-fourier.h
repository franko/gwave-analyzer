#pragma once

#include <ui.h>

extern uiWindow *gwave_open_fourier_window();
extern void compute_fourier(wav_reader_t *wav, int sample_start, int sample_size);
