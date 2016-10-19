/*
 // Copyright (c) 2016 Eliott Paris, paccpp, Université Paris 8.
 // For information on usage and redistribution, and for a DISCLAIMER OF ALL
 // WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

// A dummy object that shows how to route/handle Max messages.

// header for max objects
#include "c74_max.h"
using namespace c74::max;

static t_class* this_class = nullptr;

struct t_pa_dummy
{
    t_object    m_obj; // simple max object - always placed in first in the object's struct
    
    t_outlet*   m_out_bang;
    t_outlet*   m_out_int;
    t_outlet*   m_out_float;
    t_outlet*   m_out_sym;
};

void pa_dummy_output_args(t_pa_dummy *x, long argc, t_atom *argv)
{
    for(long i=0; i < argc; i++)
    {
        t_atom* atom_ptr = argv+i;
        
        if(atom_gettype(atom_ptr) == A_LONG)
        {
            t_atom_long l = atom_getlong(atom_ptr);
            outlet_int(x->m_out_int, l);
        }
        else if(atom_gettype(atom_ptr) == A_FLOAT)
        {
            t_atom_float f = atom_getfloat(atom_ptr);
            outlet_float(x->m_out_float, f);
        }
        else if(atom_gettype(atom_ptr) == A_SYM)
        {
            t_symbol* s = atom_getsym(atom_ptr);
            outlet_anything(x->m_out_sym, s, 0, NULL);
        }
    }
}

void pa_dummy_bang(t_pa_dummy *x)
{
    outlet_bang(x->m_out_bang);
}

void pa_dummy_int(t_pa_dummy *x, long l)
{
    outlet_int(x->m_out_int, l);
}

void pa_dummy_float(t_pa_dummy *x, double d)
{
    outlet_float(x->m_out_float, d);
}

void pa_dummy_anything(t_pa_dummy *x, t_symbol* s, long argc, t_atom *argv)
{
    outlet_anything(x->m_out_sym, s, 0, NULL);
    pa_dummy_output_args(x, argc, argv);
}

void pa_dummy_list(t_pa_dummy *x, t_symbol* s, long argc, t_atom *argv)
{
    pa_dummy_output_args(x, argc, argv);
}

void pa_dummy_assist(t_pa_dummy* x, void* unused, t_assist_function io, long index, char* string_dest)
{
    if(io == ASSIST_INLET)
    {
        sprintf(string_dest, "(bang/int/float/symbol) inlet %ld", index);
    }
    else if(io == ASSIST_OUTLET)
    {
        switch (index)
        {
            case 0:
                strncpy(string_dest, "bang", ASSIST_STRING_MAXSIZE);
                break;
                
            case 1:
                strncpy(string_dest, "(int) integer", ASSIST_STRING_MAXSIZE);
                break;
                
            case 2:
                strncpy(string_dest, "(float) floating-point", ASSIST_STRING_MAXSIZE);
                break;
                
            case 3:
                strncpy(string_dest, "(symbol) symbols", ASSIST_STRING_MAXSIZE);
                break;
        }
    }
}

void* pa_dummy_new(t_symbol *name, int argc, t_atom *argv)
{
    t_pa_dummy* x = (t_pa_dummy*)object_alloc(this_class);
    
    if(x)
    {
        for(int i=0; i < argc; i++)
        {
            t_atom* atom_ptr = argv+i;
            
            if(atom_gettype(atom_ptr) == A_LONG)
            {
                t_atom_long l = atom_getlong(atom_ptr);
                object_post((t_object*)x, "arg %i (long) = %l", i, l);
            }
            else if(atom_gettype(atom_ptr) == A_FLOAT)
            {
                t_atom_float f = atom_getfloat(atom_ptr);
                object_post((t_object*)x, "arg %i (float) = %f", i, f);
            }
            else if(atom_gettype(atom_ptr) == A_SYM)
            {
                t_symbol* s = atom_getsym(atom_ptr);
                object_post((t_object*)x, "arg %i (symbol) = %s", i, s->s_name);
            }
        }
        
        x->m_out_sym    = (t_outlet*)outlet_new((t_object*)x, "symbol");
        x->m_out_float  = (t_outlet*)outlet_new((t_object*)x, "float");
        x->m_out_int    = (t_outlet*)outlet_new((t_object*)x, "int");
        x->m_out_bang   = (t_outlet*)outlet_new((t_object*)x, "bang");
    }
    
    return x;
}

void pa_dummy_free(t_pa_dummy* x)
{
    ; // nothing to be done here
}

void ext_main(void* r)
{
    this_class = class_new("pa.dummy", (method)pa_dummy_new, (method)pa_dummy_free,
                           sizeof(t_pa_dummy), 0, A_GIMME, 0);
    
    class_addmethod(this_class, (method)pa_dummy_assist,    "assist",   A_CANT,     0);
    
    class_addmethod(this_class, (method)pa_dummy_bang,      "bang",     0);
    class_addmethod(this_class, (method)pa_dummy_int,       "int",      A_LONG,     0);
    class_addmethod(this_class, (method)pa_dummy_float,     "float",    A_FLOAT,    0);
    class_addmethod(this_class, (method)pa_dummy_anything,  "anything", A_GIMME,    0);
    class_addmethod(this_class, (method)pa_dummy_list,      "list",     A_GIMME,    0);
    
    object_post(NULL, "Hello Dummy"); // missing post() function work around.
    
    class_register(CLASS_BOX, this_class);
}
