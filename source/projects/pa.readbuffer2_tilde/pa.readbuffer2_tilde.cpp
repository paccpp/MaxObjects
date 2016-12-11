/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Read samples in a Max buffer~ at a given speed.

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_readbuffer2_tilde
{
    t_pxobject      m_obj;

    // phasor
    double          m_phase;

    // buffer
    t_buffer_ref*   m_buffer_reference;
};

void pa_readbuffer2_dsp_perform(t_pa_readbuffer2_tilde *x, t_object *dsp64,
                                double **ins, long numins, double **outs, long numouts,
                                long sampleframes, long flags, void *userparam)
{
    double *in = ins[0];
    double *out = outs[0];
    int n = sampleframes;
    double sr = sys_getsr();

    double speed = 0.f;
    double freq = 0.f;
    double phase = x->m_phase;
    double tphase = 0.f; // phase in 0. to buffersize. range

    // interpolation indexes
    long idx_1, idx_2;

    // interpolation values
    double y1, y2, frac;

    // buffer
    float *tab;
    long buffersize, nc;

    t_buffer_obj* buffer = buffer_ref_getobject(x->m_buffer_reference);
    if(buffer)
    {
        tab = buffer_locksamples(buffer);
        if(tab)
        {
            buffersize = buffer_getframecount(buffer);
            nc = buffer_getchannelcount(buffer);

            while(n--)
            {
                speed = *in++;

                freq = sr / buffersize * speed;

                // wrap phase between 0. and 1.
                if(phase >= 1.f) { phase -= 1.f; }
                else if(phase < 0.f) { phase += 1.f; }

                tphase = phase * buffersize;

                // we cast in int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
                idx_1 = (int)tphase;

                // wrap idx_2
                idx_2 = (idx_1 < (buffersize-1)) ? (idx_1+1) : 0;

                frac = tphase - idx_1;

                y1 = tab[idx_1 * nc];
                y2 = tab[idx_2 * nc];

                // linear interpolation
                *out++ = y1 + frac * (y2 - y1);

                // increment phase
                phase += (freq / sr);
            }

            buffer_unlocksamples(buffer);
        }
        else
        {
            while(n--) { *out++ = 0.; }
        }
    }
    else
    {
        while(n--) { *out++ = 0.; }
    }

    x->m_phase = phase;
}

void pa_readbuffer2_set(t_pa_readbuffer2_tilde *x, t_symbol *s)
{
    // reset phase
    x->m_phase = 0.;

    if (!x->m_buffer_reference)
        x->m_buffer_reference = buffer_ref_new((t_object *)x, s);
    else
        buffer_ref_set(x->m_buffer_reference, s);
}

void pa_readbuffer2_dsp_prepare(t_pa_readbuffer2_tilde *x, t_object *dsp64,
                                short *count, double samplerate, long maxvectorsize, long flags)
{
    dsp_add64(dsp64, (t_object *)x, (t_perfroutine64)pa_readbuffer2_dsp_perform, 0, NULL);
}

void pa_readbuffer2_assist(t_pa_readbuffer2_tilde *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) Phase between 0. and 1.");
    else
        sprintf(s,"(signal) Sample");
}

t_max_err pa_readbuffer2_notify(t_pa_readbuffer2_tilde *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    return buffer_ref_notify(x->m_buffer_reference, s, msg, sender, data);
}

void *pa_readbuffer2_new(t_symbol *s)
{
    t_pa_readbuffer2_tilde* x = (t_pa_readbuffer2_tilde*)object_alloc(this_class);

    if(x)
    {
        dsp_setup((t_pxobject *)x, 1);
        outlet_new((t_object *)x, "signal");

        pa_readbuffer2_set(x, s);
    }

    return (x);
}


void pa_readbuffer2_free(t_pa_readbuffer2_tilde *x)
{
    dsp_free((t_pxobject *)x);
    object_free(x->m_buffer_reference);
}

void ext_main(void *r)
{
    t_class *c = class_new("pa.readbuffer2~", (method)pa_readbuffer2_new, (method)pa_readbuffer2_free,
                           sizeof(t_pa_readbuffer2_tilde), 0L, A_SYM, 0);

    class_addmethod(c, (method)pa_readbuffer2_dsp_prepare,   "dsp64",    A_CANT,     0);
    class_addmethod(c, (method)pa_readbuffer2_set,           "set",      A_DEFSYM,   0);
    class_addmethod(c, (method)pa_readbuffer2_assist,        "assist",   A_CANT,     0);
    class_addmethod(c, (method)pa_readbuffer2_notify,        "notify",   A_CANT,     0);

    class_dspinit(c);
    class_register(CLASS_BOX, c);

    this_class = c;
}
