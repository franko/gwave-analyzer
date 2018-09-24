#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "debug_log.h"
#include "wav_reader.h"

#define WHEAD 44

static char *wav_head_1 = "RIFF", *wav_head_2 = "WAVEfmt ";

long wav_reader_offset(wav_reader_t *wav, int pos) {
    return pos * wav->spec.lun_sample + WHEAD;
}

int wav_reader_seek(wav_reader_t *wav, int pos) {
    const long offset = wav_reader_offset(wav, pos);
    return fseek(wav->file, offset, SEEK_SET);
}

static int read_wav_header(wav_reader_t *wav, char header[]) {
    const long byte_reads = fread(header, 1, WHEAD + 2, wav->file);
    return (byte_reads == WHEAD + 2 ? 0 : 1);
}

static void get_wav_sample_number(wav_reader_t *wav) {
    if (wav->file) {
        fseek(wav->file, 0, SEEK_END);
        wav->spec.num_sample = (ftell(wav->file) - WHEAD) / wav->spec.lun_sample;

        if (wav->spec.num_sample < 0) {
            wav->spec.num_sample = 0;
        }
    }
}

void wav_reader_set_zero(wav_reader_t *wav) {
    wav->file = (FILE *) NULL;
    const wav_spec_t spec_init = {44100, 1, 2, 2, 0};
    wav->spec = spec_init;
    wav->canal_method = 1;
}

int wav_reader_init(wav_reader_t *wav, const char *filename) {
    int len;
    char head[WHEAD+2];
    if (!wav->file) {
        return WAV_READER_NO_FILE;
    }

    int wavefile = (wav_reader_seek(wav, 0) == 0);

    if (!wavefile)
        goto alarm;

    if (read_wav_header(wav, head)) {
        goto alarm;
    }
    wavefile = ( strncmp( head, wav_head_1, 4 ) == 0 );
    wavefile = wavefile && ( strncmp( head + 8, wav_head_2, 4 ) == 0 );

 alarm:
    if (!wavefile) {
        wav->spec.num_sample = 0;
        return WAV_READER_UNKNOWN_FORMAT;
    }

    wav->spec.freq = (int) ( head[24] + 256*(unsigned char) head[25] );

    wav->spec.lun_word = head[34] / 8;

    if (head[32] / wav->spec.lun_word == 2) {
        wav->spec.num_can = 2;
    } else {
        wav->spec.num_can = 1;
    }
    wav->spec.lun_sample = wav->spec.lun_word * wav->spec.num_can;

    get_wav_sample_number(wav);

    len = *(int *) &( head[40] );
    len /= wav->spec.lun_sample;

    debug_log("File %s, Sample Freq. %i, Channel num. %i, byte/sample %i, Sample number %i",
                filename, wav->spec.freq, wav->spec.num_can,
                wav->spec.lun_word, wav->spec.num_sample);

    if (len > wav->spec.num_sample) {
        return WAV_READER_ILL_FORMED;
    }
    return WAV_READER_SUCCESS;
}

int getone(wav_reader_t *wav, double *num) {
    int ij, car[2];
    double x[2];

    assert(wav->spec.num_can <= 2);

    for ( ij = 0; ij < wav->spec.num_can; ij++ )
        {
            car[0] = fgetc(wav->file);
            if (wav->spec.lun_word == 2)
                {
                    car[1] = fgetc(wav->file);
                    if ( car[1] == -1 ) return 1;
                    if ( car[1] >= 128 ) car[1] -= 256;
                    x[ij] = ( car[0]+256*car[1] )/(double)32768;
                }
            else
                {
                    if ( car[0] == -1 ) return 1;
                    x[ij] = ( car[0] - 128 )/(double)128;
                }
        }

    *num = x[0];

    if (wav->spec.num_can == 1 || wav->canal_method == 1)
        return 0;

    if (wav->canal_method == 0)
        *num = ( *num + x[1] )/2;
    else
        *num = x[1];

    return 0;
}

void getdata(wav_reader_t *wav, double *dat, long int loc_offs, long int *noc, int ascomp) {
    int f = 1;
    long int j = 0;
    double x;

    if ( ascomp ) f = 2;

    if (wav_reader_seek(wav, loc_offs)) exit(0);
    do {
        getone(wav, &x);
        if ( ascomp ) dat[ f*j+1 ] = 0;
        dat[f*j] = x;
        j++;
    } while ( j < *noc );
    if ( j < *noc ) *noc = j;
}
