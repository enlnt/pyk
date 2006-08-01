/* -*- c-basic-offset: 8 -*- */
#define PYK_K20_MODULE 1
#include "_k20.h"

#define xcd(k) if (k) cd(k)
#define xci(k) if (k) ci(k)
#define IS_K(k) (PyObject_TypeCheck(k, &_KType))

typedef PyK_K _K;
static PyObject *KErr;

static char doc[] =
"module to access k from python.\n"
"\n"
"This module is implemented as a thin layer on top of libk20. Most\n"
"functions described in http://www.kx.com/a/k/connect/c/CandK.txt are\n"
"implemented in this module. Names of the functions are formed by\n"
"prefixing the libk20 names with an _. Note that this module\n"
"is not intended for the end user, but rather as a building block for\n"
"the higher level K module";
static char __version__[] = "$Revision: 1.2 $";
static void 
_K_dealloc(_K* self)
{
	if (self->kobj) {
		cd(self->kobj);
	}
	self->ob_type->tp_free((PyObject*)self);
}
/* XXX should be properly named and exported */
static int _is_k(PyObject*);

static PyObject*
_mk_K(K kobj);

static int
_K_init(_K *self, PyObject *args, PyObject *kwds);


static PyObject*
_K_c(PyObject* self)
{
	I c = ((_K*)self)->kobj->c;
	return Py_BuildValue("i", c);
}

static PyObject*
_K_t(PyObject* self)
{
	K k = ((_K*)self)->kobj; 
	I t = k ? k->t : 6;
	return Py_BuildValue("i", t);
}

static PyObject*
_K_n(PyObject* self)
{
	I n = ((_K*)self)->kobj->n;
	return Py_BuildValue("i", n);
}


static PyObject*
_K_repr(PyObject* self);


static PyMethodDef _K_methods[] = {
    {"refc", (PyCFunction)_K_c, METH_NOARGS,
     "K reference count of the object"},
    {"ktype", (PyCFunction)_K_t, METH_NOARGS,
     "K data type of the object"},
    {"ksize",  (PyCFunction)_K_n, METH_NOARGS,
     "K the number of data items when the object is a list or dictionary"},
    {NULL}  /* Sentinel */
};

static PyObject *
_K_getattro(_K *self, PyObject *nameo)
{
	char *name = 0;
	if (PyString_Check(nameo)) {
		name = PyString_AS_STRING(nameo);
	}
	if (name && name[1] == 0) {
		switch (name[0]) {
		case 'c':
			return Py_BuildValue("i", self->kobj->c);
		case 't':
			return Py_BuildValue("i", self->kobj->t);
		case 'n':
			return Py_BuildValue("i", self->kobj->n);
		}
	}
	return PyObject_GenericGetAttr((PyObject*)self, nameo);
}

static int
_K_setattro(_K *self, PyObject *nameo, PyObject *value)
{
	char *name = 0;
	if (PyString_Check(nameo)) {
		name = PyString_AS_STRING(nameo);
	}
	if (name && name[1] == 0 && strchr("ktc", name[0])) {
		PyErr_Format(PyExc_AttributeError,
			     "attribute %c is read only", name[0]);
		return -1;
	}
	return PyObject_GenericSetAttr((PyObject*)self, nameo, value);
}

static int
_K_length(_K *self)
{
	return self->kobj->n;
}
static PyObject *
_K_subscript(_K *self, PyObject *key)
{
	int i;
	K kobj = self->kobj;
	char *skey;
	int key_length;
	int value_index = 1;
	if (kobj->t != 5) {
		PyErr_Format(PyExc_TypeError,
			     "k object of type %d is not a dictionary", kobj->t);
		return NULL;
	}
	if (-1 == PyString_AsStringAndSize(key, &skey, &key_length)) {
		return NULL;
	}
	if (skey[key_length-1] == '.') {
		--key_length;
		++value_index;
	}
	for (i=0; i < kobj->n; ++i) {
		K e = KK(kobj)[i];
		if (0 == strncmp(skey,Ks(KK(e)[0]),key_length)) {
			PyTypeObject* type = self->ob_type;
			_K* k = (_K*)type->tp_alloc(type, 0);
			k->kobj = ci(KK(e)[value_index]);
			return (PyObject*)k;
		}
	}
	PyErr_SetObject(PyExc_KeyError, key);
	return NULL;
}

static int
_K_ass_sub(_K *self, PyObject *key, PyObject *value)
{
	int i;
	K kobj = self->kobj, kval;
	char *skey;
	int key_length;
	int value_index = 1;
	if (kobj->t != 5) {
		PyErr_Format(PyExc_TypeError,
			     "k object of type %d is not a dictionary", kobj->t);
		return -1;
	}
	if (-1 == PyString_AsStringAndSize(key, &skey, &key_length)) {
		return -1;
	}	
	if (value == NULL) {
		PyErr_Format(PyExc_NotImplementedError, "k del key");
		return -1;
	} 
	if (_is_k(value)) {
		kval = ((_K*)value)->kobj;
	} else {
		PyErr_Format(PyExc_TypeError, "value is not a k object");
		return -1;
	}
	if (skey[key_length-1] == '.') {
		--key_length;
		++value_index;
	}
	for (i=0; i < kobj->n; ++i) {
		K e = KK(kobj)[i];
		if (0 == strncmp(skey,Ks(KK(e)[0]),key_length)) {
			KK(e)[value_index] = ci(kval);
			return 0;
		}
	}
	PyErr_SetObject(PyExc_KeyError, key);
	return -1;
}

static PyMappingMethods _K_as_mapping = {
	(inquiry)_K_length, /*mp_length*/
	(binaryfunc)_K_subscript, /*mp_subscript*/
	(objobjargproc)_K_ass_sub, /*mp_ass_subscript*/
};

PyTypeObject _KType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pyk._K",                  /*tp_name*/
    sizeof(_K),                /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)_K_dealloc,    /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    _K_repr,                   /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &_K_as_mapping,            /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    (getattrofunc)_K_getattro, /*tp_getattro*/
    (setattrofunc)_K_setattro, /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "_K objects",             /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    _K_methods,               /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)_K_init,        /* tp_init */
    0,                         /* tp_alloc */
    PyType_GenericNew,                   /* tp_new */
};

static PyObject*
_gi(PyObject* self, PyObject* o)
{
	int i = PyInt_AsLong(o);
	K kobj = gi(i);
	if (!kobj) {
		PyErr_SetString(PyExc_RuntimeError, "gi returned NULL");
		return NULL;
	}
	return _mk_K(kobj);
}


static PyObject*
_gf(PyObject* self, PyObject* o)
{
	double d = PyFloat_AsDouble(o);
	K kobj = gf(d);
	if (!kobj) {
		PyErr_SetString(PyExc_RuntimeError, "gf returned NULL");
		return NULL;
	}
	return _mk_K(kobj);
}

static PyObject*
_gc(PyObject* self, PyObject* o)
{
	char* c = PyString_AsString(o);
	K kobj;
	if (strlen(c) != 1) {
		PyErr_BadArgument();
		return NULL;
	}
	kobj = gc(c[0]);
	if (!kobj) {
		PyErr_SetString(PyExc_RuntimeError, "gc returned NULL");
		return NULL;
	}
	return _mk_K(kobj);
}

static PyObject*
_gs(PyObject* self, PyObject* o)
{
	char* s = PyString_AsString(o);
	K kobj;
	if (!s) {
		PyErr_BadArgument();
		return NULL;
	}
	s = sp(s);
	if (!s) {
		PyErr_SetString(PyExc_RuntimeError, "sp returned NULL");
		return NULL;
	}
	kobj = gs(s);
	if (!kobj) {
		PyErr_SetString(PyExc_RuntimeError, "gs returned NULL");
		return NULL;
	}
	return _mk_K(kobj);
}

static PyObject*
_gn(PyObject* self)
{
	K kobj = gn();
	if (!kobj) {
		PyErr_SetString(PyExc_RuntimeError, "gn returned NULL");
		return NULL;
	}
	return _mk_K(kobj);
}

static PyObject*
_get_Ki(PyObject* self, PyObject* ko)
{
	K kobj;
	int ok;
	ok = ko && IS_K(ko);
	if (!ok) goto fail;
	kobj = ((_K*)ko)->kobj;
	if (kobj && kobj->t == 1) {
		int i = Ki(kobj);
		return Py_BuildValue("i", i);
	}
 fail:
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_get_Kf(PyObject* self, PyObject* ko)
{
	K kobj;
	int ok;
	ok = ko && IS_K(ko);
	if (!ok) goto fail;
	kobj = ((_K*)ko)->kobj;
	if (kobj && kobj->t == 2) {
		double f = Kf(kobj);
		return Py_BuildValue("f", f);
	}
 fail:
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_get_Kc(PyObject* self, PyObject* ko)
{
	K kobj;
	int ok;
	ok = ko && IS_K(ko);
	if (!ok) goto fail;
	kobj = ((_K*)ko)->kobj;
	if (kobj && kobj->t == 3) {
		char c[2] = "\0\0";
		c[0] = Kc(kobj);
		return Py_BuildValue("s", c);
	}
 fail:
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_get_Ks(PyObject* self, PyObject* ko)
{
	K kobj;
	int ok;
	ok = ko && IS_K(ko);
	if (!ok) goto fail;
	kobj = ((_K*)ko)->kobj;
	if (kobj && kobj->t == 4) {
		char* s = Ks(kobj);
		return Py_BuildValue("s", s);
	}
 fail:
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_set_Ki(PyObject* self, PyObject* args)
{
	PyObject* ko;
	int i;
	if (PyArg_ParseTuple(args, "O!i", &_KType, &ko, &i)) {
		K k = ((_K*)ko)->kobj;
		if (k && k->t == 1) {
			Ki(k) = i;
			Py_INCREF(ko);
			return ko;
		} else {
			PyErr_SetString(PyExc_TypeError, "wrong k type");
			return NULL;
		}
	}
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_set_Kf(PyObject* self, PyObject* args)
{
	PyObject* ko;
	double f;
	if (PyArg_ParseTuple(args, "O!d", &_KType, &ko, &f)) {
		K k = ((_K*)ko)->kobj;
		if (k && k->t == 2) {
			Kf(k) = f;
			Py_INCREF(ko);
			return ko;
		} else {
			PyErr_SetString(PyExc_TypeError, "wrong k type");
			return NULL;
		}
	}
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_set_Kc(PyObject* self, PyObject* args)
{
	PyObject* ko;
	char* c;
	if (PyArg_ParseTuple(args, "O!s", &_KType, &ko, &c)) {
		K k = ((_K*)ko)->kobj;
		if (k && k->t == 3) {
			Ki(k) = c[0];
			Py_INCREF(ko);
			return ko;
		} else {
			PyErr_SetString(PyExc_TypeError, "wrong k type");
			return NULL;
		}
	}
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_set_Ks(PyObject* self, PyObject* args)
{
	PyObject* ko;
	char* s;
	if (PyArg_ParseTuple(args, "O!s", &_KType, &ko, &s)) {
		K k = ((_K*)ko)->kobj;
		if (k && k->t == 4) {
			Ks(k) = sp(s);
			Py_INCREF(ko);
			return ko;
		} else {
			PyErr_SetString(PyExc_TypeError, "wrong k type");
			return NULL;
		}
	}
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_gtn(PyObject* self, PyObject* args)
{
	int type, count;
	if (PyArg_ParseTuple(args, "ii", &type, &count)) {
		_K* k;
		K kobj = gtn(type, count);
		if (!kobj) {
			PyErr_SetString(PyExc_RuntimeError, "gtn");
			return NULL;
		}
		k = (_K*)_KType.tp_alloc(&_KType, 0);
		if (!k) {
			PyErr_SetString(PyExc_MemoryError, "_K");
			return NULL;
		}
		k->kobj = kobj; 
		return (PyObject*)k;
	}
	PyErr_BadArgument();
	return NULL;
}

/* XXX unfortunately API function gnk of which pyk.gk
   is based is a vararg function and therefore cannot
   be portably exported to Python. It would be better
   if libk20 supplied a function gnk_(I, K*)
   in addition to gnk(I,...) which would take an array
   of K objects as the second argument */
static PyObject*
_gk(PyObject* self, PyObject* args)
{
	int n = PyTuple_Size(args);
	if (!n) {
		return _mk_K(gtn(0,0));
	}
	int i, type = INT_MAX;
	K* ks = (K*)malloc(n*sizeof(K));
	K kobj;
	for(i = 0; i < n; i++) {
		K ki;
		int t;
		PyObject* argi = PyTuple_GET_ITEM(args, i);
		if (!IS_K(argi)) {
			goto fail;
		}
		ks[i] = ki = ((_K*)argi)->kobj;
		t = ki->t;
		if (INT_MAX == type) {
			type = t;
		} else if (t > 4 || t < 1 || t != type) {
			type = 0;
		}
	}
	kobj = gtn((type>0 && type<5)?-type:0, n);
	if (!kobj) {
		free(ks);
		return PyErr_Format(PyExc_TypeError, "gtn(%d,%d) returned null", -type, n);
	}
	switch (type) {
	case 1:
		for (i = 0; i < n; i++) {
			KI(kobj)[i] = Ki(ks[i]);
		}
		break;
	case 2:
		for (i = 0; i < n; i++) {
			KF(kobj)[i] = Kf(ks[i]);
		}
		break;
	case 3:
		for (i = 0; i < n; i++) {
			KC(kobj)[i] = Kc(ks[i]);
		}
		break;
	case 4:
		for (i = 0; i < n; i++) {
			KS(kobj)[i] = Ks(ks[i]);
		}
		break;
	default:
		memcpy(KK(kobj), ks, n*sizeof(K));
		for (i = 0; i < n; i++) {
			ci(ks[i]);
		}
		break;
	}
	free(ks);
	return _mk_K(kobj);
 fail:
	free(ks);
	PyErr_BadArgument();
	return NULL;
}

static PyObject*
_gp(PyObject* self, PyObject* o)
{
	_K* k;
	K kobj;
	char* s;
	int n;
	if (PyString_AsStringAndSize(o, &s, &n)) {
		return NULL;
	}
	kobj = gpn(s, n);
	if (!kobj) {
		PyErr_SetString(PyExc_RuntimeError, "gp");
		return NULL;
	}
	k = (_K*)_KType.tp_alloc(&_KType, 0);
	if (!k) {
		PyErr_SetString(PyExc_MemoryError, "_K");
		return NULL;
	}
	k->kobj = kobj; 
	return (PyObject*)k;	
}


static PyObject*
_ksk(PyObject* self, PyObject* args)
{
	_K* ka = 0;
	char* s;
	if (PyArg_ParseTuple(args, "s|O!", &s, &_KType, &ka)) {
		K kobj = ksk(s, ka?ka->kobj:0);
		if (!kobj) {
			return PyErr_Format(PyExc_TypeError, 
					    "k failed to execute %s", s) ;
		}
		if (6 == kobj->t && 0 != kobj->n ) {
			return PyErr_Format(KErr, "k: %s error", (S)kobj->n);
		}
		return _mk_K(kobj);
	}
	PyErr_BadArgument();
	return NULL;
}

static PyMethodDef funcs[] = {
	{"_gi", (PyCFunction)_gi, METH_O,
	 "generate an integer atom, as in gi(3)"},
	{"_gf", (PyCFunction)_gf, METH_O,
	 "generate a floating-point atom, as in gf(3.5)"},
	{"_gc", (PyCFunction)_gc, METH_O,
	 "generate a character atom, as in gc('c')"},
	{"_gs", (PyCFunction)_gs, METH_O,
	 "generate a symbol atom, as in gs('price')"},
	{"_gn", (PyCFunction)_gn, METH_NOARGS,
	 "generate the null atom, as in gn()"},
	{"_gp", (PyCFunction)_gp, METH_O,
	 "generate a character list, as in gp('price')"},
	{"_Ki", (PyCFunction)_get_Ki, METH_O,
	 "get value of an integer atom"},
	{"_Kf", (PyCFunction)_get_Kf, METH_O,
	 "get value of a double atom"},
	{"_Kc", (PyCFunction)_get_Kc, METH_O,
	 "get value of a character atom"},
	{"_Ks", (PyCFunction)_get_Ks, METH_O,
	 "get value of a symbol atom"},
	{"_sKi", (PyCFunction)_set_Ki, METH_VARARGS,
	 "set value of an integer atom"},
	{"_sKf", (PyCFunction)_set_Kf, METH_VARARGS,
	 "set value of a double atom"},
	{"_sKc", (PyCFunction)_set_Kc, METH_VARARGS,
	 "set value of a character atom"},
	{"_sKs", (PyCFunction)_set_Ks, METH_VARARGS,
	 "set value of a symbol atom"},
	{"_gtn", (PyCFunction)_gtn, METH_VARARGS,
	 "gtn(type,count) generates a list of given type and number of items"},
	{"_gk", (PyCFunction)_gk, METH_VARARGS,
	 "generate a k list, as in gk(gi(1), gi(2))\n"
	 "like API function gnk, but with n inferred from the number of arguments"},
	{"_ksk", (PyCFunction)_ksk, METH_VARARGS,
	 "ksk(expr, args) evaluates a k expression expr with arguments args\n"
	 "args must be a k list"},
	{NULL}
};


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
init_k20(void) 
{
	PyObject* m;
	PyObject* c_api_object;

	if (PyType_Ready(&_KType) < 0)
		return;
	
	m = Py_InitModule3("_k20", funcs, doc);
	
	if (m == NULL)
		return;
	
	Py_INCREF(&_KType);
	PyModule_AddObject(m, "_K", (PyObject *)&_KType);

	if (KErr == NULL) {
		KErr = PyErr_NewException("_k20.kerr", NULL, NULL);
		if (KErr == NULL)
			return;
	}
	/* Initialize the C API pointer array */
	PyK_API[PyK_KType_NUM] = (void*)&_KType;
	PyK_API[PyK_mk_K_NUM] = (void*)&_mk_K;
	/* Create a CObject containing the API pointer array's address */
	c_api_object = PyCObject_FromVoidPtr((void*)PyK_API, NULL);
	if (c_api_object) {
		PyModule_AddObject(m, "_C_API", c_api_object);
	}
	Py_INCREF(KErr);
	PyModule_AddObject(m, "kerr", KErr);
	PyModule_AddStringConstant(m, "__version__", __version__);
	/* initialize k */
	cd(ksk("", 0));
}


static int
_K_init(_K *self, PyObject *args, PyObject *kwds)
{
	PyObject* ko = NULL;
	if (PyArg_ParseTuple(args, "|O!", &_KType, &ko)) {
		K kobj;
		if (ko) {
			kobj = ((_K*)ko)->kobj;
			xci(kobj);
		} else {
			kobj = gn();
		}
		self->kobj = kobj; 
		return 0;
	}
	PyErr_SetString(PyExc_TypeError, "not k type");
	return -1;
}

static PyObject*
_mk_K(K kobj)
{
	if (!kobj) {
		return PyErr_Format(PyExc_AssertionError, "null kobj");
	}
	_K* k = (_K*)_KType.tp_alloc(&_KType, 0);
	if (!k) {
		PyErr_SetString(PyExc_MemoryError, "_K");
		return NULL;
	}
	k->kobj = kobj; 
	return (PyObject*)k;
}

static PyObject*
_K_repr(PyObject* self)
{
	if (IS_K(self)) {
		K k = ((_K*)self)->kobj;
		if (!k) {
			PyErr_SetString(PyExc_AssertionError, "null kobj");
			return NULL;
		}
		K ks = ksk("{$5:x}", gnk(1, k));
		if (!ks) {
			PyErr_SetString(PyExc_RuntimeError, "ksk returned null");
			return NULL;
		}
		PyObject* o = Py_BuildValue("s", KC(ks));
		cd(ks);
		return o;
	}
	return PyErr_Format(PyExc_TypeError, "not k type %s", 
			    self->ob_type->tp_name);
}

/* XXX should be properly named and exported */
static int
_is_k(PyObject* o)
{
	return IS_K(o);
}
