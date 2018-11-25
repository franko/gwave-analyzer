#include "ui-fourier.h"
#include "fourier.h"

typedef struct {
    double *fourier_coeffs;
    int fourier_size;
    int freq_start;
    int freq_size;
    uiArea *area;
} fourier_display_info;

static uiWindow *fourier_window;

static fourier_display_info fourier_display;

void compute_fourier(wav_reader_t *wav, int sample_start, int sample_size) {
    fourier(wav, sample_size, sample_start);
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

    setSolidBrush(&brush, colorDodgerBlue, 1.0);
    path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawPathNewFigure(path, 0, p->AreaHeight / 2);
    const double freq_spacing = (double)p->AreaWidth / (double)fourier_display_info.freq_size;
    for (int k = 0; k < fourier_display.freq_size; k++) {
        const double fcr = fourier_display.fourier_coeffs[2 * k], fci = fourier_display.fourier_coeffs[2 * k + 1];
        const double intensity = fcr * fcr + fci * fci;
        const double x = k * freq_spacing;
        const double y = intensity * p->AreaHeight / 2 + p->AreaHeight / 2;
        uiDrawPathLineTo(path, (double) x, y);
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

static uiWindow *new_fourier_window() {
    uiWindow *win = uiNewWindow("Fourier transform", 640, 480, NULL);
    uiWindowSetMargined(win, 1);
    // uiWindowOnClosing(win, onClosing, NULL);

    uiBox *vbox = uiNewVerticalBox();
    uiBoxSetPadded(vbox, 1);
    uiWindowSetChild(win, uiControl(vbox));

    uiBox *hbox = uiNewHorizontalBox();
    uiBoxSetPadded(hbox, 1);
    uiBoxAppend(vbox, uiControl(hbox), 0);

    uiGroup *pos_group = uiNewGroup("Position");
    uiSpinbox *spinbox = uiNewSpinbox(0, 100);
    uiSpinboxOnChanged(spinbox, onSpinboxOffsetChanged, NULL);
    uiGroupSetChild(pos_group, uiControl(spinbox));
    uiBoxAppend(hbox, uiControl(pos_group), 1);
 
    uiGroup *group = uiNewGroup("Zoom");
    uiBoxAppend(hbox, uiControl(group), 1);
    uiSlider *slider = uiNewSlider(0, 10);
    // uiSliderSetValue(slider, slider_initial_value);
    uiSliderOnChanged(slider, onSliderZoomChanged, NULL);
    uiGroupSetChild(group, uiControl(slider));

    uiAreaHandler f_handler[1];
    f_handler->Draw = handlerFourierDraw;
    f_handler->MouseEvent = handlerFourierMouseEvent;
    f_handler->MouseCrossed = handlerFourierMouseCrossed;
    f_handler->DragBroken = handlerFourierDragBroken;
    f_handler->KeyEvent = handlerFourierKeyEvent;

    fourier_display.fourier_coeffs = NULL;
    fourier_display.fourier_size = 0;
    fourier_display.freq_size = sample_size;
    fourier_display.fred_start = 0;

    fourier_display.area = uiNewArea(f_handler);
    uiBoxAppend(vbox, uiControl(fourier_display.area), 1);

    uiControlShow(uiControl(win));
    return win;
}

uiWindow *gwave_open_fourier_window() {
    if (fourier_window) {
        return fourier_window;
    }
    return new_fourier_window();
}
