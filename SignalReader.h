#pragma once

class SignalReader {
public:
    enum sample_e { real_sample, complex_sample };

    void read_samples(int pos, int size, double buffer[], sample_e sample_select);
    double read_sample();
    void move_to(int pos);
};

class FFTWorkspace {
public:
    void ensure_size(int n);
    double *buffer();
};
