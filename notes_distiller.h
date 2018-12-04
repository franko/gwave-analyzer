#pragma once

#include "wav2midi_priv.h"

struct notes_distiller_;
typedef struct notes_distiller_ notes_distiller;

extern notes_distiller *notes_distiller_new();
extern int distiller_add_event(notes_distiller *dist, const event_t *ev);
extern int distiller_close(notes_distiller *dist);
extern int distiller_get_note(notes_distiller *dist, event_t *note);
