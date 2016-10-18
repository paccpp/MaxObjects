/*
 // Copyright (c) 2016 Eliott Paris.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

// A starter for Max objects

// header for max objects
#include "c74_max.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_starter
{
	t_object m_obj; // simple max object - always placed in first in the object's struct
};

void* pa_starter_new(t_symbol *name, int argc, t_atom *argv)
{
    t_pa_starter* x = (t_pa_starter*)object_alloc(this_class);
    
    if(x)
    {
        ;
    }
    
    return x;
}

void pa_starter_free(t_pa_starter* x)
{
    ;
}

void ext_main(void* r)
{
	this_class = class_new("pa.starter", (method)pa_starter_new, (method)pa_starter_free,
                           sizeof(t_pa_starter), 0, A_GIMME, 0);
	
	class_register(CLASS_BOX, this_class);
}
