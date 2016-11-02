/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Outputs a signal increasing by 1 for each sample elapsed

#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_count_tilde
{
    t_pxobject m_obj;
    
    long       m_min;
    long       m_max;
    long       m_value;
    
    long*      m_proxy;
};

void pa_count_tilde_setminmax(t_pa_count_tilde* x, long min, long max)
{
    if(min > max) min = max;
    if(max < min) max = min;
    
    x->m_min = min;
    x->m_max = max;
}

void pa_count_tilde_int(t_pa_count_tilde* x, long l)
{
    const long index = proxy_getinlet((t_object*)x);
    //object_post((t_object*)x, "index = %i", index);
    
    if(index == 0)
    {
        pa_count_tilde_setminmax(x, l, x->m_max);
    }
    else if (index == 1)
    {
        pa_count_tilde_setminmax(x, x->m_min, l);
    }
}

void pa_count_tilde_float(t_pa_count_tilde* x, double d)
{
    pa_count_tilde_int(x, (long)d);
}

void pa_count_tilde_perform64(t_pa_count_tilde* x, t_object* dsp64,
                              double** ins, long numins, double** outs, long numouts,
                              long vecsize, long flags, void* userparam)
{
    double* out = outs[0];
    
    // cache our value
    int value = x->m_value;
    
    while(vecsize--)
    {
        if(value > x->m_max)
        {
            value = x->m_min;
        }
        
        // set the output sample value then increment output pointer
        *out++ = value;
        
        // increment the value
        ++value;
    }
    
    x->m_value = value;
}

void pa_count_tilde_dsp64(t_pa_count_tilde* x, t_object* dsp64, short* count,
                            double samplerate, long maxvectorsize, long flags)
{
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_count_tilde_perform64, 0, NULL);
}

void pa_count_tilde_assist(t_pa_count_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        switch(index)
        {
            case 0:
                strncpy(string_dest, "(int) Minimum count", ASSIST_STRING_MAXSIZE);
                break;
            case 1:
                strncpy(string_dest, "(int) Maximum count", ASSIST_STRING_MAXSIZE);
                break;
        }
    }
    else if(io == ASSIST_OUTLET)
    {
        switch (index)
        {
            case 0:
                strncpy(string_dest, "(signal) Counter output", ASSIST_STRING_MAXSIZE);
                break;
        }
    }
}

void* pa_count_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_count_tilde* x = (t_pa_count_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_min = 0;
        x->m_max = 44100;
        x->m_value = 0;
        
        // first argument set the minimum count value
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            x->m_min = atom_getfloat(argv);
        }
        
        // second argument set the maximum count value
        if(argc >= 2 && (atom_gettype(argv+1) == A_FLOAT || atom_gettype(argv+1) == A_LONG))
        {
            x->m_max = atom_getfloat(argv+1);
        }
        
        pa_count_tilde_setminmax(x, x->m_min, x->m_max);
        
        proxy_new((t_object*)x, 1, x->m_proxy);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_count_tilde_free(t_pa_count_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.count~", (method)pa_count_tilde_new, (method)pa_count_tilde_free,
                           sizeof(t_pa_count_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_count_tilde_assist,  "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_count_tilde_dsp64,   "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_count_tilde_float,   "float",    A_FLOAT,    0);
    class_addmethod(this_class, (method)pa_count_tilde_int,     "int",      A_LONG,     0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
