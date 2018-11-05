#include <string.h>

#include "wav2midi.h"
#include "wav2midi_priv.h"

extern long int wav_to_midi_stepping(wav_reader_t *wav, char first_time);

double corr_arm;

wav_visual_t wav_vis = { 0, 2048, 1.0 };
event_list_t ev_list;

double *cv, *fp, *lp;
int cvall, fpall;

static char *note_lett[] = { "Do", "Re", "Mi", "Fa", "Sol", "La", "Si" };

static void workspace_memory_alloc() {
    cv = (double *) xmalloc( 2*128*sizeof(double) ); cvall = 128;

    fpall = 256;
    fp = (double *) xmalloc( fpall*sizeof(double) );
    lp = (double *) xmalloc( 2*fpall*sizeof(double) );

    ev_list.alloc = 30;
    ev_list.begin = (event_t *) xmalloc( ev_list.alloc * sizeof(event_t) );
}

void nome_nota( int nota, char *str ) {
    int ff = (nota-2)-12*((nota-2)/12), rim;
    if ( ff >= 5 ) ff++;
    rim = ff % 2;
    ff /= 2;
    strcpy( str, note_lett[ff] );
    if ( rim ) strcat( str, "#" );
}

    void wav2midi_do_conversion(wav_reader_t *wav) {
    int status = wav_to_midi_stepping(wav, 1);
    while (status >= 0) {
        status = wav_to_midi_stepping(wav, 0);
    }
}

void wav2midi_set_corr_arm(double new_corr_arm) {
    corr_arm = new_corr_arm;
}

void wav2midi_set_wavvis_offset(int offset) {
    wav_vis.offset = offset;
}

int wav2midi_init() {
    corr_arm = 0.0;
    workspace_memory_alloc();
    return 0;
}
