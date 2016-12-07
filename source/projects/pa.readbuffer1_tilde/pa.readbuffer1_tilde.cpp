/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Access a Max buffer~

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_readbuffer1_tilde
{
    t_pxobject      m_obj;
    t_buffer_ref*   m_buffer_reference;
};

void pa_readbuffer1_dsp_perform(t_pa_readbuffer1_tilde *x, t_object *dsp64,
                                double **ins, long numins, double **outs, long numouts,
                                long sampleframes, long flags, void *userparam)
{
    double *in = ins[0];
    double *out = outs[0];
    int n = sampleframes;
    float *tab;
    long index, size, nc;
    
    t_buffer_obj* buffer = buffer_ref_getobject(x->m_buffer_reference);
    if(buffer)
    {
        tab = buffer_locksamples(buffer);
        if(tab)
        {
            size = buffer_getframecount(buffer);
            nc = buffer_getchannelcount(buffer);
            
            while (n--)
            {
                index = (long)(*in++ * size);
                
                while(index < 0) { index += size; }
                while(index >= size) { index -= size; }
                
                if(nc > 1) { index *= nc; }
                
                *out++ = tab[index];
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
}

void pa_readbuffer1_set(t_pa_readbuffer1_tilde *x, t_symbol *s)
{
    if (!x->m_buffer_reference)
        x->m_buffer_reference = buffer_ref_new((t_object *)x, s);
    else
        buffer_ref_set(x->m_buffer_reference, s);
}

void pa_readbuffer1_dsp_prepare(t_pa_readbuffer1_tilde *x, t_object *dsp64,
                                short *count, double samplerate, long maxvectorsize, long flags)
{
    dsp_add64(dsp64, (t_object *)x, (t_perfroutine64)pa_readbuffer1_dsp_perform, 0, NULL);
}

void pa_readbuffer1_assist(t_pa_readbuffer1_tilde *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_OUTLET)
        sprintf(s,"(signal) Phase between 0. and 1.");
    else
        sprintf(s,"(signal) Sample");
}

t_max_err pa_readbuffer1_notify(t_pa_readbuffer1_tilde *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    return buffer_ref_notify(x->m_buffer_reference, s, msg, sender, data);
}

void *pa_readbuffer1_new(t_symbol *s)
{
    t_pa_readbuffer1_tilde* x = (t_pa_readbuffer1_tilde*)object_alloc(this_class);
    
    if(x)
    {
        dsp_setup((t_pxobject *)x, 1);
        outlet_new((t_object *)x, "signal");
        
        pa_readbuffer1_set(x, s);
    }
    
    return (x);
}


void pa_readbuffer1_free(t_pa_readbuffer1_tilde *x)
{
    dsp_free((t_pxobject *)x);
    object_free(x->m_buffer_reference);
}

void ext_main(void *r)
{
    t_class *c = class_new("pa.readbuffer1~", (method)pa_readbuffer1_new, (method)pa_readbuffer1_free,
                           sizeof(t_pa_readbuffer1_tilde), 0L, A_SYM, 0);
    
    class_addmethod(c, (method)pa_readbuffer1_dsp_prepare,   "dsp64",    A_CANT,     0);
    class_addmethod(c, (method)pa_readbuffer1_set,           "set",      A_SYM,      0);
    class_addmethod(c, (method)pa_readbuffer1_assist,        "assist",   A_CANT,     0);
    class_addmethod(c, (method)pa_readbuffer1_notify,        "notify",   A_CANT,     0);
    
    class_dspinit(c);
    class_register(CLASS_BOX, c);
    
    this_class = c;
}
