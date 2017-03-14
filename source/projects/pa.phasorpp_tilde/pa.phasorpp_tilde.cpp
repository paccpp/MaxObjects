/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

//! @brief Outputs a ramp between 0. and 1. at a given frequency

// header for msp objects
#include "c74_msp.h"
using namespace c74::max;

#include "Phasor.hpp"
using paccpp::Phasor;

static t_class* this_class = nullptr;

struct t_pa_phasorpp_tilde
{
    t_pxobject  m_obj;
    
    // store a Phasor pointer
    Phasor<double>* m_phasor;
};

void pa_phasorpp_tilde_float(t_pa_phasorpp_tilde* x, double d)
{
    x->m_phasor->setFrequency(d);
}

void pa_phasorpp_tilde_int(t_pa_phasorpp_tilde* x, long l)
{
    pa_phasorpp_tilde_float(x, (double)l);
}

void pa_phasorpp_tilde_perform64_vec(t_pa_phasorpp_tilde* x, t_object* dsp64,
                                   double** ins, long numins, double** outs, long numouts,
                                   long vecsize, long flags, void* userparam)
{
    double const* in = ins[0];
    double* out = outs[0];
    
    x->m_phasor->process(in, out, vecsize);
}

void pa_phasorpp_tilde_perform64_float(t_pa_phasorpp_tilde* x, t_object* dsp64,
                                     double** ins, long numins, double** outs, long numouts,
                                     long vecsize, long flags, void* userparam)
{
    double* out = outs[0];
    
    while(vecsize--)
    {
        *out++ = x->m_phasor->process();
    }
}


void pa_phasorpp_tilde_dsp64(t_pa_phasorpp_tilde* x, t_object* dsp64, short* count,
                           double samplerate, long maxvectorsize, long flags)
{
    x->m_phasor->setSampleRate(sys_getsr());
    
    if(count[0])
    {
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_phasorpp_tilde_perform64_vec, 0, NULL);
    }
    else
    {
        object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                             dsp64, gensym("dsp_add64"), (t_object*)x,
                             (t_perfroutine64)pa_phasorpp_tilde_perform64_float, 0, NULL);
    }
}

void pa_phasorpp_tilde_assist(t_pa_phasorpp_tilde* x, void* unused,
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

void* pa_phasorpp_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_phasorpp_tilde* x = (t_pa_phasorpp_tilde*)object_alloc(this_class);
    
    if(x)
    {
        // instantiate a new Phasor object
        // Note: dont forget to delete it in the free method !
        x->m_phasor = new Phasor<double>();
        
        if(argc >= 1 && (atom_gettype(argv) == A_FLOAT || atom_gettype(argv) == A_LONG))
        {
            // set frequency
            pa_phasorpp_tilde_float(x, atom_getfloat(argv));
        }
        
        dsp_setup((t_pxobject*)x, 1);
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_phasorpp_tilde_free(t_pa_phasorpp_tilde* x)
{
    dsp_free((t_pxobject*)x);
    
    // free the memory for the Phasor object
    delete x->m_phasor;
}

void ext_main(void* r)
{
    this_class = class_new("pa.phasorpp~", (method)pa_phasorpp_tilde_new, (method)pa_phasorpp_tilde_free,
                           sizeof(t_pa_phasorpp_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_phasorpp_tilde_assist,     "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_phasorpp_tilde_dsp64,      "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_phasorpp_tilde_float,      "float",    A_FLOAT,    0);
    class_addmethod(this_class, (method)pa_phasorpp_tilde_int,        "int",      A_LONG,     0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
