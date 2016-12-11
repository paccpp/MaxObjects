/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

// Clip signal between minimum and maximum values

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_clip_tilde
{
    t_pxobject      m_obj;
    t_atom_float    m_min;
    t_atom_float    m_max;
};

void pa_clip_tilde_set_minmax(t_pa_clip_tilde *x, t_atom_float min, t_atom_float max)
{
    if(min <= max)
    {
        x->m_min = min;
        x->m_max = max;
    }
    else
    {
        x->m_min = max;
        x->m_max = min;
    }
}

void pa_clip_tilde_set_min(t_pa_clip_tilde *x, t_atom_float value)
{
    pa_clip_tilde_set_minmax(x, value, x->m_max);
}

void pa_clip_tilde_set_max(t_pa_clip_tilde *x, t_atom_float value)
{
    pa_clip_tilde_set_minmax(x, x->m_min, value);
}

void pa_clip_tilde_dsp_perform(t_pa_clip_tilde* x, t_object* dsp64,
                               double** ins, long numins, double** outs, long numouts,
                               long vectorsize, long flags, void* userparam)
{
    double* in = ins[0];
    double* out = outs[0];
    
    const t_atom_float min = x->m_min;
    const t_atom_float max = x->m_max;
    double value = 0.f;
    
    while(vectorsize--)
    {
        value = *in++;
        
        if(value < min) value = min;
        else if(value > max) value = max;
        
        *out++ = value;
    }
}


void pa_clip_tilde_dsp_prepare(t_pa_clip_tilde* x, t_object* dsp64, short* count,
                               double samplerate, long maxvectorsize, long flags)
{
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_clip_tilde_dsp_perform, 0, NULL);
}

void pa_clip_tilde_assist(t_pa_clip_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        strncpy(string_dest, "(signal) Signal to be clipped", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) Clipped signal", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_clip_tilde_new(t_symbol* name, long ac, t_atom* av)
{
    t_pa_clip_tilde* x = (t_pa_clip_tilde*)object_alloc(this_class);
    
    if(x)
    {
        t_atom_float min = -1.;
        t_atom_float max = 1.;
        
        // first argument set the minimum value
        if(ac >= 1 && (atom_gettype(av) == A_FLOAT || atom_gettype(av) == A_LONG))
        {
            min = atom_getfloat(av);
        }
        
        // second argument set the maximum value
        if(ac >= 2 && (atom_gettype(av+1) == A_FLOAT || atom_gettype(av+1) == A_LONG))
        {
            max = atom_getfloat(av+1);
        }
        
        pa_clip_tilde_set_minmax(x, min, max);
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_clip_tilde_free(t_pa_clip_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.clip~", (method)pa_clip_tilde_new, (method)pa_clip_tilde_free,
                           sizeof(t_pa_clip_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_clip_tilde_assist,       "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_clip_tilde_dsp_prepare,  "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_clip_tilde_set_min,      "min",      A_FLOAT,    0);
    class_addmethod(this_class, (method)pa_clip_tilde_set_max,      "max",      A_FLOAT,    0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
