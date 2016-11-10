/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A fixed delay (of 441 samples)
//! @remark Max will not instantiate objects with too big static memory allocation in their structure.

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

#define DELAY1_MAX_DELAY 441

struct t_pa_delay1_tilde
{
    t_pxobject  m_obj;
    
    int         m_count;
    double      m_buffer[DELAY1_MAX_DELAY];
};

void pa_delay1_tilde_clear_buffer(t_pa_delay1_tilde* x)
{
    int i = 0;
    for(; i < DELAY1_MAX_DELAY; ++i)
    {
        x->m_buffer[i] = 0.f;
    }
    
    x->m_count = 0;
}

void pa_delay1_tilde_perform64(t_pa_delay1_tilde* x, t_object* dsp64,
                                double** ins, long numins, double** outs, long numouts,
                                long vecsize, long flags, void* userparam)
{
    double* in = ins[0];
    double* out = outs[0];
    
    double* const buffer = x->m_buffer;
    int count = x->m_count;
    double* buffer_playhead = NULL;
    double sample_to_write = 0.f;
    
    while(vecsize--)
    {
        // store current input sample to write in the buffer
        sample_to_write = *in++;
        
        // fetch buffer pointer playhead position
        buffer_playhead = buffer + count;
        
        // we read our buffer.
        *out++ = *buffer_playhead;
        
        // then store incoming sample to the buffer.
        *buffer_playhead = sample_to_write;
        
        // wrap counter between buffer boundaries
        if(++count >= DELAY1_MAX_DELAY) count = 0;
    }
    
    x->m_count = count;
}


void pa_delay1_tilde_dsp64(t_pa_delay1_tilde* x, t_object* dsp64, short* count,
                            double samplerate, long maxvectorsize, long flags)
{
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_delay1_tilde_perform64, 0, NULL);
}

void pa_delay1_tilde_assist(t_pa_delay1_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        strncpy(string_dest, "(signal) input to be delayed", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) Output", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_delay1_tilde_new(void)
{
    t_pa_delay1_tilde* x = (t_pa_delay1_tilde*)object_alloc(this_class);
    
    if(x)
    {
        // reset our buffer.
        pa_delay1_tilde_clear_buffer(x);
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_delay1_tilde_free(t_pa_delay1_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.delay1~", (method)pa_delay1_tilde_new, (method)pa_delay1_tilde_free,
                           sizeof(t_pa_delay1_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_delay1_tilde_assist,    "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_delay1_tilde_dsp64,     "dsp64",    A_CANT,		0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
