/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#include "c74_msp.h"

using namespace c74::max;

static t_class* this_class = nullptr;

struct t_dummy_tilde
{
	t_pxobject obj;
};

void* dummy_tilde_new(void)
{
	t_dummy_tilde* x = (t_dummy_tilde*)object_alloc(this_class);
	
	dsp_setup((t_pxobject*)x, 2);
	outlet_new(x, "signal");
	return x;
}

void dummy_tilde_free(t_dummy_tilde* x)
{
	dsp_free((t_pxobject*)x);
}

void dummy_tilde_perform64(t_dummy_tilde* x, t_object* dsp64,
                           double** ins, long numins, double** outs, long numouts,
                           long sampleframes, long flags, void* userparam)
{
    double* in1 = ins[0];
    double* in2 = ins[1];
    double* out = outs[0];
    
    int i;
    for(i = 0; i < sampleframes; ++i)
    {
        *out++ = *in1++ + *in2++;
    }
}


void dummy_tilde_dsp64(t_dummy_tilde* x, t_object* dsp64, short* count,
                       double samplerate, long maxvectorsize, long flags)
{
	object_method_direct(void, (t_object*, t_object*, t_perfroutine64, long, void*),
						 dsp64, gensym("dsp_add64"), (t_object*)x,
                         (t_perfroutine64)dummy_tilde_perform64, 0, NULL);
}

void dummy_tilde_assist(t_dummy_tilde* x, void* unused, t_assist_function io, long index, char* string_dest)
{
	if(io == ASSIST_INLET)
    {
		switch(index)
        {
			case 0: 
				strncpy(string_dest, "(signal) lhs operand", ASSIST_STRING_MAXSIZE);
				break;
            case 1:
                strncpy(string_dest, "(signal) rhs operand", ASSIST_STRING_MAXSIZE);
                break;
		}
	}
	else if(io == ASSIST_OUTLET)
    {
		switch (index)
        {
			case 0: 
				strncpy(string_dest, "(signal) result", ASSIST_STRING_MAXSIZE);
				break;
		}
	}
}


void ext_main(void* r)
{
	this_class = class_new("dummy~", (method)dummy_tilde_new, (method)dummy_tilde_free,
                           sizeof(t_dummy_tilde), 0, A_GIMME, 0);

	class_addmethod(this_class, (method)dummy_tilde_assist,	"assist",	A_CANT,		0);
	class_addmethod(this_class, (method)dummy_tilde_dsp64,	"dsp64",	A_CANT,		0);
	
	class_dspinit(this_class);
	class_register(CLASS_BOX, this_class);
}
