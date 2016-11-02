/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

// A phasor for Max signal objects

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_phasor_tilde
{
    t_pxobject  m_obj;
    
    double      m_phase;
    double      m_sr;
    
    // to perform with float freq
    double      m_freq;
    double      m_phase_inc;
};

void pa_phasor_tilde_float(t_pa_phasor_tilde* x, double d)
{
    x->m_freq = d;
    x->m_phase_inc = (x->m_freq / x->m_sr);
}

void pa_phasor_tilde_int(t_pa_phasor_tilde* x, long l)
{
    pa_phasor_tilde_float(x, (double)l);
}

void pa_phasor_tilde_perform64_vec(t_pa_phasor_tilde* x, t_object* dsp64,
                                   double** ins, long numins, double** outs, long numouts,
                                   long vecsize, long flags, void* userparam)
{
    double* in = ins[0];
    double* out = outs[0];
    
    const double sr = x->m_sr;
    double freq = 0.f;
    double phase_inc = 0.f;
    double phase = x->m_phase;
    
    while(vecsize--)
    {
        freq = *in++;
        
        *out++ = phase;
        
        phase_inc = (freq / sr);
        
        if(phase >= 1.f) phase -= 1.f;
        if(phase < 0.f) phase += 1.f;
        
        phase += phase_inc;
    }
    
    x->m_phase = phase;
}

void pa_phasor_tilde_perform64_float(t_pa_phasor_tilde* x, t_object* dsp64,
                                     double** ins, long numins, double** outs, long numouts,
                                     long vecsize, long flags, void* userparam)
{
    double* out = outs[0];
    
    const double phase_inc = x->m_phase_inc;
    double phase = x->m_phase;
    
    while(vecsize--)
    {
        *out++ = phase;
        
        if(phase >= 1.f) phase -= 1.f;
        if(phase < 0.f) phase += 1.f;
        
        phase += phase_inc;
    }
    
    x->m_phase = phase;
}


void pa_phasor_tilde_dsp64(t_pa_phasor_tilde* x, t_object* dsp64, short* count,
                           double samplerate, long maxvectorsize, long flags)
{
    x->m_sr = sys_getsr();
    
    if(count[0])
    {
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_phasor_tilde_perform64_vec, 0, NULL);
    }
    else
    {
        pa_phasor_tilde_float(x, x->m_freq);
        
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_phasor_tilde_perform64_float, 0, NULL);
    }
}

void pa_phasor_tilde_assist(t_pa_phasor_tilde* x, void* unused,
                            t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        strncpy(string_dest, "(signal) Frequency", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) Output", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_phasor_tilde_new(void)
{
    t_pa_phasor_tilde* x = (t_pa_phasor_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_phase = 0.;
        x->m_freq = 0.;
        x->m_phase_inc = 0.;
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_phasor_tilde_free(t_pa_phasor_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.phasor~", (method)pa_phasor_tilde_new, (method)pa_phasor_tilde_free,
                           sizeof(t_pa_phasor_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_phasor_tilde_assist,     "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_phasor_tilde_dsp64,      "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_phasor_tilde_float,      "float",    A_FLOAT,    0);
    class_addmethod(this_class, (method)pa_phasor_tilde_int,        "int",      A_LONG,     0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
