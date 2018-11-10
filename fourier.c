#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*  #include <gdk/gdkprivate.h> */
#include "comune.h"
#include "gtkcomune.h"

#define DRAW_LINE(p,gc,x1,y1,x2,y2) gdk_draw_line(p,gc,fdrawing_area->allocation.width*(x1),fdrawing_area->allocation.height*( (-(y1)+1.2)/2.2 ),fdrawing_area->allocation.width*(x2),fdrawing_area->allocation.height*( (-(y2)+1.2)/2.2 ))

GtkWidget *four_window;
GdkPixmap *four_pixmap = NULL;
/*  gint fplot_started = 0; */
/*  int fhandle; */

long int four_nod, visib_nod;
int nm;
char str[100], puro, nullo;
double vf, intens, fr, chiq;

GtkWidget *fdrawing_area;
GtkObject *f_offs_adj;
GtkObject *scala_adj;
static GdkGC *linea_pic = NULL;

void four_diag_traccia( GdkDrawable *loc_draw )
{
    unsigned long int i, nod = four_nod/2;
    double x = 0, ymax, ymin, dx;
    int jj, i_offs;

    if ( ! linea_pic )
        {
            linea_pic = gdk_gc_new( four_window->window );
            gdk_gc_set_line_attributes( linea_pic, 1,
                			  GDK_LINE_ON_OFF_DASH, 
                			  GDK_CAP_NOT_LAST,
                			  GDK_JOIN_MITER );
            gdk_rgb_gc_set_foreground( linea_pic, 0 );
        }

    i_offs = GTK_ADJUSTMENT(f_offs_adj)->value;
    dx = pow2( GTK_ADJUSTMENT(scala_adj)->value )/(double)nod;

    gdk_draw_rectangle( loc_draw, four_window->style->white_gc,
                	      TRUE,
                	      0, 0,
                	      four_window->allocation.width,
                	      four_window->allocation.height );

    ymax = ( ymin = cv[0] );
    x = 0;
    for ( i=1; i<nod; i++ ) {
        if ( cv[i] > ymax ) ymax = cv[i];
        if ( cv[i] < ymin ) ymin = cv[i];
    }
    for ( i=1; i<nod; i++ )
        cv[i] = ( cv[i] - ymin )/(ymax - ymin);

    jj = 0;
    for ( i=i_offs; i<nod; i++ ) {
        DRAW_LINE( loc_draw, linea_gc, x, -1, x, 2*cv[i] - 1 );
        x += dx;
        jj ++;
        if ( jj >= visib_nod ) break;
    }

    for ( i=0; i<nm; i++ )
        DRAW_LINE( loc_draw, linea_pic,
                             dx*(nmax[i]-i_offs), 2*cv[nmax[i]]-1,
                             dx*(nmax[i]-i_offs), 1.0 );

    if ( nm == 0 ) nmax[0] = 0;
}

void analisi_spettro( int loc_offs ) {
    unsigned int nnod, nod, i;
    int rif;
    double fatt, rejfac = 0.003;
    char iflag;
    double soglia, vmax, vmin, x;

    nod = ceiltopow( wav_vis.vis_len );
    rif = ilog2( nod );

    if ( fseek( inpf, wav_vis.offset, SEEK_SET ) ) exit(1);
    getone( &vmin );
    vmax = vmin;
    for ( i=1; i<nod; i++ ) {
        if ( getone( &x ) )
            break;
        if ( x > vmax ) vmax = x;
        if ( x < vmin ) vmin = x;
    }

    nullo = ( ( vmax - vmin ) < MAXAMPTOL );

    for ( iflag=0; iflag<2; iflag++ ) {

        fourier( nod, wav_vis.offset );
        
        if ( iflag == 0 ) nnod = nod;
        else {
            nnod = ceiltopow( nod );
            rif = ilog2( nnod );
        }

        fatt = 1/(double) nnod;
        for ( i=1; i < nnod/2; i++ ) { // parte da 1 per escludere la continua
            cv[ i ] = fatt*( cv[ 2*i ]*cv[ 2*i ] + cv[ 2*i+1 ]*cv[ 2*i+1 ] );
            soglia += cv[ i ];
        }
        cv[0] = 0; // comp. continua
        soglia *= rejfac;

        nm = trova_pic( &puro, nnod, soglia, &intens );

        if ( iflag == 0 )
            chiq = ver_armonia( nmax, nm, &vf, 0.1 );
        else
            chiq = ver_armonia( nmax, nm, &vf, 0.0 ); // Punto critico

        if ( chiq < 0 ) fr = /*nmax[0]*((double)wav_spec.freq/nod)*/ 0;
        else fr = vf*((double)wav_spec.freq/nod); // Punto critico

        if ( iflag == 0 ) nod -= elim_spurii( fr, nod, loc_offs );

        if ( nod < 8 ) break;
    }

    four_nod = nnod;

}
         
void four_agg_pixmap() {
    gdk_draw_pixmap(fdrawing_area->window,
                	  fdrawing_area->style->fg_gc[GTK_WIDGET_STATE (fdrawing_area)],
                	  four_pixmap,
                	  0, 0,
                	  0, 0,
                	  fdrawing_area->allocation.width,
                	  fdrawing_area->allocation.height);
}

static gint
fconfigure_event (GtkWidget *widget, GdkEventConfigure *event)
{
    if ( four_pixmap )
        gdk_pixmap_unref(four_pixmap);

    four_pixmap = gdk_pixmap_new(widget->window,
                		       widget->allocation.width,
                		       widget->allocation.height,
                		       -1);

    four_diag_traccia( four_pixmap );
    four_agg_pixmap();
    
    return TRUE;
}

static gint fexpose_event( GtkWidget *widget, GdkEventExpose *event ) {
    gdk_draw_pixmap(widget->window,
                	  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                	  four_pixmap,
                	  event->area.x, event->area.y,
                	  event->area.x, event->area.y,
                	  event->area.width, event->area.height);

    return FALSE;
}

static gint
cb_close_fw( GtkWidget *widget, GdkEventAny *event ) {
    gtk_grab_remove(four_window);
    gdk_pixmap_unref( four_pixmap );
    gtk_widget_destroy( four_window );
    four_pixmap = NULL;
    return TRUE;
}

static gint
cb_four_scala_change( GtkAdjustment *adj ) {
    visib_nod = four_nod/(2*(double)pow2( GTK_ADJUSTMENT(adj)->value ));
    GTK_ADJUSTMENT(f_offs_adj)->page_size = visib_nod;
    if ( GTK_ADJUSTMENT(f_offs_adj)->value > four_nod/2 - visib_nod )
        GTK_ADJUSTMENT(f_offs_adj)->value = four_nod/2 - visib_nod;
    gtk_signal_emit_by_name( f_offs_adj, "changed" );

    four_diag_traccia( four_pixmap );
    four_agg_pixmap();
    
    return TRUE;
}

static gint
cb_foffs_change( GtkAdjustment *adj, GtkWidget *AreaDisegn ) {
    four_diag_traccia( four_pixmap );
    four_agg_pixmap();
    
    return TRUE;
}

gint Fourier(GtkWidget *widget, GdkEventExpose *event) {
    GtkWidget *fframe, *fvbox, *label, *scala_sc, *pannello, *xwid;

    if ( imm_incomp || ! inpf ) return TRUE;

    analisi_spettro( wav_vis.offset );

    four_window = gtk_window_new( GTK_WINDOW_DIALOG );
    gtk_widget_set_name( four_window, "Fourier Analysis" );
    gtk_container_border_width( GTK_CONTAINER(four_window), 10 );
    
    fvbox = gtk_vbox_new( FALSE, 0 );
    gtk_container_add( GTK_CONTAINER(four_window), fvbox );
    gtk_widget_show( fvbox );

    pannello = gtk_hbox_new( FALSE, 5 );
    gtk_box_pack_start( GTK_BOX(fvbox), pannello, FALSE, FALSE, 0 );
    gtk_widget_show( pannello );

    fframe = gtk_frame_new( "Zoom" );
    gtk_box_pack_start( GTK_BOX( pannello ), fframe, FALSE, FALSE, 0 );
    gtk_widget_show( fframe );

    scala_adj = gtk_adjustment_new ( 0.0, 0.0, 9.0, 1.0, 1.0, 1.0 );
    scala_sc = gtk_hscale_new( GTK_ADJUSTMENT( scala_adj ) );
    gtk_scale_set_digits( GTK_SCALE( scala_sc ), 0 );
    gtk_container_add( GTK_CONTAINER( fframe ), scala_sc );
    gtk_widget_show( scala_sc );

    fframe = gtk_frame_new( "Parameter" );
    gtk_box_pack_start( GTK_BOX( pannello ), fframe, TRUE, TRUE, 0  );
    gtk_widget_show( fframe );

    xwid = gtk_table_new( 2, 4, FALSE );
    gtk_container_add( GTK_CONTAINER( fframe ), xwid );
    gtk_widget_show( xwid );

    sprintf( str, "Freq.:%4.0f Hz", fr );
    label = gtk_label_new( str );
    gtk_table_attach_defaults( GTK_TABLE(xwid), label, 0, 2, 0, 1 );
    gtk_widget_show( label );

    sprintf( str, "Chisq.:%6.2f", chiq );
    label = gtk_label_new( str );
    gtk_table_attach_defaults( GTK_TABLE(xwid), label, 2, 4, 0, 1 );
    gtk_widget_show( label );

    if ( puro )
        strcpy( str, "Clean" );
    else
        strcpy( str, "Dirty" );
    label = gtk_label_new( str );
    gtk_table_attach_defaults( GTK_TABLE(xwid), label, 0, 1, 1, 2 );
    gtk_widget_show( label );

    if ( nullo )
        strcpy( str, "Null" );
    else
        strcpy( str, "Present" );
    label = gtk_label_new( str );
    gtk_table_attach_defaults( GTK_TABLE(xwid), label, 1, 2, 1, 2 );
    gtk_widget_show( label );

    sprintf( str, "Int:%6.4f", intens/four_nod );
    label = gtk_label_new( str );
    gtk_table_attach_defaults( GTK_TABLE(xwid), label, 2, 3, 1, 2 );
    gtk_widget_show( label );

#if 0
    sprintf( str, "Fundam.:%f Hz", vf*((double)wav_spec.freq/nod) );
    label = gtk_label_new( str );
    gtk_table_attach_defaults( GTK_TABLE(xwid), label, 3, 4, 1, 2 );
    gtk_widget_show( label );
#endif

    gtk_widget_show( pannello );

    gtk_signal_connect( scala_adj, "value_changed", 
    		      GTK_SIGNAL_FUNC( cb_four_scala_change ), NULL );

    fdrawing_area = gtk_drawing_area_new ();
    gtk_drawing_area_size (GTK_DRAWING_AREA (fdrawing_area), 300, 200);

    gtk_box_pack_start (GTK_BOX (fvbox), fdrawing_area, TRUE, TRUE, 0);

    gtk_widget_show (fdrawing_area);

    visib_nod = four_nod/2;
    f_offs_adj = gtk_adjustment_new ( 0.0, 0.0, four_nod/2, 1.0, 20.0, visib_nod );
    xwid = gtk_hscrollbar_new( GTK_ADJUSTMENT(f_offs_adj) );
    gtk_box_pack_start( GTK_BOX(fvbox), xwid, FALSE, FALSE, 0 );
    gtk_widget_show( xwid );

    gtk_signal_connect( GTK_OBJECT(f_offs_adj), "value_changed",
                	      GTK_SIGNAL_FUNC( cb_foffs_change ), fdrawing_area );

    gtk_signal_connect (GTK_OBJECT (fdrawing_area), "expose_event",
                	      (GtkSignalFunc) fexpose_event, NULL);
    gtk_signal_connect (GTK_OBJECT(fdrawing_area),"configure_event",
                	      (GtkSignalFunc) fconfigure_event, NULL);

    gtk_widget_set_events ( fdrawing_area, GDK_EXPOSURE_MASK
        			 | GDK_LEAVE_NOTIFY_MASK );

    gtk_signal_connect ( GTK_OBJECT(four_window), "delete_event",
                	       GTK_SIGNAL_FUNC(cb_close_fw), NULL );

    gtk_widget_show( four_window );

    gtk_grab_add( four_window );

    return TRUE;
}
