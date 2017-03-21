/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Cosine wave oscillator
//! @details this object is c++ version of the [pa.osc3~] object

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

#include "Osc.hpp"
using paccpp::Osc;

static t_class* this_class = nullptr;

struct t_pa_oscpp_tilde
{
    t_pxobject  m_obj;
    
    // store a Phasor pointer
    Osc<double>* m_osc;
};

void pa_oscpp_tilde_float(t_pa_oscpp_tilde* x, double d)
{
    x->m_osc->setFrequency(d);
}

void pa_oscpp_tilde_int(t_pa_oscpp_tilde* x, long l)
{
    pa_oscpp_tilde_float(x, (double)l);
}

void pa_oscpp_tilde_perform64_vec(t_pa_oscpp_tilde* x, t_object* dsp64,
                                     double** ins, long numins, double** outs, long numouts,
                                     long vecsize, long flags, void* userparam)
{
    double const* in = ins[0];
    double* out = outs[0];
    
    x->m_osc->process(in, out, vecsize);
}

void pa_oscpp_tilde_perform64_float(t_pa_oscpp_tilde* x, t_object* dsp64,
                                       double** ins, long numins, double** outs, long numouts,
                                       long vecsize, long flags, void* userparam)
{
    double* out = outs[0];
    
    while(vecsize--)
    {
        *out++ = x->m_osc->process();
    }
}


void pa_oscpp_tilde_dsp64(t_pa_oscpp_tilde* x, t_object* dsp64, short* count,
                             double samplerate, long maxvectorsize, long flags)
{
    x->m_osc->setSampleRate(sys_getsr());
    
    if(count[0])
    {
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_oscpp_tilde_perform64_vec, 0, NULL);
    }
    else
    {
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_oscpp_tilde_perform64_float, 0, NULL);
    }
}

void pa_oscpp_tilde_assist(t_pa_oscpp_tilde* x, void* unused,
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

void* pa_oscpp_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_oscpp_tilde* x = (t_pa_oscpp_tilde*)object_alloc(this_class);
    
    if(x)
    {
        // instantiate a new Osc object
        // Note: dont forget to delete it in the free method !
        x->m_osc = new Osc<double>();
        
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            // set frequency
            pa_oscpp_tilde_float(x, atom_getfloat(argv));
        }
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_oscpp_tilde_free(t_pa_oscpp_tilde* x)
{
    dsp_free((t_pxobject*)x);
    
    // free the memory for the Osc object
    delete x->m_osc;
}

void ext_main(void* r)
{
    this_class = class_new("pa.oscpp~", (method)pa_oscpp_tilde_new, (method)pa_oscpp_tilde_free,
                           sizeof(t_pa_oscpp_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_oscpp_tilde_assist,     "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_oscpp_tilde_dsp64,      "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_oscpp_tilde_float,      "float",    A_FLOAT,    0);
    class_addmethod(this_class, (method)pa_oscpp_tilde_int,        "int",      A_LONG,     0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
