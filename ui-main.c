#include <stdio.h>
#include <string.h>
#include <ui.h>

#include "wav_reader.h"

// names and values from https://msdn.microsoft.com/en-us/library/windows/desktop/dd370907%28v=vs.85%29.aspx
#define colorWhite 0xFFFFFF
#define colorBlack 0x000000
#define colorDodgerBlue 0x1E90FF

struct {
    wav_reader_t wav[1];
    int sample_start; /* Where the shown waveform starts, in samples number. */
    int sample_size; /* Number for waveform's sample to show on window. */
    uiArea *area; /* Area used to show waveform. */
} app;

static int onClosing(uiWindow *w, void *data) {
    uiQuit();
    return 1;
}

static int onShouldQuit(void *data) {
    uiWindow *mainwin = uiWindow(data);
    uiControlDestroy(uiControl(mainwin));
    return 1;
}

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

static void handlerDraw(uiAreaHandler *a, uiArea *area, uiAreaDrawParams *p) {
    uiDrawBrush brush;
    setSolidBrush(&brush, colorWhite, 1.0);

    uiDrawPath *path = uiDrawNewPath(uiDrawFillModeWinding);
    uiDrawPathAddRectangle(path, 0, 0, p->AreaWidth, p->AreaHeight);
    uiDrawPathEnd(path);
    uiDrawFill(p->Context, path, &brush);
    uiDrawFreePath(path);

    setSolidBrush(&brush, colorDodgerBlue, 1.0);
    path = uiDrawNewPath(uiDrawFillModeWinding);

    wav_reader_seek(app.wav, app.sample_start);
    uiDrawPathNewFigure(path, 0, p->AreaHeight / 2);
    const double sample_pixel_size = (double)p->AreaWidth / (double)app.sample_size;
    for (int sample = 0; sample < app.sample_size; sample++) {
        double amp[2];
        getone(app.wav, amp);
        double x = sample * sample_pixel_size;
        double y = amp[0] * p->AreaHeight / 2 + p->AreaHeight / 2;
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

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e) {
}

static void handlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left) {
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a) {
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e) {
    return 0;
}

static void onSpinboxOffsetChanged(uiSpinbox *spinbox, void *data) {
    const int offset = uiSpinboxValue(spinbox);
    app.sample_start = offset * (app.wav->spec.num_sample / 100);
    uiAreaQueueRedrawAll(app.area);
}

static void onSliderZoomChanged(uiSlider *slider, void *data) {
    const unsigned int p = uiSliderValue(slider);
    app.sample_size = (1 << p);
    uiAreaQueueRedrawAll(app.area);
}

static void onFourierClicked(uiButton *b, void *data) {

}

static FILE *fopen_binary_read(const char *filename) {
#ifdef WIN32
    return fopen(filename, "rb");
#else
    return fopen(filename, "r");
#endif
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    wav_reader_set_zero(app.wav);

    const char *filename = argv[1];
    app.wav->file = fopen_binary_read(filename);
    int wav_read_status = wav_reader_init(app.wav, filename);
    if (wav_read_status != WAV_READER_SUCCESS) {
        fprintf(stderr, "Error reading wav file %s. Error code: %d.\n", filename, wav_read_status);
        return 1;
    }

    uiInitOptions options;
    memset(&options, 0, sizeof (uiInitOptions));
    const char *err = uiInit(&options);
    if (err != NULL) {
        fprintf(stderr, "%s", err);
        uiFreeInitError(err);
        return 1;
    }

    uiMenu *fileMenu = uiNewMenu("File");
    uiMenuItem *openItem = uiMenuAppendItem(fileMenu, "Open");
    uiMenuItem *quitItem = uiMenuAppendQuitItem(fileMenu);

    uiMenu *helpMenu = uiNewMenu("Help");
    uiMenuItem *helpItem = uiMenuAppendItem(helpMenu, "Help");
    uiMenuItem *aboutItem = uiMenuAppendAboutItem(helpMenu);

    uiWindow *mainwin = uiNewWindow("GWave analyzer", 640, 480, 1);
    uiWindowSetMargined(mainwin, 1);
    uiWindowOnClosing(mainwin, onClosing, NULL);
    uiOnShouldQuit(onShouldQuit, mainwin);

    uiBox *vbox = uiNewVerticalBox();
    uiBoxSetPadded(vbox, 1);
    uiWindowSetChild(mainwin, uiControl(vbox));

    uiBox *hbox = uiNewHorizontalBox();
    uiBoxSetPadded(hbox, 1);
    uiBoxAppend(vbox, uiControl(hbox), 0);

    uiButton fourier_button = uiNewButton("Fourier");
    uiBoxAppend(hbox, uiControl(fourier_button), 0);
    uiButtonOnClicked(fourier_button, onFourierClicked, NULL);

    uiSpinbox *spinbox = uiNewSpinbox(0, 100);
    uiSpinboxOnChanged(spinbox, onSpinboxOffsetChanged, NULL);
    uiBoxAppend(hbox, uiControl(spinbox), 1);

    const unsigned int slider_initial_value = 12;
    uiGroup *group = uiNewGroup("Zoom");
    uiBoxAppend(hbox, uiControl(group), 1);
    uiSlider *slider = uiNewSlider(6, 20);
    uiSliderSetValue(slider, slider_initial_value);
    uiSliderOnChanged(slider, onSliderZoomChanged, NULL);
    uiGroupSetChild(group, uiControl(slider));

    uiRadioButtons *rb = uiNewRadioButtons();
    uiRadioButtonsAppend(rb, "Channel 1");
    uiRadioButtonsAppend(rb, "Channel 2");
    uiRadioButtonsAppend(rb, "Average");
    // uiRadioButtonsOnSelected(rb, onRBSelected, NULL);
    uiBoxAppend(hbox, uiControl(rb), 0);

    uiAreaHandler wav_display_handler[1];
    wav_display_handler->Draw = handlerDraw;
    wav_display_handler->MouseEvent = handlerMouseEvent;
    wav_display_handler->MouseCrossed = handlerMouseCrossed;
    wav_display_handler->DragBroken = handlerDragBroken;
    wav_display_handler->KeyEvent = handlerKeyEvent;

    app.sample_size = (1 << slider_initial_value);
    app.sample_start = 0;

    app.area = uiNewArea(wav_display_handler);
    uiBoxAppend(vbox, uiControl(app.area), 1);

    uiControlShow(uiControl(mainwin));
    uiMain();
    return 0;
}
