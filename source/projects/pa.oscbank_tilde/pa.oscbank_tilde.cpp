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

#include <vector>

static t_class* this_class = nullptr;

struct t_pa_oscbank_tilde
{
    t_pxobject  m_obj;
    
    // store a vector of Osc pointers
    std::vector<Osc<double>*> m_oscbank;
};

void pa_oscbank_tilde_list(t_pa_oscbank_tilde* x, t_symbol* s, int argc, t_atom* argv)
{
    float sr = sys_getsr();
    
    // reserve space for new oscillators in the vector
    x->m_oscbank.reserve(argc);
    
    for(int i = 0; i < argc; ++i)
    {
        // allocate a new Osc object if needed
        if(x->m_oscbank.size() <= i)
        {
            x->m_oscbank.push_back(new Osc<double>());
            x->m_oscbank[i]->setSampleRate(sr);
        }
        
        if(atom_gettype(argv+i) == A_FLOAT || atom_gettype(argv+i) == A_LONG)
        {
            x->m_oscbank[i]->setFrequency(atom_getfloat(argv+i));
        }
        else
        {
            x->m_oscbank[i]->setFrequency(0.);
            object_error((t_object*)x, "bad frequency for osc %i, reset to 0Hz", i);
        }
    }
    
    // delete all useless pointers and remove them from the vector
    if(x->m_oscbank.size() > argc)
    {
        for(int i = argc; i < x->m_oscbank.size(); ++i)
        {
            delete x->m_oscbank[i];
        }
        
        x->m_oscbank.erase(x->m_oscbank.begin()+argc, x->m_oscbank.end());
    }
}

void pa_oscbank_tilde_perform64(t_pa_oscbank_tilde* x, t_object* dsp64,
                                       double** ins, long numins, double** outs, long numouts,
                                       long vecsize, long flags, void* userparam)
{
    double* outputs = outs[0];
    const size_t osc_count = (!x->m_oscbank.empty()) ? x->m_oscbank.size() : 1ul;
    
    while(vecsize--)
    {
        double out = 0.f;
        
        for(Osc<double>* osc : x->m_oscbank)
        {
            out += osc->process();
        }
        
        *outputs++ = out / osc_count;
    }
}


void pa_oscbank_tilde_dsp64(t_pa_oscbank_tilde* x, t_object* dsp64, short* count,
                             double samplerate, long maxvectorsize, long flags)
{
    // set samplerate of all oscillators
    const float sr = sys_getsr();
    
    for(Osc<double>* osc : x->m_oscbank)
    {
        osc->setSampleRate(sr);
    }
    
    object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
                         dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)pa_oscbank_tilde_perform64, 0, NULL);
}

void pa_oscbank_tilde_assist(t_pa_oscbank_tilde* x, void* unused,
                              t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        strncpy(string_dest, "(list) list of floats sets osc frequencies", ASSIST_STRING_MAXSIZE);
    }
    else if(io == ASSIST_OUTLET)
    {
        strncpy(string_dest, "(signal) Output", ASSIST_STRING_MAXSIZE);
    }
}

void* pa_oscbank_tilde_new(t_symbol *name, long argc, t_atom *argv)
{
    t_pa_oscbank_tilde* x = (t_pa_oscbank_tilde*)object_alloc(this_class);
    
    if(x)
    {
        outlet_new(x, "signal");
    }
    
    return x;
}

void pa_oscbank_tilde_free(t_pa_oscbank_tilde* x)
{
    dsp_free((t_pxobject*)x);
    
    // free the memory allocated for each Osc objects of the vector
    for(Osc<double>* osc : x->m_oscbank)
    {
        delete osc;
    }
    
    // then clear the vector
    x->m_oscbank.clear();
}

void ext_main(void* r)
{
    this_class = class_new("pa.oscbank~", (method)pa_oscbank_tilde_new, (method)pa_oscbank_tilde_free,
                           sizeof(t_pa_oscbank_tilde), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_oscbank_tilde_assist,     "assist",   A_CANT,		0);
    class_addmethod(this_class, (method)pa_oscbank_tilde_dsp64,      "dsp64",    A_CANT,		0);
    class_addmethod(this_class, (method)pa_oscbank_tilde_list,       "list",     A_GIMME,       0);
    
    class_dspinit(this_class);
    class_register(CLASS_BOX, this_class);
}
