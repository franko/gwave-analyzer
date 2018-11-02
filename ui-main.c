#include <stdio.h>
#include <string.h>
#include <ui.h>

static int onClosing(uiWindow *w, void *data) {
    uiQuit();
    return 1;
}

static int onShouldQuit(void *data) {
    uiWindow *mainwin = uiWindow(data);
    uiControlDestroy(uiControl(mainwin));
    return 1;
}

int main() {
    uiInitOptions options;
    memset(&options, 0, sizeof (uiInitOptions));
    const char *err = uiInit(&options);
    /* workaround: in libui alpha4.1 Common Controls return an error but with code 0, so there
       is actually no error. */
    if (err != NULL && strstr(err, "initializing Common Controls; code 0 ") == NULL) {
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
    uiWindowOnClosing(mainwin, onClosing, NULL);
    uiOnShouldQuit(onShouldQuit, mainwin);

    uiBox *vbox = uiNewVerticalBox();
    uiBoxSetPadded(vbox, 1);

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

    uiWindowSetChild(mainwin, uiControl(vbox));
    uiWindowSetMargined(mainwin, 1);

    uiControlShow(uiControl(mainwin));
    uiMain();
    return 0;
}
