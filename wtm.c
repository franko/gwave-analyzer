#include <string.h>
#include <gtk/gtk.h>
#include "comune.h"

double corr_arm;

static long int minnod = 0, defnod = 0;
static FILE *outf;
static char midi_data[]= {
    0x4d, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06, 
    0x00, 0x00, 0x00, 0x01, 0x00, 0x78, 0x4d, 0x54, 
    0x72, 0x6b, 0x00, 0x00, 0x60, 0x41, 0x00, 0xff, 
    0x03, 0x0c, 0x53, 0x65, 0x6e, 0x7a, 0x61, 0x20, 
    0x54, 0x69, 0x74, 0x6f, 0x6c, 0x6f, 0x00, 0xff, 
    0x01, 0x18, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 
    0x74, 0x6f, 0x20, 0x64, 0x61, 0x20, 0x45, 0x78, 
    0x74, 0x72, 0x6f, 0x76, 0x65, 0x72, 0x73, 0x69, 
    0x74, 0x79, 0x00, 0xff, 0x58, 0x04, 0x04, 0x02, 
    0x18, 0x08, 0x00, 0xff, 0x59, 0x02, 0x00, 0x00, 
    0x00, 0xff, 0x51, 0x03, 0x09, 0x27, 0xc0, 0x00 };

int mididur( int dur, FILE *loutf ) {
    char ch[10];
    int i, j = 0;
    while ( dur >= 128 ) {
        ch[j] = dur - 128*( dur/128 );
        dur /= 128;
        j++;
    }
    ch[j] = dur;
    for ( i=j; i>0; i-- )
        fputc( 128 + ch[i], loutf );
    fputc( ch[0], loutf );
    return j+1;
} 

inline double camp2time( long int ncam ) {
    return ncam/(double)wav_spec.freq;
}

int freq2nota( double fr ) {
    int nota;
    nota = (int) rint( 12*( LOG2D( fr ) - 8.2 ) + corr_arm + 62 );
    if ( nota < 0 || nota > 255 ) nota = 0;
    return nota;
}

int camp2dur( long int ncam ) {
    int dur;
    dur = ( 200*ncam )/(double)wav_spec.freq;
    // dur = 15*(dur/15);
    //  if ( dur < 5 ) dur = 0;  Punto critico
    return dur;
}

int write_midi() {
    int dur, i, strum = 75;
    char buf[4], str_nota[11];
    FILE *text_f;

    text_f = fopen( "melodia.txt", "w" );

    for ( i=0; i<18; i++ )
        fputc( midi_data[i], outf );
/*   Primi 18 byte del file midi */

    for ( i=4; i>0; i--)
        fputc( 0, outf );

    for ( i=22; i<88; i++ )
        fputc( midi_data[i], outf );

    fprintf( outf, "%c%c%c%c", 192, strum, 0, 144 );

    dur = 70; /* 66 + 4(strum. spec.) */ 

    for ( i = 0; i < ev_list.number; i++ )
        {
            fputc( ev_list.begin [ i ].nota, outf );
            fputc( 'd', outf );
            dur += mididur( ev_list.begin [ i ].nd, outf );
            fputc( ev_list.begin [ i ].nota, outf );
            fputc( 0, outf );
            
            dur += mididur( ev_list.begin [ i ].pd, outf );
        
            if ( text_f )
                {
                    nome_nota( ev_list.begin [ i ].nota, str_nota );
                    fprintf( text_f, "%4s %3i, %3i, %3i\n", str_nota,
                	   ev_list.begin [ i ].nota, ev_list.begin [ i ].nd,
                	   ev_list.begin [ i ].pd );
                }
        }
    dur += 4*ev_list.number + 3;

    fprintf( outf, "%c%c%c", 255, 47 , 0 );
    
    /* Now we come back to write the correct duration of the track */
    fseek( outf, 18, SEEK_SET );
    for ( i=4; i>0; i--)
        {
            buf[i-1] = dur % 256;
            dur /= 256;
        }
    for ( i=0; i<4; i++)
        fputc( buf[i], outf );

    fflush( outf );
    fclose( outf );

    fprintf( text_f, "FINE\n" );
    fclose( text_f );

#if 1
    g_message( "\nThe melodic analisys has been successfully executed.\nThe result is written in the file melody.mid" );
#else
    g_message( "\nL'analisi melodica del file e' stata eseguita con successo.\nLa melodia si trova scritta nel file prova.mid" );
#endif
    return 0;
}

long int
wtm_true_timeout( char first_time ) {
    static long int nn, ii, jj;
    static int trovata, pausa, offs;
    static int o_nota;
    static char eoinpf, anal_fatt;
    static double intens, fr, chiq;
    static int lnod, nota;
    static char puro, nullo;
    static event_t eva;

    if ( first_time ) {
        int tmp;
        if ( ! inpf ) return -1;
        if ( ! ( outf = fopen( "melody.mid", "w" ) ) ) return -1;

        defnod = ceiltopow( ( 2048 / (double)44100.0 )* wav_spec.freq );
        tmp = ( 512  / (double)44100.0 )* wav_spec.freq;
        minnod = ceiltopow( tmp );
        if ( minnod > tmp && minnod >= 2 )
            minnod /= 2;

        ev_list.number = 0;
    
        anal_fatt = 0; 
        nn = 0;
        eoinpf = 0;
        jj = 0; ii = 0;
        pausa = 0;
        trovata = 0;
        offs = 0;
        eva.offs_inizio = 0;
    }

    if ( anal_fatt )
        anal_fatt = 0;
    else {
        lnod = defnod;
        while ( lnod >= minnod ) {
            fr = detfreq( lnod, &puro, &nullo, &chiq, &intens, offs );
            if ( ( eoinpf = ( fr <= 0 ) ) ) break;
            nullo = nullo || (intens/(double)lnod) < 0.0001; // Punto critico
            puro = puro && chiq <= 1 && chiq >= 0;
            if ( nullo || puro ) break;
            lnod /= 2;
        }
        if ( eoinpf ) goto as_break;
        if ( lnod < minnod ) lnod = minnod;
        offs += lnod;
    }

    if ( nullo ) {
        pausa = 1;
        if ( trovata )
            jj += nn;
        else
            ii += nn;
        ii += lnod;
        nn = 0;
        return offs;
    }

    if ( pausa ) goto as_break;

    if ( ! puro )
        nn += lnod;
    else {
        nota = freq2nota( fr );
        if ( trovata )
            if ( nota == o_nota ) {
                jj += nn + lnod;
                nn = 0;
            }
            else {
                jj += nn/2;
                nn -= nn/2;
                goto as_break;
            }
        else {
            trovata = 1;
            jj += nn + lnod;
            nn = 0;
            o_nota = nota;
        }
    }
    return offs;

 as_break:
    anal_fatt = 1;

    eva.nota = o_nota;
    eva.pd = camp2dur( ii );
    eva.nd = camp2dur( jj );

    if ( eva.nd < 5 ) {
        eva.pd += eva.nd;
        eva.nd = 0;
    }

    if ( eva.nd == 0 && ! ev_list.number ) {
        eva.offs_inizio = offs - lnod - nn;
        goto sec_point;
    }

    if ( eva.nd == 0 ) {
        ev_list.begin [ ev_list.number - 1 ].pd += eva.pd;
        eva.offs_inizio = offs - lnod - nn;
        goto sec_point;
    }
    else {

        if ( eva.nd ) eva.durata_nota = jj;
        else eva.durata_nota = 0;
        ev_list.begin [ ev_list.number ] = eva;
        ev_list.number++;
        eva.offs_inizio = offs - lnod - nn;
            
        if ( ev_list.number >= ev_list.alloc ) {
            ev_list.alloc *= 2;
            ev_list.begin = (event_t *) xrealloc( ev_list.begin,
                				    ev_list.alloc*sizeof(event_t) );
        }

        /*        k = (offs-WHEAD)/lun_sam-(ii+jj); */
        /*        printf( "%4.2f %i %i %i offs:%li dur:%li\n", \ */
        /*  	      k/(double)(filedur*freq), \ */
        /*  	      eva.nota, eva.nd, eva.pd, k, ii+jj ); */
    }
 sec_point:
    jj = 0; ii = 0;
    pausa = 0;
    trovata = 0;

    if ( fr > (double)0 )
        return offs;
    else {
        write_midi();
        return -1;
    }

} 
