/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A Sample And Hold object.
//! @detail Capture and continually output the value of an input signal
//! whenever another "control" signal rises above a specified threshold value.

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_sah_tilde
{
    t_pxobject  m_obj;
    double      m_threshold;
    double      m_hold_value;
    double      m_last_ctrl_sample;
};

void pa_sah_tilde_float(t_pa_sah_tilde *x, double f)
{
    x->m_threshold = f;
}

void pa_sah_tilde_perform64(t_pa_sah_tilde* x, t_object* dsp64,
                            double** ins, long numins, double** outs, long numouts,
                            long vectorsize, long flags, void* userparam)
{
    double* in1 = ins[0];
    double* in2 = ins[1];
    double* out = outs[0];
    
    const double thresh = x->m_threshold;
    double value;
    double ctrl_sample;
    
    while(vectorsize--)
    {
        value = *in1++;
        ctrl_sample = *in2++;
        
        if(x->m_last_ctrl_sample <= thresh && ctrl_sample > thresh)
        {
            x->m_hold_value = value;
        }
        
        *out++ = x->m_hold_value;
        x->m_last_ctrl_sample = ctrl_sample;
    }
}


void pa_sah_tilde_dsp64(t_pa_sah_tilde* x, t_object* dsp64, short* count,
                        double samplerate, long maxvectorsize, long flags)
{
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_sah_tilde_perform64, 0, NULL);
}

void pa_sah_tilde_assist(t_pa_sah_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        if(index == 0)
            strncpy(string_dest, "(signal) Values to sample", ASSIST_STRING_MAXSIZE);
        else
            strncpy(string_dest, "(signal) Trigger input", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) Output sampled value", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_sah_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pa_sah_tilde* x = (t_pa_sah_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_threshold = 0.f;
        x->m_hold_value = 0.f;
        x->m_last_ctrl_sample = 0.f;
        
        // first argument set the threshold value
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            x->m_threshold = atom_getfloat(argv);
        }
        
        dsp_setup((t_pxobject*)x, 2);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_sah_tilde_free(t_pa_sah_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.sah~", (method)pa_sah_tilde_new, (method)pa_sah_tilde_free,
                           sizeof(t_pa_sah_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_sah_tilde_assist,    "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_sah_tilde_dsp64,     "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_sah_tilde_float,     "float",    A_FLOAT,    0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
