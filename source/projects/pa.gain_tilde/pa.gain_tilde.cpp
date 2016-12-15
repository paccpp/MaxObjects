// Copyright (c) 2016 Eliott Paris.
// For information on usage and redistribution, and for a DISCLAIMER OF ALL
// WARRANTIES, see the file, "LICENSE.txt," in this distribution.

//! @brief Multiply signal with a smooth transition

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_gain_tilde
{
    t_pxobject  m_obj;
    double      m_gain;
    double      m_gain_to;
    double      m_gain_increment;
    int         m_samps_to_fade;
};

void pa_gain_tilde_set_gain(t_pa_gain_tilde *x, double new_gain, double ramp_time_ms)
{
    // gain should be positive
    x->m_gain_to = (new_gain > 0.) ? new_gain : 0.;
    
    // if the ramp time is positive then calculate the number of samples needed to smooth gain
    x->m_samps_to_fade = (ramp_time_ms > 0) ? (int)(sys_getsr() * 0.001 * ramp_time_ms) : 0;
    
    // compute gain increment for each samples
    x->m_gain_increment = (x->m_samps_to_fade > 0) ? (x->m_gain_to - x->m_gain) / (float)x->m_samps_to_fade : 0;
}

void pa_gain_tilde_dsp_perform(t_pa_gain_tilde* x, t_object* dsp64,
                               double** ins, long numins, double** outs, long numouts,
                               long vecsize, long flags, void* userparam)
{
    double const* in = ins[0];
    double* out = outs[0];
    
    while(vecsize--)
    {
        if(x->m_samps_to_fade > 0)
        {
            x->m_gain += x->m_gain_increment;
            x->m_samps_to_fade--;
        }
        else
        {
            x->m_gain = x->m_gain_to;
        }
        
        *out++ = *in++ * x->m_gain;
    }
}


void pa_gain_tilde_dsp_prepare(t_pa_gain_tilde* x, t_object* dsp64, short* count,
                               double samplerate, long maxvectorsize, long flags)
{
    // reset gain
    pa_gain_tilde_set_gain(x, x->m_gain_to, 0.);
    
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_gain_tilde_dsp_perform, 0, NULL);
}

void pa_gain_tilde_assist(t_pa_gain_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        strncpy(string_dest, "(signal) input", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) output", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_gain_tilde_new(void)
{
    t_pa_gain_tilde* x = (t_pa_gain_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_gain = x->m_gain_to = x->m_gain_increment = 0.;
        x->m_samps_to_fade = 0;
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_gain_tilde_free(t_pa_gain_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.gain~", (method)pa_gain_tilde_new, (method)pa_gain_tilde_free,
                           sizeof(t_pa_gain_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_gain_tilde_assist,       "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_gain_tilde_dsp_prepare,  "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_gain_tilde_set_gain,     "gain",     A_FLOAT, A_DEFFLOAT, 0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
