/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Converts signal into float at a given time interval.

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_snapshot_tilde
{
    t_pxobject  m_obj;
    
    double      m_value;
    double      m_interval_ms;
    
    t_clock*    m_clock;
    void*       m_outlet;
};

void pa_snapshot_tilde_bang(t_pa_snapshot_tilde* x)
{
    outlet_float(x->m_outlet, x->m_value);
}

void pa_snapshot_tilde_tick(t_pa_snapshot_tilde *x)
{
    if(sys_getdspstate() && x->m_interval_ms > 0)
    {
        pa_snapshot_tilde_bang(x);
        
        // schedule the execution of the clock.
        clock_fdelay(x->m_clock, x->m_interval_ms);
    }
}

void pa_snapshot_tilde_dsp_perform(t_pa_snapshot_tilde* x, t_object* dsp64,
                                double** ins, long numins, double** outs, long numouts,
                                long vecsize, long flags, void* userparam)
{
    double const* in = ins[0];
    
    while(vecsize--)
    {
        x->m_value = *in++;
    }
}


void pa_snapshot_tilde_dsp_prepare(t_pa_snapshot_tilde* x, t_object* dsp64, short* count,
                                   double samplerate, long maxvectorsize, long flags)
{
    if(x->m_interval_ms > 0)
    {
        // schedule the execution of the clock.
        clock_fdelay(x->m_clock, x->m_interval_ms);
        pa_snapshot_tilde_bang(x);
    }
    
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_snapshot_tilde_dsp_perform, 0, NULL);
}

void pa_snapshot_tilde_assist(t_pa_snapshot_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        strncpy(string_dest, "(signal) input, (bang) reports signal values", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(float) Signal values", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_snapshot_tilde_new(t_symbol* name, long argc, t_atom *argv)
{
    t_pa_snapshot_tilde* x = (t_pa_snapshot_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_interval_ms = 0.;
        x->m_value = 0.f;
        
        // first argument set the reporting interval in ms (must be positive)
        // 0 means no automatic report
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            const double interval = atom_getfloat(argv);
            x->m_interval_ms = interval >= 1.f ? interval : 0.f;
        }
        
        // create the clock, passing the method to be called by the clock as second parameter
        x->m_clock = clock_new(x, (method)pa_snapshot_tilde_tick);
        
        // setup one signal inlet
        dsp_setup((t_pxobject*)x, 1);
        
        x->m_outlet = outlet_new(x, "float");
    }
    
    return x;
}

void pa_snapshot_tilde_free(t_pa_snapshot_tilde* x)
{
    // get rid of the clock
    freeobject(x->m_clock);
    
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.snapshot~", (method)pa_snapshot_tilde_new, (method)pa_snapshot_tilde_free,
                           sizeof(t_pa_snapshot_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_snapshot_tilde_assist,       "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_snapshot_tilde_dsp_prepare,  "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_snapshot_tilde_bang,         "bang",                 0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
