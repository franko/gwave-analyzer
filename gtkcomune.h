#include <gtk/gtk.h>

/* Backing pixmap for drawing area */
extern GdkPixmap *pixmap;
extern GdkPixmap *four_pixmap;

extern GtkWidget *four_window;
extern GtkWidget *window;

extern gint plot_started, fplot_started;

extern int handle, fhandle;

extern gint Fourier(GtkWidget *widget, GdkEventExpose *event);

extern GdkGC *linea_gc;
