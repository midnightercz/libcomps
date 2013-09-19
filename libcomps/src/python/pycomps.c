/* libcomps - C alternative to yum.comps library
 * Copyright (C) 2013 Jindrich Luza
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to  Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA
 */

#include "pycomps_23macros.h"
#include "pycomps.h"

#if PY_MAJOR_VERSION >= 3
    #define MODINIT_RET_NONE return NULL
    #define PY_OBJ_HEAD_INIT PyVarObject_HEAD_INIT(NULL, 0)
    #define IS_PY3K
#else
    #define MODINIT_RET_NONE return
    #define PY_OBJ_HEAD_INIT PyObject_HEAD_INIT(NULL)\
                             0,
#endif

inline PyCOMPS_GetSetClosure * get_closure(void * closure) {
    return ((PyCOMPS_GetSetClosure*)closure);
}

PyObject* PyCOMPS_toxml_f(PyObject *self, PyObject *other) {
    const char *errors = NULL;
    char *tmps;
    int i;
    COMPS_ListItem *it;
    PyObject *ret, *tmp;
    PyCOMPS *self_comps = (PyCOMPS*)self;

    if (__pycomps_arg_to_char(other, &tmps)) return NULL;
    if (!self_comps->comps_doc->encoding)
       self_comps->comps_doc->encoding = comps_str("UTF-8");
    comps2xml_f(self_comps->comps_doc, tmps, 0);
    free(tmps);

    ret = PyList_New(self_comps->comps_doc->log->logger_data->len);
    for (i = 0, it = self_comps->comps_doc->log->logger_data->first;
         it != NULL; it = it->next, i++) {
        tmps = comps_log_entry_str(it->data);
        tmp = PyUnicode_DecodeUTF8(tmps, strlen(tmps), errors);
        PyList_SetItem(ret, i, tmp);
        free(tmps);
    }
    return ret;
}

PyObject* PyCOMPS_toxml_str(PyObject *self) {
    PyObject *ret;
    const char *errors = NULL;

    char *s = comps2xml_str(((PyCOMPS*)self)->comps_doc);
    ret = PyUnicode_DecodeUTF8(s, strlen(s), errors);
    free(s);
    return ret;
}

void pycomps_clear(PyObject *self) {
    COMPS_OBJECT_DESTROY(((PyCOMPS*)self)->comps_doc);
}

PyObject* PyCOMPS_clear(PyObject *self) {

    COMPS_Str *enc;
    enc = (COMPS_Str*)
    comps_object_incref((COMPS_Object*)((PyCOMPS*)self)->comps_doc->encoding);

    COMPS_OBJECT_DESTROY(((PyCOMPS*)self)->comps_doc);
    ((PyCOMPS*)self)->comps_doc = (COMPS_Doc*)
                                  comps_object_create(&COMPS_Doc_ObjInfo, NULL);
    ((PyCOMPS*)self)->comps_doc->encoding = enc;
    Py_RETURN_NONE;
}

PyObject* PyCOMPS_fromxml_f(PyObject *self, PyObject *other) {
    FILE *f;
    COMPS_Parsed *parsed;
    char *fname;
    signed char parsed_ret;
    PyCOMPS *self_comps = (PyCOMPS*)self;

    if (__pycomps_arg_to_char(other, &fname)) return NULL;

    parsed = comps_parse_parsed_create();
    comps_parse_parsed_init(parsed, "UTF-8", 0);
    f =  fopen(fname, "r");
    if (!f) {
        PyErr_Format(PyExc_IOError, "Cannot open %s for reading", fname);
        free(fname);
        comps_parse_parsed_destroy(parsed);
        return NULL;
    }
    parsed_ret = comps_parse_file(parsed, f);
    COMPS_OBJECT_DESTROY(self_comps->comps_doc);
    if (parsed->comps_doc) {
        self_comps->comps_doc = parsed->comps_doc;
        self_comps->comps_doc->encoding = parsed->comps_doc->encoding;
        parsed->comps_doc->encoding = NULL;
    } else {
        self_comps->comps_doc = (COMPS_Doc*)comps_object_create(&COMPS_Doc_ObjInfo,
                        (COMPS_Object*[]){(COMPS_Object*)comps_str_x("UTF-8")});
    }
    comps_log_destroy(self_comps->comps_doc->log);
    self_comps->comps_doc->log = parsed->log;
    parsed->log = NULL;

    free(fname);
    parsed->comps_doc = NULL;
    comps_parse_parsed_destroy(parsed);

    return PyINT_FROM_LONG((long)parsed_ret);
}

PyObject* PyCOMPS_get_last_errors(PyObject *self, void *closure)
{
    PyObject *ret;
    COMPS_ListItem *it;
    char *tmps;
    PyObject *tmp;
    const char *errors = NULL;

    (void)closure;

    ret = PyList_New(0);
    for (it = ((PyCOMPS*)self)->comps_doc->log->logger_data->first;
         it != NULL; it = it->next) {
        if (((COMPS_LoggerEntry*)it->data)->type == COMPS_LOG_ERROR) {
            tmps = comps_log_entry_str(it->data);
            tmp = PyUnicode_DecodeUTF8(tmps, strlen(tmps), errors);
            PyList_Append(ret, tmp);
            Py_DECREF(tmp);
            free(tmps);
        }
    }
    return ret;
}
PyObject* PyCOMPS_get_last_log(PyObject *self, void *closure)
{
    PyObject *ret;
    COMPS_ListItem *it;
    char *tmps;
    PyObject *tmp;
    const char *errors = NULL;

    (void)closure;

    ret = PyList_New(0);
    for (it = ((PyCOMPS*)self)->comps_doc->log->logger_data->first;
         it != NULL; it = it->next) {
        tmps = comps_log_entry_str(it->data);
        tmp = PyUnicode_DecodeUTF8(tmps, strlen(tmps), errors);
        PyList_Append(ret, tmp);
        Py_DECREF(tmp);
        free(tmps);
    }
    return ret;
}

PyObject* PyCOMPS_fromxml_str(PyObject *self, PyObject *other) {
    char *tmps;
    signed char parsed_ret;
    PyCOMPS *self_comps = (PyCOMPS*)self;

    if (__pycomps_arg_to_char(other, &tmps)) return NULL;

    COMPS_Parsed *parsed;
    parsed = comps_parse_parsed_create();
    comps_parse_parsed_init(parsed, "UTF-8", 0);
    parsed_ret = comps_parse_str(parsed, tmps);
    free(tmps);

    //pycomps_clear(self);
    //pycomps_doc_destroy((void*)self_comps->comps);
    COMPS_OBJECT_DESTROY(self_comps->comps_doc);
    self_comps->comps_doc = parsed->comps_doc;
    comps_log_destroy(self_comps->comps_doc->log);
    self_comps->comps_doc->log = parsed->log;
    parsed->log = NULL;
    parsed->comps_doc = NULL;
    comps_parse_parsed_destroy(parsed);

    return PyINT_FROM_LONG((long)parsed_ret);
}

PyObject* PyCOMPS_get_(PyCOMPS *self, void *closure) {
    PyObject *ret;

    if (!(PyObject*)GET_FROM(self, get_closure(closure)->pobj_offset)) {
        ret = PyCOMPSSeq_new(get_closure(closure)->type, NULL, NULL);
        Py_TYPE(ret)->tp_init(ret, NULL, NULL);
        //TODO
        SET_TO(self, get_closure(closure)->pobj_offset, ret)
    } else {
        ret = (PyObject*)GET_FROM(self, get_closure(closure)->pobj_offset);
    }
    Py_INCREF(ret);
    return ret;
}

int PyCOMPS_set_(PyCOMPS *self, PyObject *val, void *closure) {

    (void)closure;

    if (Py_TYPE(val) != get_closure(closure)->type) {
        PyErr_Format(PyExc_TypeError, "Not %s instance",
                     get_closure(closure)->type->tp_name);
        return -1;
    }
    if ((PyObject*)GET_FROM(self, get_closure(closure)->pobj_offset)){
        Py_DECREF((PyObject*)GET_FROM(self, get_closure(closure)->pobj_offset));
        SET_TO(self, get_closure(closure)->pobj_offset, NULL);
    }
    //TODO
    SET_TO(self, get_closure(closure)->pobj_offset, val);
    Py_INCREF(val);
    return 0;
}

PyCOMPS_GetSetClosure envs_closure = {&PyCOMPS_EnvsType,
                                      offsetof(PyCOMPS, p_environments),
                                      &comps_doc_environments,
                                      &comps_doc_set_environments};
PyCOMPS_GetSetClosure groups_closure = {&PyCOMPS_GroupsType,
                                        offsetof(PyCOMPS, p_groups),
                                        &comps_doc_groups,
                                        &comps_doc_set_groups};
PyCOMPS_GetSetClosure cats_closure = {&PyCOMPS_CatsType,
                                      offsetof(PyCOMPS, p_categories),
                                      &comps_doc_categories,
                                      &comps_doc_set_categories};
PyGetSetDef PyCOMPS_getset[] = {
    {"categories",
     (getter)PyCOMPS_get_, (setter)PyCOMPS_set_,
     "COMPS list of categories", &cats_closure},
    {"groups",
     (getter)PyCOMPS_get_, (setter)PyCOMPS_set_,
     "COMPS list of groups", &groups_closure},
    {"environments",
     (getter)PyCOMPS_get_, (setter)PyCOMPS_set_,
     "COMPS list of environments", &envs_closure},
    {NULL}  /* Sentinel */
};

static PyMemberDef PyCOMPS_members[] = {
    {NULL}};

static PyMethodDef PyCOMPS_methods[] = {
    {"xml_f", (PyCFunction)PyCOMPS_toxml_f, METH_O,
    "write XML represenstation of COMPS to file"},
    {"xml_str", (PyCFunction)PyCOMPS_toxml_str, METH_NOARGS,
    "return XML represenstation of COMPS as string"},
    {"fromxml_f", (PyCFunction)PyCOMPS_fromxml_f, METH_O,
    "Load COMPS from xml file"},
    {"fromxml_str", (PyCFunction)PyCOMPS_fromxml_str, METH_O,
    "Load COMPS from xml string"},
    {"clear", (PyCFunction)PyCOMPS_clear, METH_NOARGS,
    "Clear COMPS"},
    {"get_last_errors", (PyCFunction)PyCOMPS_get_last_errors,
     METH_NOARGS,"return list of messages from log of last parse action."
                 "Containg errors only"},
    {"get_last_log", (PyCFunction)PyCOMPS_get_last_log,
     METH_NOARGS,"return list of messages from log of last parse action."},
    {NULL}  /* Sentinel */
};

static void PyCOMPS_dealloc(PyCOMPS* self)
{
    Py_XDECREF(self->p_groups);
    Py_XDECREF(self->p_categories);
    Py_XDECREF(self->p_environments);
    COMPS_OBJECT_DESTROY(self->comps_doc);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* PyCOMPS_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    (void) args;
    (void) kwds;
    PyCOMPS *self;
    self = (PyCOMPS*) type->tp_alloc(type, 0);
    if (self != NULL) {
        self->comps_doc = (COMPS_Doc*)comps_object_create(&COMPS_Doc_ObjInfo,
                                                          NULL);
        self->p_groups = NULL;
        self->p_categories = NULL;
        self->p_environments = NULL;
    }
    return (PyObject*) self;
}

static int PyCOMPS_init(PyCOMPS *self, PyObject *args, PyObject *kwds)
{
    char *enc = "UTF-8";
    PyObject *c_caps = NULL; /* ignored here */
    (void)kwds;
    if (!args && !kwds) {
    }
    else if (!PyArg_ParseTuple(args, "|sO!", &enc, &PyCapsule_Type, &c_caps))
        return -1;
    self->comps_doc->encoding = comps_str(enc);
    return 0;
}

static PyObject* PyCOMPS_union(PyObject *self, PyObject *other) {
    PyCOMPS *res;

    if (Py_TYPE(other) != &PyCOMPS_Type) {
        PyErr_SetString(PyExc_TypeError, "Not COMPS instance");
        return NULL;
    }

    PyCOMPS *self_t = (PyCOMPS *)self;
    PyCOMPS *other_t = (PyCOMPS *)other;

    COMPS_Doc *un_comps = comps_doc_union(self_t->comps_doc,
                                          other_t->comps_doc);
    res = (PyCOMPS*)PyCOMPS_new(&PyCOMPS_Type, NULL, NULL);
    COMPS_OBJECT_DESTROY(res->comps_doc);
    res->comps_doc = un_comps;
    return (PyObject*)res;
}

PyObject* PyCOMPS_cmp(PyObject *self, PyObject *other, int op) {
    char res;

    CMP_OP_EQ_NE_CHECK(op)
    res = COMPS_OBJECT_CMP(((PyCOMPS*)self)->comps_doc,
                           ((PyCOMPS*)other)->comps_doc);
    if (op == Py_EQ && res) {
        Py_RETURN_TRUE;
    } else if (op == Py_NE && !res){
        Py_RETURN_TRUE;
    } else {
        Py_RETURN_FALSE;
    }
}

PyNumberMethods PyCOMPS_Nums = {
    .nb_add = PyCOMPS_union
};


PyTypeObject PyCOMPS_Type = {
    PY_OBJ_HEAD_INIT
    "_libpycomps.Comps",             /*tp_name*/
    sizeof(PyCOMPS), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyCOMPS_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    &PyCOMPS_Nums,              /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT,        /*tp_flags*/
    PYCOMPS_DOCU,             /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    PyCOMPS_cmp,               /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    PyCOMPS_methods,           /* tp_methods */
    PyCOMPS_members,           /* tp_members */
    PyCOMPS_getset,            /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyCOMPS_init,      /* tp_init */
    0,                         /* tp_alloc */
    PyCOMPS_new,                /* tp_new */};

#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
            PyModuleDef_HEAD_INIT,
            "_libpycomps",
            "libcomps module",
            -1,
            NULL, //myextension_methods,
            NULL,
            NULL, //myextension_traverse,
            NULL, //myextension_clear,
            NULL
    };
#endif

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif


PyMODINIT_FUNC
PYINIT_FUNC(void) {
    PyObject *m;
    PyCOMPS_GroupType.tp_new = PyCOMPSGroup_new;
    PyCOMPS_Type.tp_new = PyCOMPS_new;
    if (PyType_Ready(&PyCOMPS_Type) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_CatType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_CatsType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_GIDType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_GIDsType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_EnvsType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_EnvType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_GroupType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_GroupsType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_PacksType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_PackType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_DictType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_SeqIterType) < 0 ) {
        MODINIT_RET_NONE;
    }
    if (PyType_Ready(&PyCOMPS_DictIterType) < 0 ) {
        MODINIT_RET_NONE;
    }
    #if PY_MAJOR_VERSION >= 3
        m = PyModule_Create(&moduledef);
    #else
        m = Py_InitModule("_libpycomps", NULL);
    #endif
    Py_INCREF(&PyCOMPS_Type);
    PyModule_AddObject(m, "Comps", (PyObject*) &PyCOMPS_Type);
    Py_INCREF(&PyCOMPS_CatsType);
    PyModule_AddObject(m, "CategoryList", (PyObject*) &PyCOMPS_CatsType);
    Py_INCREF(&PyCOMPS_CatType);
    PyModule_AddObject(m, "Category", (PyObject*) &PyCOMPS_CatType);
    Py_INCREF(&PyCOMPS_GIDsType);
    PyModule_AddObject(m, "IdList", (PyObject*) &PyCOMPS_GIDsType);
    Py_INCREF(&PyCOMPS_GroupType);
    PyModule_AddObject(m, "Group", (PyObject*) &PyCOMPS_GroupType);
    Py_INCREF(&PyCOMPS_GroupsType);
    PyModule_AddObject(m, "GroupList", (PyObject*) &PyCOMPS_GroupsType);
    Py_INCREF(&PyCOMPS_GIDType);
    PyModule_AddObject(m, "GroupId", (PyObject*) &PyCOMPS_GIDType);
    Py_INCREF(&PyCOMPS_PacksType);
    PyModule_AddObject(m, "PackageList", (PyObject*) &PyCOMPS_PacksType);
    Py_INCREF(&PyCOMPS_PackType);
    PyModule_AddObject(m, "Package", (PyObject*) &PyCOMPS_PackType);
    Py_INCREF(&PyCOMPS_EnvType);
    PyModule_AddObject(m, "Environment", (PyObject*) &PyCOMPS_EnvType);
    Py_INCREF(&PyCOMPS_EnvsType);
    PyModule_AddObject(m, "EnvList", (PyObject*) &PyCOMPS_EnvsType);
    Py_INCREF(&PyCOMPS_DictType);
    PyModule_AddObject(m, "Dict", (PyObject*) &PyCOMPS_DictType);

    PyModule_AddIntConstant(m, "PACKAGE_TYPE_DEFAULT", COMPS_PACKAGE_DEFAULT);
    PyModule_AddIntConstant(m, "PACKAGE_TYPE_OPTIONAL", COMPS_PACKAGE_OPTIONAL);
    PyModule_AddIntConstant(m, "PACKAGE_TYPE_CONDITIONAL", COMPS_PACKAGE_CONDITIONAL);
    PyModule_AddIntConstant(m, "PACKAGE_TYPE_MANDATORY", COMPS_PACKAGE_MANDATORY);
    PyModule_AddIntConstant(m, "PACKAGE_TYPE_UNKNOWN", COMPS_PACKAGE_UNKNOWN);
    #if PY_MAJOR_VERSION >= 3
        return m;
    #endif
}

