enum { DISTILLER_ACCUMULATING, DISTILLER_EVAL_NEW };
enum { DISTILLER_MAX_EVAL_DURATION = 50 };

typedef struct {
    short status;
    short note_ready;

    double pitch;
    int duration;

    double eval_pitch;
    int eval_duration;

    double distilled_pitch;
    int distilled_duration;
    int distilled_pause;
}  notes_distiller;

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

int distiller_add_event(notes_distiller *dist, const event_t *ev) {
    if (ev->pd > 0) {
        collapse_eval_note(dist);
        distill_current(dist, ev->pd);
        return 1;
    }
    const double event_pitch = freq2pitch(ev->dominant_freq);
    if (dist->status == DISTILLER_EVAL_NEW) {
        if (fabs(event_pitch - dist->eval_pitch) < 0.5) {
            add_event_to_eval(dist, ev);
            if (dist->eval_duration > DISTILLER_MAX_EVAL_DURATION) {
                distill_current(dist, 0);
                set_eval_as_current(dist);
                return 1;
            } else {
                return 0;
            }
        } else if (fabs(event_pitch - dist->current_pitch) < 0.5) {
            collapse_eval_note(dist);
            add_event_to_current(dist, ev);
            return 0;
        } else {
            collapse_eval_note(dist);
            distill_current(dist, 0);
            return 1;
        }
    } else if (dist->status == DISTILLER_ACCUMULATING) {
        if (fabs(event_pitch - dist->current_pitch) < 0.5) {
            add_event_to_current(dist, ev);
            return 0;
        } else {
            dist->eval_pitch = event_pitch;
            dist->eval_duration = ev->nd;
            dist->status = DISTILLER_EVAL_NEW;
            return 0;
        }
    }
    return 0;
}

static void yield_note(notes_distiller *dist, event_t *note) {
    note->nota = rint(dist->distilled_pitch);
    note->nd = dist->distilled_duration;
    note->pd = dist->distill_pause;
    note->dominant_freq = exp(log(2) * ((dist->distilled_pitch - 62.0) / 12.0 + 8.2));
    note->offs_inizio = 0;
    dist->note_ready = 0;
}

int distiller_get_note(notes_distiller *dist, event_t *note) {
    if (dist->note_ready) {
        yield_note(dist, note);
        return 1;
    } else {
        collapse_eval_note(dist);
        if (dist->duration > DISTILLER_MAX_EVAL_DURATION) {
            distill_current(dist, 0);
            yield_note(dist, note);
            return 1;
        }
    }
    return 0;
}
