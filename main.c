#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "wav2midi.h"
#include "wav_reader.h"

#define NOPZ 3
#define NOPZS 1
static char *opz[NOPZ] = { "--offs", "-c", "--corr-arm" };
static char *opzs[NOPZS] = { "--dummy" };

#define STR_LEN 160
char *stripped_filename;
char filename[ STR_LEN + 1 ] = "";

static FILE *fopen_binary_read(const char *filename) {
#ifdef WIN32
    return fopen(filename, "rb");
#else
    return fopen(filename, "r");
#endif
}

static void strip_filename() {
    char *pt = strchr(filename, 0);
    while ( pt > filename && *pt != '/' ) {
        pt--;
    }
    if (*pt == '/') pt++;
    stripped_filename = pt;
}

int main (int argc, char *argv[]) {
    int ce, i, j, ce_file = 0;

    wav2midi_init();

    wav_reader_t wav[1];
    wav_reader_set_zero(wav);

    double corr_arm = 0.0;
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            for (j = 0; j < NOPZ; j++) {
                if ((ce = !strcmp(opz[j], argv[i]))) break;
            }
            if (ce) {
                switch (j) {
                case 0:
                    wav2midi_set_wavvis_offset(atoi(argv[i + 1]));
                    break;
                case 1:
                    wav->canal_method = atoi(argv[i + 1]);
                    break;
                case 2:
                    corr_arm = strtod(argv[i+1], NULL);
                    wav2midi_set_corr_arm(corr_arm);
                    printf("Corr. arm :%f\n", corr_arm);
                }
                i++;
            } else {
                for (j=0; j < NOPZS; j++) {
                    if ((ce = !strcmp(opzs[j], argv[i]))) break;
                }
                if (ce) {
                    switch (j) {
                    case 0:
                        puts("Dummy!\n");
                    }
                } else {
                    strncpy(filename, argv[i], STR_LEN + 1);
                    ce_file = 1;
                }
            }
        }
    }

    if (ce_file) {
        wav->file = fopen_binary_read(filename);
    } else {
        wav->file = (FILE *) NULL;
    }

    strip_filename();
    int wav_read_status = wav_reader_init(wav, stripped_filename);
    if (wav_read_status != WAV_READER_SUCCESS) {
        fprintf(stderr, "Error reading wav file %s. Error code: %d.\n", filename, wav_read_status);
        return 1;
    }

    if (wav->canal_method > wav->spec.num_can) {
        wav->canal_method = 1;
    }

    if (ce_file) {
        wav2midi_do_conversion(wav);
    }
    return 0;
}
