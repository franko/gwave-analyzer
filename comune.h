#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI_2 6.28318530717958647692529
#define LN2 0.69314718055994530941723212146
#define SQR(x) (x)*(x)
#define LOG2D(x) log(x)/(double)LN2

// Ampiezza massima al di sotto della quale il suono � considerato nullo
#define MAXAMPTOL 0.05

typedef struct {
    long int offs_inizio;
    int durata_nota;
    int nota; 
    int pd;
    int nd;
} event_t;

typedef struct
{
    int offset;
    int vis_len;
    double y_magn;
}
wav_visual_t;

typedef struct
{
    event_t *begin;
    int number;
    int alloc;
}
event_list_t;

/*  extern long int dur, offs, l_filedur; */
/*  extern int freq, lunw, lun_sam; */
/*  extern int stereo; */
/*  extern int canal_method; */
extern wav_visual_t wav_vis;
extern event_list_t ev_list;

extern char *note_lett[];
extern double corr_arm;

extern double *cv, *fp, *lp;
extern int cvall, fpall;
extern int imm_incomp;
extern double filedur;

static __inline__ long int
ceiltopow( long int n )
{
    register long int out = 1;
    while ( out < n )
        out = out << 1;
    return out;
}

static __inline__ int
ilog2( long int n )
{
    register int j = 0;
    while ( n > 1 ) {
        n = n >> 1;
        j++;
    }
    return j;
}
 
static __inline__ int
pow2( int n )
{
    int i=1;
    return i << n;
}

static __inline__ void
*xmalloc( size_t n )
{
    register void *ret = (void *) malloc( n );
    if ( ! ret ) {
        fprintf( stderr, "Errore nell'allocare memoria\n" );
        exit(1);
    }
    return ret;
}

static __inline__ void
*xrealloc( void *pt, size_t n )
{
    register void *ret = (void *) realloc( pt, n );
    if ( ! ret ) {
        fprintf( stderr, "Errore nell'allocare memoria\n" );
        exit(1);
    }
    return ret;
}
         
extern void
nome_nota( int nota, char *str );

extern void
in_alloca();
