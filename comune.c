#include <assert.h>
#include <string.h>

#include "comune.h"
#include "wav_reader.h"

wav_visual_t wav_vis = { 0, 2048, 1.0 };
event_list_t ev_list;

double *cv, *fp, *lp;
int cvall, fpall;

void
in_alloca() {
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
