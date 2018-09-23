#include <alloca.h>

#include "fourier.h"

int fourier(wav_reader_t *wav, long int noc, int loc_offs) {
    unsigned int k, pd, i, j, m, nod;
    double wi, wr, wti, wtr, temi, temr;
    int rif;
    double ya, yb, ir, passo;
    int imin, iad;

    nod = ceiltopow( noc );
    rif = ilog2( nod );

    if ( nod > cvall ) {
        cvall = nod;
        cv = (double *) xrealloc( cv, 2*cvall*sizeof(double) );
    }

    if ( nod == noc ) {
        getdata(wav, cv, loc_offs, &noc, 1);
        if ( noc != nod ) return 1;
    }
    else {
        passo = noc/(double)nod;
        ir = 0;
        if (wav_reader_seek(wav, loc_offs)) exit(0);
        getone(wav, &ya);
        getone(wav, &yb);
        i = 0;
        iad = 1;
        do {
            imin = (int) ir;
            if ( imin >= iad ) {
                ya = yb;
                getone(wav, &yb);
                iad++;
            }
            cv[ 2*i+1 ] = 0;

            cv[ 2*i ] = ya*( imin+1-ir ) + yb*(ir-imin);
            i++;
            ir += passo;
        } while ( i < nod );
    }

    j= 1;
    m= nod/2;
    while ( j < nod ) {
        temr      = cv[2*j];
        temi      = cv[2*j+1];
        cv[2*j] = cv[2*m];
        cv[2*j+1] = cv[2*m+1];
        cv[2*m] = temr;
        cv[2*m+1] = temi;
        do {
            j++;
            i = pow2( rif - 1 );
            do {
                if ( m < i ) {
                    m += i;
                    break; }
                else {
                    m -= i;
                    i /= 2; }
            } while ( i );
        }
        while ( ( m<=j ) && ( j<nod ) );
    }
    pd=1;
    for ( i= 0; i < rif; i++ ) {
        pd = 2*pd;
        wtr= cosl( PI_2/ (double) pd );
        wti= sinl( PI_2/ (double) pd );
        for ( j= 0; j < 2*nod; j+=2*pd ) {
            wr= 1.; wi= 0.;
            for ( k= j; k < j+pd; k+=2 ) {
                temr = cv[ k+pd ];
                temi = cv[ k+pd+1 ];
                cv[ k+pd   ] = cv[ k   ] - wr*temr + wi*temi;
                cv[ k+pd+1 ] = cv[ k+1 ] - wi*temr - wr*temi;
                cv[ k   ]    = cv[ k   ] + wr*temr - wi*temi;
                cv[ k+1 ]    = cv[ k+1 ] + wi*temr + wr*temi;
                temr= wr;
                wr = wtr*wr - wi*wti;
                wi = wtr*wi + temr*wti;
            }
        }
    }
    return 0;
}

static unsigned int elim_spurii(wav_reader_t *wav, double frfond, long int nod, int loc_offs) {
    double mchiq, chiq, fmed, max, min, famp, med, amp;
    int i, j, betoffs = 0;
    long int ncpp;

    ncpp = (long int) ceil(wav->spec.freq / frfond);
    if ( ncpp <= 2 ) return nod;

    if ( ncpp > fpall ) {
        fpall = ncpp;
        fp = (double *) xrealloc( fp, fpall*sizeof(double) );
        lp = (double *) xrealloc( lp, 2*fpall*sizeof(double) );
    }

    getdata(wav, fp, loc_offs, &ncpp, 0);

    if ( 3*ncpp > nod  ) return nod;

    ncpp *= 2;
    getdata(wav, lp, loc_offs + ( nod - ncpp ), &ncpp, 0);
    ncpp /= 2;

    fmed = fp[0]; max = fmed; min = fmed;
    for ( i=1; i<ncpp; i++ ) {
        fmed += fp[i];
        if ( fp[i] > max ) max = fp[i];
        if ( fp[i] < min ) min = fp[i];
    }
    fmed /= (double) ncpp;
    famp = max - min;
    for ( i=0; i<ncpp; i++ ) {
        med = lp[ncpp-i]; max = med; min = med;
        for ( j= ncpp-i+1; j<2*ncpp-i; j++ ) {
            med += lp[j];
            if ( lp[j] > max ) max = lp[j];
            if ( lp[j] < min ) min = lp[j];
        }
        med /= (double) ncpp;
        amp = max - min;
        chiq = 0;
        for ( j=0; j<ncpp; j++ )
            {
                double ted;
                ted = fp[j] - ( lp[ ncpp - i + j ] - med )*famp/amp + fmed;
                chiq += SQR( ted );
            }
        chiq /= (double) ( ncpp - 2 );
        if ( i == 0 ) mchiq = chiq;
        else
            if ( mchiq > chiq ) {
                mchiq = chiq;
                betoffs = i;
            }
    }
#if 1
    return (unsigned int) ( ncpp + betoffs );
#else
    return (unsigned int) betoffs;  /* Punto critico */
#endif
}

static double ver_armonia(unsigned int *nmax, int nm, double *prob_fund, double fatt_ra) {
#if 0
    int nfrciu, limfrciu = 4, ibfnd = 0, i_pr_fnd = 0, j, i, nmpp;
    double bchiq = -1, chiq, max = 0, best_fund;
    unsigned int *nmaxpp;
    unsigned long int tsqr;
#else
    const int max_arm = 10;
    int j, i, nmpp;
    double bchiq = -1, chiq, max = 0, best_fund;
    unsigned int *nmaxpp;
#endif

    nmaxpp = (unsigned int *) alloca( nm*sizeof(int) );

    for ( i=0; i<nm; i++ )
        if ( cv[ nmax[i] ] > max ) max = cv[ nmax[i] ];

    nmaxpp[0] = 0;
    j = 1;
    for ( i=0; i<nm; i++ )
        if ( cv[ nmax[i] ] >= fatt_ra * max ) {
            nmaxpp[j] = nmax[i];
            j++;
        }
    nmpp = j;

    i = 0;
    j = 1;
    while ( 1 )
        {
            double pr_diff;
            int mul, k;

            pr_diff =  abs( nmaxpp[ i ] - nmaxpp[ j ] );

            if ( i == j )
                goto termina;

            chiq = 0;

            for ( k = 1; k < nmpp; k++ )
                {
                    mul = (int) rint( (double) nmaxpp[ k ] / pr_diff );
                    if ( mul >= max_arm )
                        {
                            chiq += 100;
                            break;
                        }
                    else
                        chiq += ( cv[ nmaxpp[ k ] ]/max ) * \
                            SQR( mul*pr_diff - (double) nmaxpp[k] ) / \
                            (double) ((nmpp-1)*(mul+1));
                }

            if ( bchiq < 0 || chiq < bchiq )
                {
                    bchiq = chiq;
                    best_fund = pr_diff;
                }

            if ( chiq >= 0 && chiq < 0.1 ) /* punto critico */
                break;

        termina:
            j++;

            if ( j >= nmpp )
                {
                    j = 0;
                    i++;
                    if ( i >= nmpp )
                        break;
                }
        }

    if ( bchiq >= 0 )
        *prob_fund = best_fund;

    return bchiq;

#if 0
    i_pr_fnd = 0;

    while ( i_pr_fnd < nmpp - 1 ) {

        chiq = 0;

        j = 2;

        for ( i=i_pr_fnd+1; i<nmpp; i++ ) {

            tsqr = SQR( j*nmaxpp[ i_pr_fnd ] - nmaxpp[i] );

            nfrciu = 0;

            while ( SQR( (j+1)*nmaxpp[ i_pr_fnd ] - nmaxpp[i] ) < tsqr ) {
                j++;
                nfrciu++;
                tsqr = SQR( j*nmaxpp[ i_pr_fnd ] - nmaxpp[i] );
            }

            if ( nfrciu > limfrciu ) {
                chiq += 100;
                break;
            }

            chiq += ( cv[nmaxpp[i]]/max )* \
                            (double) tsqr/(double)( j*(nmpp - i_pr_fnd - 1) );
        }

        if ( bchiq < 0 || chiq < bchiq ) {
            bchiq = chiq;
            ibfnd = i_pr_fnd; }

        i_pr_fnd++;
    }

    // E' un picco isolato ?
    if ( ( bchiq > 10 || bchiq < 0 ) \
             && nmpp <= 2 && cv[ nmaxpp[nmpp-1] ] == max ) {// punto critico
        ibfnd = nmpp-1;
        bchiq = 0.9; // assegnazione di un valore (< 1) convenzionale
    }

    if ( bchiq >= 0 ) {
        for ( i=0; i<nm; i++ )
            if ( nmax[i] == nmaxpp[ ibfnd ] ) break;

        *i_vera_fnd = i;
    }
    else *i_vera_fnd = -1;

/*    free( nmaxpp ); */

    return bchiq;
#endif
}

struct peak_ws {
    unsigned int *nmax;
    int size;
};

static void peak_ws_init(struct peak_ws *workspace, int size) {
    workspace->size = size;
    workspace->nmax = (unsigned int *) xmalloc(workspace->size * sizeof(int));
}

static void peak_ws_increase(struct peak_ws *workspace) {
    workspace->size *= 2;
    workspace->nmax = (unsigned int *) xrealloc(workspace->nmax, workspace->size * sizeof(int));
}

static void peak_ws_free(struct peak_ws *workspace) {
    free(workspace->nmax);
}

static int trova_pic(struct peak_ws *workspace, char *puro, unsigned int nod, double soglia, double *pintens) {
    int i = 0, j, nm = 0, cond, necc, nmaxecc, tolpur;
    double pmax;

    nmaxecc = (int) ( nod/(double)512 );

    tolpur = 4;
    if ( nod > 1024 ) tolpur = nod/512; // Punto critico

    *pintens = 0;
    *puro = 1;
    do {
        i++;
        if ( cv[i] < soglia ) continue;

        if (nm >= workspace->size) {
            peak_ws_increase(workspace);
        }

        unsigned int *nmax = workspace->nmax;
        j = 0;
        *pintens += cv[i];
        pmax = cv[ i ];
        nmax[ nm ] = i;
        necc = 0;
        do {
            j++;
            *pintens += cv[i+j];
            if ( cv[ i+j ] > pmax ) {
                pmax = cv[ i+j ];
                nmax[ nm ] = i+j; }
            if ( ( cond = ( cv[ i+j ] < soglia ) ) ) necc++;
            else necc = 0;
        } while ( necc <= nmaxecc );
        *puro = (*puro) && ( j - necc <= tolpur ); // Punto critico
        i += j-1;
        if ( cv[ nmax[nm] - 1 ] < pmax  && cv[ nmax[nm] + 1 ] < pmax ) nm++;
    } while ( i < nod/2 - 1 );
    *pintens *= 2; // Moltiplico per due perch� la procedura analizza mezzo spettro
    return nm;
}

double detfreq(wav_reader_t *wav, long int nod, char *puro, char *nullo, double *chiq, double *pintens, int loc_offs) {
    unsigned int i, nnod;
    double vmin, vmax, fatt, x, vf;
    double soglia = 0, rejfac = 0.003;
    int iflag, nm;
    long double fr = 0;

    if (wav_reader_seek(wav, loc_offs)) exit(1);

    getone(wav, &vmin);
    vmax = vmin;
    for ( i=1; i<nod; i++ ) {
        if (getone(wav, &x))
            {
                *nullo = 1;
                return (double)-1;
            }
        if ( x > vmax ) vmax = x;
        if ( x < vmin ) vmin = x;
    }

    *nullo = ( ( vmax - vmin ) < MAXAMPTOL ); /* Punto critico */
    if ( *nullo ) return 1;

    struct peak_ws workspace[1];
    // TODO: from the original version of gwave this shoud be allocated outside
    // of this function.
    peak_ws_init(workspace, 20);

    for ( iflag=0; iflag<2; iflag++ ) {

        fourier(wav, nod, loc_offs);

        if ( iflag == 0 )
            nnod = nod;
        else
            nnod = ceiltopow( nod );

        fatt = 1/(double) nnod;
        for ( i=1; i < nnod/2; i++ ) { // parte da 1 per escludere la continua
            cv[ i ] = fatt*( cv[ 2*i ]*cv[ 2*i ] + cv[ 2*i+1 ]*cv[ 2*i+1 ] );
            soglia += cv[ i ];
        }
        cv[0] = 0; // comp. continua
        soglia *= rejfac;

        nm = trova_pic(workspace, puro, nnod, soglia, pintens);
        unsigned int *nmax = workspace->nmax;

        if ( nm == 0 ) nmax[0] = 0;

        if ( iflag == 0 )
            *chiq = ver_armonia( nmax, nm, &vf, 0.1 );
        else
            *chiq = ver_armonia( nmax, nm, &vf, 0.0 ); // Punto critico

        if ( *chiq < 0 )
            fr = nmax[0]*((double)wav->spec.freq/nod);
        else
            fr = vf*((double)wav->spec.freq/nod); // Punto critico

        if ( iflag == 0 ) nod -= elim_spurii(wav, fr, nod, loc_offs);

        if ( nod < 8 ) break;
    }

    peak_ws_free(workspace);

    return fr;
}
