/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A signal driven variable delay line.

#include "c74_msp.h"
using namespace c74::max;

#include <stdlib.h> // malloc, calloc, free...

static t_class* this_class = nullptr;

struct t_pa_delay4_tilde
{
    t_pxobject  m_obj;
    
    double*     m_buffer;
    size_t      m_buffersize;
    size_t      m_writer_playhead;
};

void pa_delay4_tilde_delete_buffer(t_pa_delay4_tilde* x)
{
    if(x->m_buffer)
    {
        // free allocated buffer
        free(x->m_buffer);
        x->m_buffer = nullptr;
    }
}

void pa_delay4_tilde_clear_buffer(t_pa_delay4_tilde* x)
{
    int i = 0;
    if(x->m_buffer)
    {
        for(; i < x->m_buffersize; ++i)
        {
            x->m_buffer[i] = 0.f;
        }
    }
}

void pa_delay4_tilde_create_buffer(t_pa_delay4_tilde* x)
{
    pa_delay4_tilde_delete_buffer(x);
    
    // allocate and clear buffer
    x->m_buffer = (double*)calloc(x->m_buffersize, sizeof(double));
}

//! @brief returns a buffer value at a given index position.
//! @details idx will be wrapped between low and high buffer boundaries in a circular way.
double get_buffer_value(t_pa_delay4_tilde* x, int idx)
{
    const size_t buffersize = x->m_buffersize;
    
    // wrap idx between low and high buffer boundaries.
    while(idx < 0) idx += buffersize;
    while(idx >= buffersize) idx -= buffersize;
    
    return x->m_buffer[idx];
}

double linear_interp(double y1, double y2, double delta)
{
    return y1 + delta * (y2 - y1);
}

void pa_delay4_tilde_perform64(t_pa_delay4_tilde* x, t_object* dsp64,
                               double** ins, long numins, double** outs, long numouts,
                               long vecsize, long flags, void* userparam)
{
    double* in1 = ins[0];
    double* in2 = ins[1];
    double* out = outs[0];
    
    double y1, y2, delta;
    double delay_size_samps = 0.f;
    double* buffer = x->m_buffer;
    const size_t buffersize = x->m_buffersize;
    double sample_to_write = 0.f;
    long reader;
    
    while(vecsize--)
    {
        // store current input sample to write it later in the buffer.
        sample_to_write = *in1++;
        
        // get new delay size value.
        delay_size_samps = *in2++;
        
        // clip delay size to buffersize - 1
        if(delay_size_samps >= buffersize)
        {
            delay_size_samps = buffersize - 1;
        }
        else if(delay_size_samps < 1.) // read first implementation : 0 samps delay = max delay
        {
            delay_size_samps = 1.;
        }
        
        // extract the fractional part
        delta = delay_size_samps - (int)delay_size_samps;
        
        reader = x->m_writer_playhead - (int)delay_size_samps;
        
        // Reading our buffer.
        y1 = get_buffer_value(x, reader);
        y2 = get_buffer_value(x, reader - 1);
        
        // without interpolation
        //*out1++ = y1;
        
        // with linear interpolation
        *out++ = linear_interp(y1, y2, delta);
        
        // then store incoming sample to the buffer.
        buffer[x->m_writer_playhead] = sample_to_write;
        
        // increment then wrap counter between buffer boundaries
        if(++x->m_writer_playhead >= x->m_buffersize) x->m_writer_playhead -= x->m_buffersize;
    }
}


void pa_delay4_tilde_dsp64(t_pa_delay4_tilde* x, t_object* dsp64, short* count,
                            double samplerate, long maxvectorsize, long flags)
{
    // as you want :
    //pa_delay4_tilde_clear_buffer(x);
    
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_delay4_tilde_perform64, 0, NULL);
}

void pa_delay4_tilde_assist(t_pa_delay4_tilde* x, void* unused,
                             t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        if(index == 0)
        {
            strncpy(string_dest, "(signal) input to be delayed", ASSIST_STRING_MAXSIZE);
        }
        else
        {
            strncpy(string_dest, "(signal) delay size in samps", ASSIST_STRING_MAXSIZE);
        }
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) Output", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_delay4_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_delay4_tilde* x = (t_pa_delay4_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_buffer = nullptr;
        x->m_writer_playhead = 0;
        
        int buffersize = sys_getsr() * 0.1; // default to 100ms
        
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            if(atom_getlong(argv) > 0)
            {
                buffersize = atom_getlong(argv);
            }
            else
            {
                object_error((t_object*)x, "buffer size must be > 0");
            }
        }
        
        x->m_buffersize = buffersize;
        
        // reset our buffer.
        pa_delay4_tilde_create_buffer(x);
        
        dsp_setup((t_pxobject*)x, 2);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_delay4_tilde_free(t_pa_delay4_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.delay4~", (method)pa_delay4_tilde_new, (method)pa_delay4_tilde_free,
                           sizeof(t_pa_delay4_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_delay4_tilde_assist,         "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_delay4_tilde_dsp64,          "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_delay4_tilde_clear_buffer,   "clear",                0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
