#ifndef UVIC_RTSYS_FILTER_INCLUDED
#define UVIC_RTSYS_FILTER_INCLUDED

struct filter_state
{
    int out_1;
};

inline int filter_init(filter_state *f)
{
    f->out_1 = 0;
}

inline int filter_process(filter_state *f, int in)
{
    int out = (0.2 * in + 0.8 * f->out_1);
    f->out_1 = out;
    return out;
}

#endif // UVIC_RTSYS_FILTER_INCLUDED
