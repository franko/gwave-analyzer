#include <stdio.h>
#include <string.h>
#include <ui.h>

// names and values from https://msdn.microsoft.com/en-us/library/windows/desktop/dd370907%28v=vs.85%29.aspx
#define colorWhite 0xFFFFFF
#define colorBlack 0x000000
#define colorDodgerBlue 0x1E90FF

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
}

static void handlerMouseEvent(uiAreaHandler *a, uiArea *area, uiAreaMouseEvent *e) {
#if 0
    double graphWidth, graphHeight;
    double xs[10], ys[10];
    int i;

    graphSize(e->AreaWidth, e->AreaHeight, &graphWidth, &graphHeight);
    pointLocations(graphWidth, graphHeight, xs, ys);

    for (i = 0; i < 10; i++)
        if (inPoint(e->X, e->Y, xs[i], ys[i]))
            break;
    if (i == 10)        // not in a point
        i = -1;

    currentPoint = i;
    // TODO only redraw the relevant area
    uiAreaQueueRedrawAll(histogram);
#endif
}

static void handlerMouseCrossed(uiAreaHandler *ah, uiArea *a, int left)
{
    // do nothing
}

static void handlerDragBroken(uiAreaHandler *ah, uiArea *a)
{
    // do nothing
}

static int handlerKeyEvent(uiAreaHandler *ah, uiArea *a, uiAreaKeyEvent *e)
{
    // reject all keys
    return 0;
}

int main() {
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

    uiBoxAppend(hbox, uiControl(uiNewButton("Fourier")), 0);

    uiSpinbox *spinbox = uiNewSpinbox(0, 100);
    // uiSpinboxOnChanged(spinbox, onSpinboxChanged, NULL);
    uiBoxAppend(hbox, uiControl(spinbox), 1);

    uiGroup *group = uiNewGroup("Zoom");
    uiBoxAppend(hbox, uiControl(group), 1);
    uiSlider *slider = uiNewSlider(0, 10);
    // uiSliderOnChanged(slider, onSliderChanged, NULL);
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

    uiArea *wav_display_area = uiNewArea(wav_display_handler);
    uiBoxAppend(vbox, uiControl(wav_display_area), 1);

    uiControlShow(uiControl(mainwin));
    uiMain();
    return 0;
}
