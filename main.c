#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "comune.h"
#include "wav_reader.h"

extern long int wav_to_midi_stepping(wav_reader_t *wav, char first_time);

#define NOPZ 3
#define NOPZS 1
static char *opz[NOPZ] = { "--offs", "-c", "--corr-arm" };
static char *opzs[NOPZS] = { "--dummy" };

#define STR_LEN 160
char *stripped_filename;
char filename[ STR_LEN + 1 ] = "";

char *note_lett[] = { "Do", "Re", "Mi", "Fa", "Sol", "La", "Si" };
int imm_incomp;
double corr_arm;

static void strip_filename() {
    char *pt = strchr(filename, 0);
    while ( pt > filename && *pt != '/' ) {
        pt--;
    }
    if (*pt == '/') pt++;
    stripped_filename = pt;
}

static void wav_to_midi(wav_reader_t *wav) {
    int status = wav_to_midi_stepping(wav, 1);
    while (status >= 0) {
        status = wav_to_midi_stepping(wav, 0);
    }
}

int main (int argc, char *argv[]) {
    int ce, i, j, ce_file = 0;

    in_alloca();

    wav_reader_t wav[1];
    wav_reader_set_zero(wav);

    corr_arm = 0;
    if (argc > 1) {
        for (i = 1; i < argc; i++) {
            for (j = 0; j < NOPZ; j++) {
                if ((ce = !strcmp(opz[j], argv[i]))) break;
            }
            if (ce) {
                switch (j) {
                case 0:
                    wav_vis.offset = atoi(argv[i + 1]);
                    break;
                case 1:
                    wav->canal_method = atoi(argv[i + 1]);
                    break;
                case 2:
                    corr_arm = strtod(argv[i+1], NULL);
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
        wav->file = fopen( filename, "r" );
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
    imm_incomp = (wav->spec.num_sample < wav_vis.vis_len);

    if (ce_file) {
        wav_to_midi(wav);
    }
    return 0;
}
