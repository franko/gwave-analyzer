#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "comune.h"

extern long int wtm_true_timeout( );

#define NOPZ 3
#define NOPZS 1
static char *wav_head_1 = "RIFF", *wav_head_2 = "WAVEfmt ";
static char *opz[NOPZ] = { "--offs", "-c", "--corr-arm" };
static char *opzs[NOPZS] = { "--dummy" };

#define STR_LEN 160
char status_msg[ STR_LEN + 1 ] = "";

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

static void det_file_durata() {
    if (inpf) {
        fseek(inpf, 0, SEEK_END);
        wav_spec.num_sample = (ftell(inpf) - WHEAD) / wav_spec.lun_sample;

        if (wav_spec.num_sample < 0) {
            wav_spec.num_sample = 0;
        }

        imm_incomp = ( wav_spec.num_sample < wav_vis.vis_len );
    }
}

static void analyze_wav_header() {
    int wavefile, len;
    char head[WHEAD+2];
    if ( ! inpf )
        {
            strcpy( status_msg, "No file selected." );
            return;
        }

    wavefile = (fseek( inpf, 0, SEEK_SET ) == 0);

    if ( ! wavefile )
        goto alarm;

    wavefile = ( fread( head, 1, WHEAD+2, inpf ) == WHEAD+2 );
    if ( ! wavefile )
        goto alarm;
    wavefile = ( strncmp( head, wav_head_1, 4 ) == 0 );
    wavefile = wavefile && ( strncmp( head + 8, wav_head_2, 4 ) == 0 );

 alarm:
    if ( ! wavefile )
        {
            strcpy( status_msg,
                            "The file selected seems to be not a RIFF wav file." );
            wav_spec.num_sample = 0;
            fprintf(stderr, "wav header: invalid file\n");
            return;
        }

    wav_spec.freq = (int) ( head[24] + 256*(unsigned char) head[25] );

    wav_spec.lun_word = head[34] / 8;

    if ( head[32] / wav_spec.lun_word == 2 )
        wav_spec.num_can = 2;
    else
        {
            wav_spec.num_can = 1;
            wav_vis.canal_method = 1;
        }
    wav_spec.lun_sample = wav_spec.lun_word * wav_spec.num_can;

    strip_filename();

    det_file_durata();

    len = *(int *) &( head[40] );
    len /= wav_spec.lun_sample;

    snprintf(status_msg, STR_LEN,
                "File %s, Sample Freq. %i, Channel num. %i, byte/sample %i, Sample number %i",
                stripped_filename, wav_spec.freq, wav_spec.num_can,
                wav_spec.lun_word, wav_spec.num_sample);

    fprintf(stderr, "wav header freq: %d channels: %i bps: %d samples: %d\n", wav_spec.freq, wav_spec.num_can, wav_spec.lun_word, wav_spec.num_sample);

    if ( len != wav_spec.num_sample )
        {
            const char *msg1 = ", bad file";
            const int len_1 = strlen( status_msg ) + strlen( msg1 ) - STR_LEN;
            if ( len_1 <= 0 )
                strcat( status_msg, msg1 );
            else
                strncat( status_msg, msg1, strlen( msg1 ) - len_1 );
        }
}

static void wav_to_midi() {
    int status = wtm_true_timeout(1);
    while (status >= 0) {
        status = wtm_true_timeout(0);
    }
}

int main (int argc, char *argv[]) {
    int ce, i, j, ce_file = 0;

    in_alloca();

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
                    wav_vis.canal_method = atoi(argv[i + 1]);
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
        inpf = fopen( filename, "r" );
    } else {
        inpf = (FILE *) NULL;
    }

    analyze_wav_header();

    if (ce_file) {
        wav_to_midi();
    }
    return 0;
}
