/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A fixed delay (initialized with first argument using dynamic allocation)

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

#include <stdlib.h> // malloc, calloc, free...

static t_class* this_class = nullptr;

struct t_pa_delay2_tilde
{
    t_pxobject  m_obj;
    
    double*     m_buffer;
    int         m_buffersize;
    int         m_count;
};

void pa_delay2_tilde_delete_buffer(t_pa_delay2_tilde* x)
{
    if(x->m_buffer)
    {
        // free allocated buffer
        free(x->m_buffer);
        x->m_buffer = NULL;
    }
}

void pa_delay2_tilde_clear_buffer(t_pa_delay2_tilde* x)
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

void pa_delay2_tilde_create_buffer(t_pa_delay2_tilde* x)
{
    pa_delay2_tilde_delete_buffer(x);
    
    // allocate buffer
    x->m_buffer = (double*)malloc(sizeof(double) * x->m_buffersize);
    
    if(x->m_buffer)
    {
        pa_delay2_tilde_clear_buffer(x);
    }
    
    // or alloc and init to 0 directly with calloc
    //x->m_buffer = (double*)calloc(x->m_buffersize, sizeof(double));
}

void pa_delay2_tilde_perform64(t_pa_delay2_tilde* x, t_object* dsp64,
                                double** ins, long numins, double** outs, long numouts,
                                long vecsize, long flags, void* userparam)
{
    double* in = ins[0];
    double* out = outs[0];
    
    double* buffer = x->m_buffer;
    int count = x->m_count;
    double* buffer_playhead = NULL;
    double sample_to_write = 0.f;
    
    int buffersize = x->m_buffersize;
    
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
        if(++count >= buffersize) count = 0;
    }
    
    x->m_count = count;
}


void pa_delay2_tilde_dsp64(t_pa_delay2_tilde* x, t_object* dsp64, short* count,
                            double samplerate, long maxvectorsize, long flags)
{
    // as you want :
    //pa_delay2_tilde_clear_buffer(x);
    //x->m_count = 0;
    
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_delay2_tilde_perform64, 0, NULL);
}

void pa_delay2_tilde_assist(t_pa_delay2_tilde* x, void* unused,
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

void* pa_delay2_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_delay2_tilde* x = (t_pa_delay2_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_buffer = nullptr;
        x->m_count = 0;
        
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
        pa_delay2_tilde_create_buffer(x);
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_delay2_tilde_free(t_pa_delay2_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.delay2~", (method)pa_delay2_tilde_new, (method)pa_delay2_tilde_free,
                           sizeof(t_pa_delay2_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_delay2_tilde_assist,    "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_delay2_tilde_dsp64,     "dsp64",    A_CANT,		0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
