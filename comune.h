#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI_2 6.28318530717958647692529
#define LN2 0.69314718055994530941723212146
#define SQR(x) (x)*(x)
#define LOG2D(x) log(x)/(double)LN2

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
