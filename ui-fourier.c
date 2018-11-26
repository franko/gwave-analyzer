#include <string.h>

#include "ui-fourier.h"
#include "fourier.h"
#include "wav2midi_priv.h"

static struct {
    double *fourier_coeffs;
    int fourier_size;
    double intensity_max;
} fourier_display;

static struct {
    uiSpinbox *offset_spinbox;
    uiSlider *zoom_slider;
    uiArea *area;
} controls;

static uiWindow *fourier_window = NULL;
static uiAreaHandler f_handler[1];

static void compute_fourier(wav_reader_t *wav, int sample_start, int sample_size) {
    wav_read_fourier(wav, sample_size, sample_start);
    free(fourier_display.fourier_coeffs);
    fourier_display.fourier_coeffs = malloc(sizeof(double) * 2 * sample_size);
    if (fourier_display.fourier_coeffs == NULL) {
        fourier_display.fourier_size = 0;
        return;
    }
    fourier_display.fourier_size = sample_size;
    for (int i = 0; i < 2 * sample_size; i++) {
        fourier_display.fourier_coeffs[i] = cv[i];
    }

    double intensity_max = 0.0;
    for (int k = 0; k < 2 * sample_size; k += 2) {
        const double fcr = fourier_display.fourier_coeffs[k], fci = fourier_display.fourier_coeffs[k + 1];
        const double intensity = fcr * fcr + fci * fci;
        if (intensity > intensity_max) {
            intensity_max = intensity;
        }
    }
    fourier_display.intensity_max = intensity_max;
}

#define colorWhite 0xFFFFFF
#define colorDodgerBlue 0x1E90FF

// helper to quickly set a brush color
static void setSolidBrush(uiDrawBrush *brush, uint32_t color, double alpha)
{
    uint8_t component;

    brush->Type = uiDrawBrushTypeSolid;
    component = (uint8_t) ((color >> 16) & 0xFF);
    brush->R = ((double) component) / 255;
    component = (uint8_t) ((color >> 8) & 0xFF);
    brush->G = ((double) component) / 255;
    component = (uint8_t) (color & 0xFF);
    brush->B = ((double) component) / 255;
    brush->A = alpha;
}

static void handlerFourierDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p) {
    uiDrawBrush brush;
    setSolidBrush(&brush, colorWhite, 1.0);

    uiDrawPath *path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawPathAddRectangle(path, 0, 0, p->AreaWidth, p->AreaHeight);
    uiDrawPathEnd(path);
    uiDrawFill(p->Context, path, &brush);
    uiDrawFreePath(path);

    if (fourier_display.fourier_size == 0) return;

    const int zoom = uiSliderValue(controls.zoom_slider);
    const int offset = uiSpinboxValue(controls.offset_spinbox);
    const int freq_size = fourier_display.fourier_size >> zoom;
    const int freq_start = (offset * fourier_display.fourier_size) / 100;

    setSolidBrush(&brush, colorDodgerBlue, 1.0);
    path = uiDrawNewPath(uiDrawFillModeWinding);
    const double freq_spacing = (double)p->AreaWidth / freq_size;
    const double y_margin = p->AreaHeight * 0.05;
    const double y_height = p->AreaHeight * 0.9;
    for (int k = 0; k < freq_size; k++) {
        const int i = 2 * (freq_start + k);
        if (freq_start + k >= fourier_display.fourier_size) break;
        const double fcr = fourier_display.fourier_coeffs[i], fci = fourier_display.fourier_coeffs[i + 1];
        const double intensity = fcr * fcr + fci * fci;
        const double x = k * freq_spacing;
        const double y = y_margin + y_height - (intensity / fourier_display.intensity_max) * y_height;
        uiDrawPathNewFigure(path, x, y_margin + y_height);
        uiDrawPathLineTo(path, x, y);
    }
    uiDrawPathEnd(path);

    uiDrawStrokeParams sp;
    memset(&sp, 0, sizeof (uiDrawStrokeParams));
    sp.Cap = uiDrawLineCapFlat;
    sp.Join = uiDrawLineJoinMiter;
    sp.Thickness = 1;
    sp.MiterLimit = uiDrawDefaultMiterLimit;
    uiDrawStroke(p->Context, path, &brush, &sp);
    uiDrawFreePath(path);
}

static void handlerFourierMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e) {
}

static void handlerFourierMouseCrossed(uiAreaHandler *ah, uiArea *a, int left) {
}

static void handlerFourierDragBroken(uiAreaHandler *ah, uiArea *a) {
}

static int handlerFourierKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e) {
    return 0;
}

static void onSpinboxOffsetChanged(uiSpinbox *spinbox, void *data) {
    uiAreaQueueRedrawAll(controls.area);
}

static void onSliderZoomChanged(uiSlider *slider, void *data) {
    uiAreaQueueRedrawAll(controls.area);
}

static int onFourierClosing(uiWindow *w, void *data) {
    fourier_window = NULL;
    return 1;
}

static uiWindow *new_fourier_window() {
    uiWindow *win = uiNewWindow("Fourier transform", 640, 480, 0);
    uiWindowSetMargined(win, 1);
    uiWindowOnClosing(win, onFourierClosing, NULL);

    uiBox *vbox = uiNewVerticalBox();
    uiBoxSetPadded(vbox, 1);
    uiWindowSetChild(win, uiControl(vbox));

    uiBox *hbox = uiNewHorizontalBox();
    uiBoxSetPadded(hbox, 1);
    uiBoxAppend(vbox, uiControl(hbox), 0);

    uiGroup *pos_group = uiNewGroup("Position");
    uiBoxAppend(hbox, uiControl(pos_group), 1);
    uiSpinbox *spinbox = uiNewSpinbox(0, 100);
    uiSpinboxOnChanged(spinbox, onSpinboxOffsetChanged, NULL);
    uiGroupSetChild(pos_group, uiControl(spinbox));
 
    uiGroup *group = uiNewGroup("Zoom");
    uiBoxAppend(hbox, uiControl(group), 1);
    uiSlider *slider = uiNewSlider(0, 10);
    // uiSliderSetValue(slider, slider_initial_value);
    uiSliderOnChanged(slider, onSliderZoomChanged, NULL);
    uiGroupSetChild(group, uiControl(slider));

    f_handler->Draw = handlerFourierDraw;
    f_handler->MouseEvent = handlerFourierMouseEvent;
    f_handler->MouseCrossed = handlerFourierMouseCrossed;
    f_handler->DragBroken = handlerFourierDragBroken;
    f_handler->KeyEvent = handlerFourierKeyEvent;

    controls.offset_spinbox = spinbox;
    controls.zoom_slider = slider;

    controls.area = uiNewArea(f_handler);
    uiBoxAppend(vbox, uiControl(controls.area), 1);

    uiControlShow(uiControl(win));
    return win;
}

void open_fourier_window(wav_reader_t *wav, int sample_start, int sample_size) {
    if (!fourier_window) {
        compute_fourier(wav, sample_start, sample_size);
        fourier_window = new_fourier_window();
    }
}

void update_fourier_window(wav_reader_t *wav, int sample_start, int sample_size) {
    if (fourier_window) {
        compute_fourier(wav, sample_start, sample_size);
        uiAreaQueueRedrawAll(controls.area);
    }
}
