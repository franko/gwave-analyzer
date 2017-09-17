#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "comune.h"
#include "gtkcomune.h"

#define STR_LEN 160

#define NOPZ 3
#define NOPZS 1

/* una macro da paura ;-) */
#define DRAW_LINE(p,gc,x1,y1,x2,y2) gdk_draw_line(p,gc,drawing_area->allocation.width*(x1),drawing_area->allocation.height*( (-(y1)+1.2)/2.2 ),drawing_area->allocation.width*(x2),drawing_area->allocation.height*( (-(y2)+1.2)/2.2 ))

extern long int wtm_true_timeout( );

void draw( long int noc, GdkDrawable *loc_draw );
void wave_pixm_agg( );

typedef struct {
  GtkWidget *window;
  GtkWidget *pbar;
  int timer;
} ProgressData;

#include "icona.xpm"

static char *wav_head_1 = "RIFF", *wav_head_2 = "WAVEfmt ";

static char *opz[NOPZ] = { "--offs", "-c", "--corr-arm" };
static char *opzs[NOPZS] = { "--dummy" };

char *note_lett[] = { "Do", "Re", "Mi", "Fa", "Sol", "La", "Si" }; 
long int j;
int car[4], imm_incomp;
char str[100];
char fist_time_wtm;
char status_msg[ STR_LEN + 1 ] = "";

GtkWidget *drawing_area;
GtkWidget *status_label;
GtkObject *offs_adj, *wscala_adj;
GdkGC *linea_gc = NULL;

GtkWidget *window;
GdkPixmap *pixmap = NULL;
GdkFont *fixed_font;
char *stripped_filename;
char filename[ STR_LEN + 1 ] = "";

void
strip_filename()
{
  char *pt = strchr( filename, 0 );
  while ( pt > filename && *pt != '/' )
    pt--;
  if ( *pt == '/' ) pt++;
  stripped_filename = pt;
}

void det_file_durata() {
  if ( inpf )
    {
      fseek( inpf, 0, SEEK_END );
      wav_spec.num_sample = ( ftell( inpf )-WHEAD )/wav_spec.lun_sample;

      if ( wav_spec.num_sample < 0 )
	wav_spec.num_sample = 0;

      imm_incomp = ( wav_spec.num_sample < wav_vis.vis_len );
    }
}

void
analyze_wav_header( void )
{
  int wavefile, len;
  char head[WHEAD+2];
  if ( ! inpf )
    {
      strcpy( status_msg, "No file selected." );
      return;
    }

  wavefile = ( fseek( inpf, 0, SEEK_SET ) == 0 );

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

  snprintf( status_msg, STR_LEN,
"File %s, Sample Freq. %i, Channel num. %i, byte/sample %i, Sample number %i",
	    stripped_filename, wav_spec.freq, wav_spec.num_can,
	    wav_spec.lun_word, wav_spec.num_sample );

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

void wave_pixm_agg( ) {
  int i, j;
  long int xinf, xsup, xx, xp;
  char c1, c2;

  gdk_draw_rectangle (pixmap,
		      window->style->white_gc,
		      TRUE,
		      0, 0,
		      drawing_area->allocation.width,
		      drawing_area->allocation.height);
  if ( ! inpf )
    return;
  draw( wav_vis.vis_len, pixmap );
  
  for ( i=0; i < ev_list.number; i++ ) {
    xinf = wav_vis.offset;
    xsup = xinf + wav_vis.vis_len;
    xx = ev_list.begin[i].offs_inizio;
    xp = ev_list.begin[i].offs_inizio + ev_list.begin[i].durata_nota;
    c1 = ( xx != xp && xx >= xinf && xx <= xsup );
    c2 = ( xp >= xinf && xp <= xsup );
    if ( c1 ) {
      DRAW_LINE( pixmap, window->style->black_gc,
		 (xx-xinf)/(double)wav_vis.vis_len, 1.2,
		 (xx-xinf)/(double)wav_vis.vis_len, 1.0 );
      nome_nota( ev_list.begin[i].nota, str );
      j = strlen( str );
      sprintf( &str[j], " - %i", ev_list.begin[i].nota );
      gdk_draw_text( pixmap,
		     fixed_font,
		     window->style->black_gc,
		     drawing_area->allocation.width*((xx-xinf)/ \
						     (double)wav_vis.vis_len),
		     20, str, strlen( str ) );
    }
    if ( ( xx >= xinf || xp >= xinf ) && ( xx <= xsup || xp <= xsup ) )
      DRAW_LINE( pixmap, window->style->black_gc,
		 (xx-xinf)/(double)wav_vis.vis_len, 1.0,
		 (xp-xinf)/(double)wav_vis.vis_len, 1.0 );

    if ( c2 )
      DRAW_LINE( pixmap, window->style->black_gc,
		     (xp-xinf)/(double)wav_vis.vis_len, 1.2,
		     (xp-xinf)/(double)wav_vis.vis_len, 1.0 );

  }

  gdk_draw_pixmap(drawing_area->window, 
		  drawing_area->style->fg_gc[GTK_WIDGET_STATE (drawing_area)], 
		  pixmap,
		  0, 0, 
		  0, 0, 
		  drawing_area->allocation.width, 
		  drawing_area->allocation.height);
}


/* Create a new backing pixmap of the appropriate size */
static gint
configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
  if (pixmap)
    gdk_pixmap_unref(pixmap);

  pixmap = gdk_pixmap_new(widget->window,
			  widget->allocation.width,
			  widget->allocation.height,
			  -1);

  wave_pixm_agg();
  
  return TRUE;
}

/* Redraw the screen from the backing pixmap */
static gint
expose_event (GtkWidget *widget, GdkEventExpose *event)
{
  gdk_draw_pixmap(widget->window,
		  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		  pixmap,
		  event->area.x, event->area.y,
		  event->area.x, event->area.y,
		  event->area.width, event->area.height);

  return FALSE;
}

static gint
cb_offs_change( GtkAdjustment *l_offs_adj )
{
  wav_vis.offset = (int) l_offs_adj->value;

  if ( wav_vis.offset > wav_spec.num_sample - wav_vis.vis_len )
    {
      wav_vis.offset = wav_spec.num_sample - wav_vis.vis_len;
      l_offs_adj->value = wav_vis.offset;
    }

  if ( ( imm_incomp = (  wav_vis.offset < 0 ) ) )
    wav_vis.offset = 0;

  wave_pixm_agg();
  return (TRUE);
}

static gint
my_key_press_event (GtkWidget *widget, GdkEventKey *event)
{
  int agg = 0, magn = 0;
  switch ( event->keyval )
    {
    case GDK_i:
      agg = -ceiltopow( wav_vis.vis_len );
      break;
    case GDK_a:
      agg = ceiltopow( wav_vis.vis_len );
      break;
    case GDK_plus:
      magn = 1;
      wav_vis.y_magn *= 1.5;
      break;
    case GDK_minus:
      magn = 1;
      wav_vis.y_magn /= 1.5;
    }

  if ( ( ( agg && wav_vis.offset+agg > 0 ) || magn ) && pixmap != NULL )
    {
      wav_vis.offset += agg;

      if ( wav_vis.vis_len >= wav_spec.num_sample )
	wav_vis.offset = wav_spec.num_sample -  wav_vis.vis_len;

      if ( ( imm_incomp = ( wav_vis.offset < 0 ) ) )
	wav_vis.offset = 0;

      gtk_adjustment_set_value( GTK_ADJUSTMENT(offs_adj), wav_vis.offset );
      wave_pixm_agg();
    }
  
  return TRUE;
}

void
quit ()
{
  gtk_exit (0);
}

void
draw( long int noc, GdkDrawable *loc_draw )
{
  double dx, y, old_y, x = 0;
  long int j = 0;

  if ( ! linea_gc )
    {
      linea_gc = gdk_gc_new( window->window );
      gdk_rgb_gc_set_foreground( linea_gc, 12983067 );
    }

  if ( noc > wav_spec.num_sample - wav_vis.offset )
    {
      noc = wav_spec.num_sample - wav_vis.offset;
      if ( noc <= 0 )
	return;
    }

  dx = 1/(double)noc;
  x = 0;
  if ( fseek( inpf, TO_BYTE( wav_vis.offset ), SEEK_SET ) ) exit(0);
  getone( &old_y );
  old_y *= wav_vis.y_magn;
  do {
    if ( getone( &y ) )
      break;
    y *= wav_vis.y_magn;
    x += dx;
    DRAW_LINE( loc_draw, linea_gc, x-dx, old_y, x, y );
    old_y = y;
    j++;
  } while ( j < noc );
}

static gint 
cb_scale_change( GtkAdjustment *adj, GtkAdjustment *correggi_adj ) {
  long int td;
  if ( ( td = pow2( adj->value + 8 ) ) )
    {
      wav_vis.vis_len = td;
      if ( wav_vis.offset + wav_vis.vis_len > wav_spec.num_sample )
	  wav_vis.offset = wav_spec.num_sample - wav_vis.vis_len;

      if ( ( imm_incomp = ( wav_vis.offset < 0 ) ) )
	wav_vis.offset = 0;

    correggi_adj->step_increment = wav_vis.vis_len/16;
    correggi_adj->page_increment = wav_vis.vis_len;
    correggi_adj->page_size = wav_vis.vis_len;
    gtk_signal_emit_by_name (GTK_OBJECT (correggi_adj), "changed");
    wave_pixm_agg();
  }
  return TRUE;
}

void destroy_progress( GtkWidget     *widget,
		       ProgressData *pdata)
{
  gtk_widget_destroy( widget );
  gtk_timeout_remove (pdata->timer);
  pdata->timer = 0;
  g_free(pdata);
  wave_pixm_agg();
}

static gint
wtm_timeout( gpointer pdata ) {
  long int ris;
  if ( ( ris = wtm_true_timeout( fist_time_wtm ) ) < 0 )
    gtk_widget_destroy( ((ProgressData *) pdata)->window );
  else
    {
      fist_time_wtm = 0;
      gtk_progress_set_value (GTK_PROGRESS (((ProgressData *) pdata)->pbar),
			      100*(ris/(double)wav_spec.num_sample) );
    }
  return (TRUE);
}

static void
cb_wavtomidi(GtkWidget *w, gpointer data) {
  ProgressData *pdata;
  GtkWidget *xwid, *vbox;
  GtkAdjustment *prog_adj;

  pdata = (ProgressData *) g_malloc( sizeof(ProgressData) );

  pdata->window = gtk_window_new( GTK_WINDOW_DIALOG );

  gtk_signal_connect( GTK_OBJECT(pdata->window), "destroy",
		      GTK_SIGNAL_FUNC( destroy_progress ), pdata );

  gtk_container_border_width( GTK_CONTAINER(pdata->window), 6 );
  vbox = gtk_vbox_new( FALSE, 0 );
  gtk_container_add( GTK_CONTAINER(pdata->window), vbox );
  gtk_widget_show( vbox );

  prog_adj = (GtkAdjustment *) gtk_adjustment_new( 1.0, 0.0, 101.0, 1.0,
						   1.0, 1.0 );
  pdata->pbar = gtk_progress_bar_new_with_adjustment ( prog_adj );
  gtk_box_pack_start( GTK_BOX(vbox), pdata->pbar, FALSE, FALSE, 0 );
  gtk_widget_show( pdata->pbar );

  xwid = gtk_button_new_with_label( "Stop" );
  gtk_box_pack_start( GTK_BOX(vbox), xwid, FALSE, FALSE, 0 );
  gtk_widget_show( xwid );
  gtk_signal_connect_object( GTK_OBJECT(xwid), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy, 
			     GTK_OBJECT(pdata->window) );

  pdata->timer = gtk_timeout_add (10, wtm_timeout, pdata);

  gtk_widget_show( pdata->window );

  gtk_grab_add( pdata->window );

  fist_time_wtm = 1;

  return;
}

static gint
file_ok_sel( GtkWidget *ok_button, GtkWidget *filew ) {
  FILE *tinpf;
  char *nomef = gtk_file_selection_get_filename( GTK_FILE_SELECTION (filew) );

  tinpf = fopen( nomef, "r" );

  if ( tinpf )
    {
      inpf = tinpf;

      strncpy( filename, nomef, STR_LEN+1 );

      ev_list.number = 0;

      analyze_wav_header();

      gtk_label_set_text( GTK_LABEL( status_label ), status_msg );

      wav_vis.offset = 0;

      GTK_ADJUSTMENT(offs_adj)->value = 0.0;
      GTK_ADJUSTMENT(offs_adj)->lower = 0.0;
      GTK_ADJUSTMENT(offs_adj)->upper = wav_spec.num_sample;
      GTK_ADJUSTMENT(offs_adj)->step_increment = wav_vis.vis_len/16;
      GTK_ADJUSTMENT(offs_adj)->page_increment = wav_vis.vis_len;
      GTK_ADJUSTMENT(offs_adj)->page_size = wav_vis.vis_len;
      gtk_signal_emit_by_name (GTK_OBJECT (offs_adj), "changed");
      GTK_ADJUSTMENT(wscala_adj)->value = LOG2D( wav_vis.vis_len ) - 8;
      gtk_signal_emit_by_name (GTK_OBJECT (wscala_adj), "changed");

      wave_pixm_agg();
    }  
  gtk_widget_destroy( filew );
  return TRUE;
}

static void
cb_apri_file(GtkWidget *w, gpointer data) {
  GtkWidget *filew;

  /* Create a new file selection widget */
  filew = gtk_file_selection_new ("Seleziona un file");
          
  gtk_signal_connect (GTK_OBJECT (filew), "destroy",
		      (GtkSignalFunc) gtk_widget_destroy, GTK_OBJECT (filew) );
  /* Connect the ok_button to file_ok_sel function */
  gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
		      "clicked", GTK_SIGNAL_FUNC(file_ok_sel), filew );
          
  /* Connect the cancel_button to destroy the widget */
  gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
					 (filew)->cancel_button),
			     "clicked", (GtkSignalFunc) gtk_widget_destroy,
			     GTK_OBJECT (filew));
          
  /* Lets set the filename, as if this were a save dialog, and we are giving
     a default filename */
  gtk_widget_show(filew);
  gtk_grab_add( filew );

}

static void
cb_about(GtkWidget *w, gpointer data) {
#if 1
  g_message(
   "GWave-Analyzer\nAuthor: Francesco abbate\n\nIf you want help to develop\
 this program please\n\
 email to france.abbate@tiscalinet.it" );
#else
  g_message("\nQuesto programma non ha ancora un nome!\n\nIn ogni caso l'autore è Francesco Abbate");
#endif
}

static GtkItemFactoryEntry menu_items[] = {
  {"/_File",          NULL,         NULL, 0, "<Branch>"},
  {"/File/_Open",     "<control>O", cb_apri_file, 0, NULL},
  {"/File/sep1",      NULL,         NULL, 0, "<Separator>"},
  {"/File/Quit",      "<control>Q", gtk_main_quit, 0, NULL},
  {"/_Tools",      NULL,        NULL, 0, "<Branch>"},
  {"/Tools/Extract melody",   "<control>E", cb_wavtomidi, 0, NULL},
  {"/Tools/Playmidi", NULL, NULL, 0, NULL},
  {"/_Help",          NULL,        NULL,     0, "<LastBranch>"},
  {"/_Help/About",NULL,     cb_about, 0, NULL},
};

void get_main_menu(GtkWidget *window, GtkWidget ** menubar) {
  int nmenu_items = sizeof(menu_items) / sizeof(menu_items[0]);
  GtkItemFactory *item_factory;
  GtkAccelGroup *accel_group;

  accel_group = gtk_accel_group_new();

  /* This function initializes the item factory.
     Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
              or GTK_TYPE_OPTION_MENU.
     Param 2: The path of the menu.
     Param 3: A pointer to a gtk_accel_group.  The item factory sets up
              the accelerator table while generating menus.
  */

  item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", 
				       accel_group);

  /* This function generates the menu items. Pass the item factory,
     the number of items in the array, the array itself, and any
     callback data for the the menu items. */
  gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);

  /* Attach the new accelerator group to the window. */
  gtk_accel_group_attach (accel_group, GTK_OBJECT (window));

  if (menubar)
    /* Finally, return the actual menu bar created by the item factory. */ 
    *menubar = gtk_item_factory_get_widget(item_factory, "<main>");
}

static gint
butt_can_1(GtkWidget *widget, GdkEventExpose *event)
{
  if ( wav_vis.canal_method != 1 )
    {
      wav_vis.canal_method = 1;
      wave_pixm_agg();
    }

  return TRUE;
}

static gint
butt_can_2(GtkWidget *widget, GdkEventExpose *event)
{
  if ( wav_vis.canal_method != 2 )
    {
      wav_vis.canal_method = 2;
      wave_pixm_agg();
    }

  return TRUE;
}

static gint
butt_can_m(GtkWidget *widget, GdkEventExpose *event)
{
  if ( wav_vis.canal_method != 0 && wav_spec.num_can == 2 )
    {
      wav_vis.canal_method = 0;
      wave_pixm_agg();
    }

  return TRUE;
}

int
main (int argc, char *argv[])
{
  GtkWidget *vbox, *toolbarbox, *four_button, *wave_scrollbar;
  GtkWidget *button, *xwid, *frame;
  GtkStyle *stile_w;
  GdkBitmap *mask;
  GdkPixmap *pr_icona;
  int ce, i, j, ce_file = 0;

  gtk_init (&argc, &argv);

  gdk_rgb_init();

  in_alloca();

  corr_arm = 0;

  if ( argc > 1 )
    for ( i=1; i < argc; i++ ) {
      for ( j=0; j < NOPZ; j++ )
	if ( ( ce = ! strcmp( opz[j], argv[i] ) ) ) break;
      if ( ce ) {
	switch( j ) {
	case 0:
	  wav_vis.offset = atoi( argv[i+1] );
	  break;
	case 1:
	  wav_vis.canal_method = atoi( argv[i+1] );
	  break;
	case 2:
	  corr_arm = strtod( argv[i+1], NULL );
	  printf( "Corr. arm :%f\n", corr_arm );
	}
	i++;
      }
      else {
	for ( j=0; j < NOPZS; j++ )
	  if ( ( ce = ! strcmp( opzs[j], argv[i] ) ) ) break;
	if ( ce ) 
	  switch( j ) {
	  case 0:
	    puts( "Dummy! \n" );

	  }
	else {
	  strncpy( filename, argv[i], STR_LEN+1 );
	  ce_file = 1;
	}
      }
    }

  if ( ce_file )
      inpf = fopen( filename, "r" );
  else
    inpf = (FILE *) NULL;

  analyze_wav_header();

  fixed_font = gdk_font_load ("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*");

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name (window, "Analizzatore di suono");

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_show (vbox);

  get_main_menu( window, &xwid );
  gtk_box_pack_start( GTK_BOX(vbox), xwid, FALSE, FALSE, 0 );
  gtk_widget_show( xwid );

  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC (quit), NULL);

  toolbarbox = gtk_hbox_new( FALSE, 0 );
  gtk_container_border_width (GTK_CONTAINER (toolbarbox), 10);

  four_button = gtk_button_new_with_label( "Fourier" );
  gtk_box_pack_start (GTK_BOX(vbox), toolbarbox, FALSE, FALSE, 0);

  gtk_box_pack_start( GTK_BOX(toolbarbox), four_button, TRUE, TRUE, 0 );
  gtk_widget_show( four_button );

  if ( inpf )
    offs_adj = gtk_adjustment_new (0.0, 0.0, wav_spec.num_sample,
		wav_vis.vis_len/16, wav_vis.vis_len, wav_vis.vis_len );
  else
    offs_adj = gtk_adjustment_new (0.0, 0.0, 1.0, 0.1, 1.0, 0.1); 

  xwid = gtk_spin_button_new( GTK_ADJUSTMENT(offs_adj), 1.0, 0 );
  gtk_box_pack_start( GTK_BOX( toolbarbox ), xwid, TRUE, TRUE, 0 );
  gtk_widget_show( xwid );

  frame = gtk_frame_new( "Zoom" );
  gtk_box_pack_start( GTK_BOX( toolbarbox ), frame, TRUE, TRUE, 0 );
  gtk_widget_show( frame );

  wscala_adj = gtk_adjustment_new ( 3.0, 0.0, 13.0, 1.0, 1.0, 1.0 );
  xwid = gtk_hscale_new( GTK_ADJUSTMENT( wscala_adj ) );
  gtk_scale_set_digits( GTK_SCALE( xwid ), 0 );
  gtk_container_add( GTK_CONTAINER( frame ), xwid );
  gtk_widget_show( xwid );

  xwid = gtk_vbox_new( FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(toolbarbox), xwid, FALSE, FALSE, 0 );

  button = gtk_radio_button_new_with_label (NULL, "Channel 1");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_box_pack_start (GTK_BOX (xwid), button, TRUE, TRUE, 0);
  gtk_widget_show( button );

  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (butt_can_1),
			     GTK_OBJECT (window));

  button = gtk_radio_button_new_with_label (
	     gtk_radio_button_group (GTK_RADIO_BUTTON (button)),
		 "Channel 2");
  gtk_box_pack_start (GTK_BOX (xwid), button, TRUE, TRUE, 0);
  gtk_widget_show( button );

  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (butt_can_2),
			     GTK_OBJECT (window));

  button = gtk_radio_button_new_with_label (
              gtk_radio_button_group (GTK_RADIO_BUTTON (button)),
	      "Average");
  gtk_box_pack_start (GTK_BOX (xwid), button, TRUE, TRUE, 0);
  gtk_widget_show( button );
  
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     GTK_SIGNAL_FUNC (butt_can_m),
			     GTK_OBJECT (window));

  gtk_widget_show( xwid );

  gtk_widget_show( toolbarbox );

  /* Create the drawing area */

  drawing_area = gtk_drawing_area_new ();
  gtk_drawing_area_size (GTK_DRAWING_AREA (drawing_area), 620, 280);

  gtk_box_pack_start (GTK_BOX (vbox), drawing_area, TRUE, TRUE, 0);

  gtk_widget_show (drawing_area);

  wave_scrollbar = gtk_hscrollbar_new( GTK_ADJUSTMENT(offs_adj) );

  gtk_widget_show( wave_scrollbar );

  gtk_box_pack_start(GTK_BOX (vbox), wave_scrollbar , FALSE, FALSE, 0);

  gtk_signal_connect( GTK_OBJECT(wscala_adj), "value_changed",
		      GTK_SIGNAL_FUNC(cb_scale_change), offs_adj );

  gtk_signal_connect( GTK_OBJECT(offs_adj), "value_changed",
		      GTK_SIGNAL_FUNC(cb_offs_change), NULL );

  /* Signals used to handle backing pixmap */

  gtk_signal_connect (GTK_OBJECT (drawing_area), "expose_event",
		      (GtkSignalFunc) expose_event, NULL);
  gtk_signal_connect (GTK_OBJECT(drawing_area),"configure_event",
		      (GtkSignalFunc) configure_event, NULL);

  /* Event signals */

/*    gtk_signal_connect (GTK_OBJECT (drawing_area), "button_press_event", */
/*  		      (GtkSignalFunc) button_press_event, NULL); */

  gtk_signal_connect (GTK_OBJECT (window), "key_press_event",
		      (GtkSignalFunc) my_key_press_event, NULL);

  gtk_widget_set_events ( window, GDK_EXPOSURE_MASK
    			 | GDK_LEAVE_NOTIFY_MASK  
    			 | GDK_KEY_PRESS_MASK 
  			 );

/*    GTK_WIDGET_SET_FLAGS (drawing_area, GTK_CAN_FOCUS); */
/*    gtk_widget_grab_focus (drawing_area); */

  status_label = gtk_label_new( status_msg );
  gtk_box_pack_start (GTK_BOX (vbox), status_label, FALSE, FALSE, 0);
  gtk_widget_show( status_label );

  gtk_signal_connect_object (GTK_OBJECT (four_button), "clicked",
			     GTK_SIGNAL_FUNC (Fourier),
			     GTK_OBJECT (window));

  gtk_widget_show (window);

  stile_w = gtk_widget_get_style( window );
  pr_icona = \
    gdk_pixmap_create_from_xpm_d( window->window,
				  &mask,
				  &stile_w->bg[GTK_STATE_NORMAL],
				  (gchar **) wm_hint_xpm );

  gdk_window_set_icon( window->window, NULL, pr_icona, mask );
			       
  gtk_main ();

  return 0;
}
