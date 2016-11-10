/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief A sinusoidal oscillator using linear interpolation on a 512+1 point buffer

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

#include <cmath> // cos...

static t_class* this_class = nullptr;

#define OSC3_COSTABLE_SIZE 512
static double osc3_cos_table[OSC3_COSTABLE_SIZE+1]; // size + 1 additional sample to facilitate linear interpolation

struct t_pa_osc3_tilde
{
    t_pxobject  m_obj;
    
    double      m_sr;
    double      m_phase;
    
    // to perform with float freq
    double      m_freq;
    double      m_phase_inc;
};

void pa_osc3_tilde_fill_cos_table()
{
    int i;
    for(i=0; i < OSC3_COSTABLE_SIZE; i++)
    {
        osc3_cos_table[i] = cos(2.f * M_PI * i / OSC3_COSTABLE_SIZE);
        //post("%f, ", osc3_cos_table[i]);
    }
    
    osc3_cos_table[OSC3_COSTABLE_SIZE] = osc3_cos_table[0]; // last sample = first sample
}

void pa_osc3_tilde_reset_phase(t_pa_osc3_tilde *x, double d)
{
    const int tsize = (OSC3_COSTABLE_SIZE-1);
    float phase = d * tsize;
    
    // wrap between table boundaries
    while(phase >= tsize) { phase -= tsize; }
    while(phase < 0) { phase += tsize; }
    
    x->m_phase = phase;
}

void pa_osc3_tilde_float(t_pa_osc3_tilde* x, double d)
{
    x->m_freq = d;
    x->m_phase_inc = (x->m_freq / x->m_sr) * (OSC3_COSTABLE_SIZE-1);
}

void pa_osc3_tilde_int(t_pa_osc3_tilde* x, long l)
{
    pa_osc3_tilde_float(x, (double)l);
}

void pa_osc3_tilde_perform64_vec(t_pa_osc3_tilde* x, t_object* dsp64,
                                 double** ins, long numins, double** outs, long numouts,
                                 long vecsize, long flags, void* userparam)
{
    double* in = ins[0];
    double* out = outs[0];
    
    // interpolation index
    int idx_1;
    
    const int tsize = (OSC3_COSTABLE_SIZE-1);
    
    const double sr = x->m_sr;
    
    // interpolation values
    double y1, delta;
    double *cos_table = osc3_cos_table;
    double freq;
    double tphase = x->m_phase; // phase in 0. to OSC3_COSTABLE_SIZE. range
    
    while(vecsize--)
    {
        freq = *in++;
        
        // wrap between table boundaries
        if(tphase >= tsize) { tphase -= tsize; }
        else if(tphase < 0) { tphase += tsize; }
        
        // we cast to int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
        idx_1 = (int)tphase;
        
        // delta = tphase - integral part of the the floating-point number
        delta = tphase - idx_1;
        
        y1 = cos_table[idx_1];
        
        // linear interpolation
        // (idx2 can always be idx_1+1 thanks to the additional sample in the table)
        *out++ = y1 + delta * (cos_table[idx_1+1] - y1);
        
        // increment phase
        tphase += (freq / sr) * tsize;
    }
    
    x->m_phase = tphase;
}

void pa_osc3_tilde_perform64_float(t_pa_osc3_tilde* x, t_object* dsp64,
                                   double** ins, long numins, double** outs, long numouts,
                                   long vecsize, long flags, void* userparam)
{
    double* out = outs[0];
    
    /// interpolation index
    int idx_1;
    
    const int tsize = (OSC3_COSTABLE_SIZE-1);
    
    // interpolation values
    double y1, delta;
    double *cos_table = osc3_cos_table;
    double tphase = x->m_phase; // phase in 0. to OSC3_COSTABLE_SIZE. range
    const double phase_inc = x->m_phase_inc;
    
    while(vecsize--)
    {
        // wrap between table boundaries
        if(tphase >= tsize) { tphase -= tsize; }
        else if(tphase < 0) { tphase += tsize; }
        
        // we cast to int to keep only the integer part of the floating-point number (eg. 3.99 => 3)
        idx_1 = (int)tphase;
        
        // delta = tphase - integral part of the the floating-point number
        delta = tphase - idx_1;
        
        y1 = cos_table[idx_1];
        
        // linear interpolation
        // (idx_2 can allways be idx_1+1 thanks to the additionnal sample in the buffer)
        *out++ = y1 + delta * (cos_table[idx_1+1] - y1);
        
        // increment phase
        tphase += phase_inc;
    }
    
    x->m_phase = tphase;
}


void pa_osc3_tilde_dsp64(t_pa_osc3_tilde* x, t_object* dsp64, short* count,
                         double samplerate, long maxvectorsize, long flags)
{
    x->m_sr = sys_getsr();
    
    // You can reset the phase here
    //x->m_phase = 0.f;
    
    if(count[0])
    {
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_osc3_tilde_perform64_vec, 0, NULL);
    }
    else
    {
        pa_osc3_tilde_float(x, x->m_freq);
        
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_osc3_tilde_perform64_float, 0, NULL);
    }
}

void pa_osc3_tilde_assist(t_pa_osc3_tilde* x, void* unused,
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

void* pa_osc3_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_osc3_tilde* x = (t_pa_osc3_tilde*)object_alloc(this_class);
    
    if(x)
    {
        x->m_phase = 0.;
        x->m_freq = 0.;
        x->m_phase_inc = 0.;
        
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            // set freq and phase_inc
            pa_osc3_tilde_float(x, atom_getfloat(argv));
        }
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_osc3_tilde_free(t_pa_osc3_tilde* x)
{
    dsp_free((t_pxobject*)x);
}

void ext_main(void* r)
{
    this_class = class_new("pa.osc3~", (method)pa_osc3_tilde_new, (method)pa_osc3_tilde_free,
                           sizeof(t_pa_osc3_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_osc3_tilde_assist,     "assist",   A_CANT,       0);
    class_addmethod(this_class, (method)pa_osc3_tilde_dsp64,      "dsp64",    A_CANT,       0);
    class_addmethod(this_class, (method)pa_osc3_tilde_float,      "float",    A_FLOAT,      0);
    class_addmethod(this_class, (method)pa_osc3_tilde_int,        "int",      A_LONG,       0);
    
    pa_osc3_tilde_fill_cos_table();
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
