#ifndef INCLUDED_k20_h_
#define INCLUDED_k20_h_ 1
#ifdef __cplusplus
extern "C" {
#endif

#include <Python.h>
#include "k20.h"
typedef struct {
	PyObject_HEAD
	K kobj;
} PyK_K;

#define PyK_KType (PyTypeObject *)PyK_API[PyK_KType_NUM]
#define PyK_KType_NUM 0
#define PyK_KCheck(op) \
   PyObject_TypeCheck(op, PyK_KType)


#define PyK_mk_K_RET PyObject *
#define PyK_mk_K_PROTO (K kobj)
#define PyK_mk_K_NUM 1

#define PyK_API_pointers 2

#if defined(PYK_K20_MODULE)
static void *PyK_API[PyK_API_pointers];
#else
static void **PyK_API;
#define PyK_mk_K \
  (*(PyK_mk_K_RET (*)PyK_mk_K_PROTO)		\
   PyK_API[PyK_mk_K_NUM])
#endif


#define import_pyk_k20() \
{ \
  PyObject *module = PyImport_ImportModule("pyk._k20"); \
  if (module != NULL) { \
    PyObject *c_api_object = PyObject_GetAttrString(module, "_C_API"); \
    if (PyCObject_Check(c_api_object)) { \
      PyK_API = (void **)PyCObject_AsVoidPtr(c_api_object); \
    } \
    Py_DECREF(c_api_object); \
  } \
}
#ifdef __cplusplus
}
#endif
#endif /* INCLUDED_k20_h_ */
