#include <stdlib.h>
#include "notes_distiller.h"
#include "wav2midi.h"

enum { DISTILLER_STARTING, DISTILLER_ACCUMULATING, DISTILLER_EVAL_NEW };
enum { DISTILLER_MAX_EVAL_DURATION = 50 };

struct notes_distiller_ {
    short status;
    short note_ready;

    double pitch;
    int duration;

    double eval_pitch;
    int eval_duration;

    double distilled_pitch;
    int distilled_duration;
    int distilled_pause;
};

static void collapse_eval_note(notes_distiller *dist) {
    dist->duration += dist->eval_duration;
    dist->eval_duration = 0;
    dist->status = DISTILLER_ACCUMULATING;
}

static void distill_current(notes_distiller *dist, int pd) {
    dist->distilled_pitch = dist->pitch;
    dist->distilled_duration = dist->duration;
    dist->distilled_pause = pd;
    dist->duration = 0;
    dist->note_ready = 1;
}

static void set_eval_as_current(notes_distiller *dist) {
    dist->pitch = dist->eval_pitch;
    dist->duration = dist->eval_duration;
    dist->eval_duration = 0;
    dist->status = DISTILLER_ACCUMULATING;
}

static void add_event_to_current(notes_distiller *dist, const event_t *ev) {
    const double event_pitch = freq2pitch(ev->dominant_freq);
    dist->pitch = (dist->pitch * dist->duration + event_pitch * ev->nd) / (dist->duration + ev->nd);
    dist->duration += ev->nd;
}

static void add_event_to_eval(notes_distiller *dist, const event_t *ev) {
    const double event_pitch = freq2pitch(ev->dominant_freq);
    dist->eval_pitch = (dist->eval_pitch * dist->eval_duration + event_pitch * ev->nd) / (dist->eval_duration + ev->nd);
    dist->eval_duration += ev->nd;
}

static int check_eval_and_return(notes_distiller *dist) {
    if (dist->eval_duration > DISTILLER_MAX_EVAL_DURATION) {
        distill_current(dist, 0);
        set_eval_as_current(dist);
        return 1;
    }
    return 0;
}

int distiller_add_event(notes_distiller *dist, const event_t *ev) {
    if (ev->pd > 0) {
        collapse_eval_note(dist);
        distill_current(dist, ev->pd);
        return 1;
    }
    const double event_pitch = freq2pitch(ev->dominant_freq);
    if (dist->status == DISTILLER_STARTING) {
        dist->status = DISTILLER_EVAL_NEW;
        add_event_to_eval(dist, ev);
        return check_eval_and_return(dist);
    } else if (dist->status == DISTILLER_EVAL_NEW) {
        if (fabs(event_pitch - dist->eval_pitch) < 0.5) {
            add_event_to_eval(dist, ev);
            return check_eval_and_return(dist);
        } else if (fabs(event_pitch - dist->pitch) < 0.5) {
            collapse_eval_note(dist);
            add_event_to_current(dist, ev);
            return 0;
        } else {
            collapse_eval_note(dist);
            add_event_to_eval(dist, ev);
            return 1;
        }
    } else if (dist->status == DISTILLER_ACCUMULATING) {
        if (fabs(event_pitch - dist->pitch) < 0.5) {
            add_event_to_current(dist, ev);
            return 0;
        } else {
            add_event_to_eval(dist, ev);
            dist->status = DISTILLER_EVAL_NEW;
            return check_eval_and_return(dist);
        }
    }
    return 0;
}

int distiller_close(notes_distiller *dist) {
    if (dist->status == DISTILLER_EVAL_NEW) {
        collapse_eval_note(dist);
    }
    return check_eval_and_return(dist);
}

static void yield_note(notes_distiller *dist, event_t *note) {
    note->nota = rint(dist->distilled_pitch);
    note->nd = dist->distilled_duration;
    note->pd = dist->distilled_pause;
    note->dominant_freq = exp(log(2) * ((dist->distilled_pitch - 62.0) / 12.0 + 8.2));
    note->offs_inizio = 0;
    dist->note_ready = 0;
}

int distiller_get_note(notes_distiller *dist, event_t *note) {
    if (dist->note_ready) {
        yield_note(dist, note);
        return 1;
    }
    return 0;
}

notes_distiller *notes_distiller_new() {
    notes_distiller *dist = malloc(sizeof(notes_distiller));
    if (dist) {
        dist->status = DISTILLER_STARTING;
        dist->note_ready = 0;
        dist->duration = 0;
        dist->eval_duration = 0;
        dist->distilled_duration = 0;
    }
    return dist;
}
