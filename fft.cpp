#include <cmath>

#include "SignalReader.h"

#define TAU 6.28318530717958647692529

static unsigned pow2(int n) {
    return 1 << n;
}

static int ceiltopow (int n) {
    int out = 1;
    while (out < n) {
        out = out << 1;
    }
    return out;
}

static int log2(int n) {
    int j = 0;
    while (n > 1) {
        n = n >> 1;
        j ++;
    }
    return j;
}

static void fft(double value[], const int n) {
    int j = 1;
    int m = n / 2;
    const int log_n = log2(n);

    while (j < n) {
        const double temr = value[2 * j];
        const double temi = value[2 * j + 1];
        value[2 * j    ] = value[2 * m];
        value[2 * j + 1] = value[2 * m + 1];
        value[2 * m    ] = temr;
        value[2 * m + 1] = temi;

        do {
            j++;
            int i = n / 2;
            do {
                if (m < i) {
                    m += i;
                    break;
                } else {
                    m -= i;
                    i /= 2;
                }
            } while (i > 0);
        } while ((m <= j) && (j < n));
    }
    int pd = 1;
    for (int i= 0; i < log_n; i++) {
        pd = 2 * pd;
        const double wtr = std::cos(TAU / double(pd));
        const double wti = std::sin(TAU / double(pd));
        for (j= 0; j < 2 * n; j += 2 * pd) {
            double wr= 1.0, wi= 0.0;
            for (int k = j; k < j + pd; k += 2) {
                const double temr = value[ k+pd ];
                const double temi = value[ k+pd+1 ];
                value[k + pd]     = value[ k   ] - wr * temr + wi * temi;
                value[k + pd + 1] = value[ k+1 ] - wi * temr - wr * temi;
                value[k]     = value[k]     + wr * temr - wi * temi;
                value[k + 1] = value[k + 1] + wi * temr + wr * temi;
                const double wrsave = wr;
                wr = wtr * wr - wi * wti;
                wi = wtr * wi + wrsave * wti;
            }
        }
    }
}

/* Read m samples from signal where m < n and n = 2^p for some integer p. 
   The signal is interpolated using n sampling point at regular intervals and
   written into buffer. */
static void read_samples_pow2_interp(SignalReader& reader, double buffer[], const int m, const int n) {
        const double wavelength_step = double(m) / double(n);
        double ya = reader.read_sample();
        double yb = reader.read_sample();
        double ir = 0.0;
        for (int i = 0, iad = 1; i < n; i++, ir += wavelength_step) {
            int imin = int(ir);
            if (imin >= iad) {
                ya = yb;
                yb = reader.read_sample();
                iad++;
            }
            buffer[2 * i] = ya * (imin + 1 - ir) + yb * (ir - imin);
            buffer[2 * i + 1] = 0.0;
        }
}

int fft_wav(SignalReader& reader, const int wav_position, const int sample_number, FFTWorkspace& workspace) {
    const int sample_number_2p = ceiltopow(sample_number);
    workspace.ensure_size(sample_number_2p);
    double *buffer = workspace.buffer();
    if (sample_number_2p == sample_number) {
        reader.read_samples(wav_position, sample_number_2p, buffer, SignalReader::complex_sample);
    } else {
        reader.move_to(wav_position);
        read_samples_pow2_interp(reader, buffer, sample_number, sample_number_2p);
    }
    fft(buffer, sample_number_2p);
    return 0;
}
