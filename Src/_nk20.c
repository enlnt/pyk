/* -*- c-basic-offset: 8 -*- */
#include "_k20.h"
#include <Numeric/arrayobject.h>
static char doc[] =
"connects k with Numeric\n"
;
static char __version__[] = "$Revision: 1.2 $";

static int types[] = {
	PyArray_OBJECT, /* t = 0 (K object)    */
	PyArray_LONG,   /* t = 1 (K integer)   */
	PyArray_DOUBLE, /* t = 2 (K double)    */
	PyArray_UBYTE,  /* t = 3 (K character) */
	PyArray_OBJECT,  /* t = 4 (K symbol) */
	PyArray_OBJECT  /* t = 5 (K dictionary) */
};

static int
ktype(int atype)
{
	switch (atype) {
	case PyArray_SHORT:
	case PyArray_USHORT:
	case PyArray_INT:
	case PyArray_UINT: 
	case PyArray_LONG: return 1;
	case PyArray_FLOAT:
	case PyArray_DOUBLE: return 2;
	case PyArray_CHAR:
	case PyArray_SBYTE:		        
	case PyArray_UBYTE: return 3;
	}
	return -1;
}
#define LEN(x) (sizeof(x)/sizeof(*x))

static PyObject*
_ktoarray(PyObject* self, PyObject* k)
{
	PyArrayObject *ret;
	if (!PyK_KCheck(k)) {
		return PyErr_Format(PyExc_TypeError, "not k object");
	}
	
	K kobj = ((PyK_K*)k)->kobj;
	if (!kobj) {
		return PyErr_Format(PyExc_AssertionError, "null kobj");
	}
	int t = kobj->t;
	/* XXX k objects of type 0 should be converted 
	 * to non-contiguous arrays rather than trigger
	 * an error.
	 */
	if (abs(t) >= LEN(types) && t != 5 ) {
		return PyErr_Format(PyExc_TypeError, 
				    "cannot create an array from a "
				    "k object of type %d", t);
	}
	int type = types[abs(t)]; /* PyArray type */
	int nd = t <= 0 || t == 5;          /* Number of dimensions (0 or 1) */
	int* d = &kobj->n;        /* Shape */
	char* data;
	switch (t) {
	case 1:
		data = (char*)&Ki(kobj);
		break;
	case 3:
		data = &Kc(kobj);
		break;
	case 4:
		data = Ks(kobj);
		break;
	default:
		data = KC(kobj);
	}
	/* Special handling for symbols arrays: convert data to Python strings */
	PyObject** buf = 0;
	if (t == -4) {
		int n = *d, i = 0;
		buf = (PyObject**)malloc(n * sizeof(PyObject*));
		for (i = 0; i < n; ++i) {
			char* s = KS(kobj)[i];
			if (!s) goto fail;
			buf[i] = PyString_FromString(s);
			if (!buf[i]) goto fail;
		}
		data = (char*)buf;
	} else if (t == 0 || t == 5) {
		int n = *d, i = 0;
		buf = (PyObject**)malloc(n * sizeof(PyObject*));
		for (i = 0; i < n; ++i) {
			K ki = KK(kobj)[i];
			if (!ki) goto fail;
			ci(ki);
			buf[i] = PyK_mk_K(ki);
			if (!buf[i]) {
				cd(ki);
				goto fail;
			}
		}
		data = (char*)buf;
	}
	if (!(ret = (PyArrayObject *)PyArray_FromDimsAndData(nd, d, type, data))) {
		goto fail;
	}
	if (buf) {
		ret->flags |= OWN_DATA;
	} else {
		Py_INCREF(k);
		ret->base = k;
	}
	return (PyObject*)ret;
 fail:
	if (buf) free(buf);
	return NULL;
}

static PyObject*
_arraytok(PyObject* self, PyObject* a)
{
	K kobj = 0;
	if (!PyArray_Check(a)) {
		return PyErr_Format(PyExc_TypeError, 
				    "argument is not a numeric array");	
	}
	PyArrayObject* arr = (PyArrayObject*)a;
	if (!PyArray_ISCONTIGUOUS(arr)) {
		return PyErr_Format(PyExc_TypeError, 
				    "cannot handle non-contiguous arrays");	
	}
	int n = PyArray_SIZE(arr);
	int t = ktype(arr->descr->type_num);
	if (t > 0) {
		kobj = gtn(-t,n);
		memcpy(kobj->k, arr->data, PyArray_NBYTES(arr));
	} else if (arr->descr->type_num == PyArray_OBJECT) {
		/* special handling for arrays of strings */
		char** strings = malloc(n*sizeof(char*));
		PyObject** objects = (PyObject**)arr->data;
		int i;
		for (i = 0; i < n; ++i) {
			char* str = PyString_AsString(objects[i]);
			if (str) {
				strings[i] = str;
			} else {
				free(strings);
				/* XXX should we raise our own exception here       *
				 * XXX or keep the one which came from "AsString"?  */
				return NULL;
			}
		}
		kobj = gtn(-4, n);
		for (i = 0; i < n; ++i) {
			KS(kobj)[i] = sp(strings[i]);
		}
	}
	return PyK_mk_K(kobj);
}
static PyMethodDef funcs[] = {
	{"_ktoarray", (PyCFunction)_ktoarray, METH_O,
	 "convert a k object to a Numeric array"},
	{"_arraytok", (PyCFunction)_arraytok, METH_O,
	 "convert a Numeric array to a k object"},
	{NULL}
};


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
init_nk20(void) 
{
	PyObject* m;
	import_array();	
	import_pyk_k20();
	m = Py_InitModule3("_nk20", funcs, doc);	
	PyModule_AddStringConstant(m, "__version__", __version__);
}


