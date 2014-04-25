/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 *	  $Id$
 *
 *****************************************************************************/

#include "bindings.h"
#include "InputServiceBinder.h"
#include "PythonCallback.h"
#include "PythonStruct.h"

namespace Opde
{

	namespace Python
	{

        // InputEventType type info
		template<> struct TypeInfo<InputEventType> {
			VariableType type;
			const char* typeName;

			TypeInfo() : type(VT_CUSTOM_TYPE), typeName("InputEventType") {};
			virtual ~TypeInfo() {};

			virtual PyObject* toPyObject(InputEventType val) const {
				return PyLong_FromLong(val);
			}
		};

		class PythonInputMessage : public PythonStruct<InputEventMsg> {
			public:
				static void init() {
					field("event", &InputEventMsg::event);
					field("command", &InputEventMsg::command);
					field("params", &InputEventMsg::params);
					// Todo: Expose the InputEventType string repr. to Dict of the module (need PyOBject* param)
				}
		};

		template<> const char* PythonStruct<InputEventMsg>::msName = "InputEventMsg";

		class PythonInputMessageConverter {
			public:
				PyObject* operator()(const InputEventMsg& ev) {
					PyObject* result = PythonInputMessage::create(ev);
					return result;
				}
		};


		typedef PythonCallback<InputEventMsg, PythonInputMessageConverter> PythonInputCallback;
		typedef shared_ptr<PythonInputCallback> PythonInputCallbackPtr;

		// -------------------- Input Service --------------------
		const char* InputServiceBinder::msName = "InputService";

		// ------------------------------------------
		PyTypeObject InputServiceBinder::msType = {
			PyVarObject_HEAD_INIT(&PyType_Type, 0)
			"opde.services.InputService",		// char *tp_name; */
			sizeof(InputServiceBinder::Object), // int tp_basicsize; */
			0,									// int tp_itemsize;       /* not used much */
			InputServiceBinder::dealloc,		// destructor tp_dealloc; */
			0,									// printfunc  tp_print;   */
			0,		// getattrfunc  tp_getattr; /* __getattr__ */
			0,								// setattrfunc  tp_setattr;  /* __setattr__ */
			0,									// cmpfunc  tp_compare;  /* __cmp__ */
			0,									// reprfunc  tp_repr;    /* __repr__ */
			0,									// PyNumberMethods *tp_as_number; */
			0,									// PySequenceMethods *tp_as_sequence; */
			0,									// PyMappingMethods *tp_as_mapping; */
			0,									// hashfunc tp_hash;     /* __hash__ */
			0,									// ternaryfunc tp_call;  /* __call__ */
			0,									// reprfunc tp_str;      /* __str__ */
			PyObject_GenericGetAttr,  			// getattrofunc tp_getattro; */
			0,									// setattrofunc tp_setattro; */
			0,									// PyBufferProcs *tp_as_buffer; */
			0,									// long tp_flags; */
			0,									// char *tp_doc;  */
			0,									// traverseproc tp_traverse; */
			0,									// inquiry tp_clear; */
			0,									// richcmpfunc tp_richcompare; */
			0,									// long tp_weaklistoffset; */
			0,									// getiterfunc tp_iter; */
			0,									// iternextfunc tp_iternext; */
			msMethods,							// struct PyMethodDef *tp_methods; */
			0,									// struct memberlist *tp_members; */
			0,									// struct getsetlist *tp_getset; */
		};

		// ------------------------------------------
		PyMethodDef InputServiceBinder::msMethods[] =
		{
		    {"createBindContext", createBindContext, METH_VARARGS},
		    {"setBindContext", setBindContext, METH_VARARGS},
		    {"command", command, METH_VARARGS},
		    {"registerCommandTrap", registerCommandTrap, METH_VARARGS},
		    {"unregisterCommandTrap", unregisterCommandTrap, METH_VARARGS},
		    {"setInputMapped", setInputMapped, METH_VARARGS},
		    {NULL, NULL},
		};


		// ------------------------------------------
		PyObject* InputServiceBinder::createBindContext(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InputServicePtr o;

			if (!python_cast<InputServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
			    o->createBindContext(name);

			    __PY_NONE_RET;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::setBindContext(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InputServicePtr o;

			if (!python_cast<InputServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				o->setBindContext(name);

				__PY_NONE_RET;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::command(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InputServicePtr o;

			if (!python_cast<InputServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			char* command;

			if (PyArg_ParseTuple(args, "s", &command))
			{
				std::string Command = command;
			    o->processCommand(Command);

			    __PY_NONE_RET;
			} else {
				PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::registerCommandTrap(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			PyObject *result = NULL;
			InputServicePtr o;

			if (!python_cast<InputServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			char* name;
			PyObject* callable;

			if (PyArg_ParseTuple(args, "sO", &name, &callable)) {
				try {
					InputService::ListenerPtr pcp(new PythonInputCallback(callable));

					// call the is to register the command trap
					o->registerCommandTrap(name, pcp);

					__PY_NONE_RET;
				} catch (BasicException) {
					PyErr_SetString(PyExc_TypeError, "Error setting a callback, is the given argument a callable?");
					return NULL;
				}
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a string and callable parameters!");
			}

			return result;
			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::unregisterCommandTrap(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			InputServicePtr o;

			if (!python_cast<InputServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			char* name;

			if (PyArg_ParseTuple(args, "s", &name)) {
				o->unregisterCommandTrap(name);

				__PY_NONE_RET;
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a string parameter!");
					return NULL;
			}

			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::setInputMapped(PyObject* self, PyObject* args)
		{
			__PYTHON_EXCEPTION_GUARD_BEGIN_;
			InputServicePtr o;

			if (!python_cast<InputServicePtr>(self, &msType, &o))
				__PY_CONVERR_RET;

			PyObject *mapped;

			if (PyArg_ParseTuple(args, "O", &mapped)) {
				bool cmap;

				if (!TypeInfo<bool>::fromPyObject(mapped, cmap))
					__PY_BADPARMS_RET;

				o->setInputMode(cmap ? IM_MAPPED : IM_DIRECT);

				__PY_NONE_RET;
			} else {
					PyErr_SetString(PyExc_TypeError, "Expected a bool parameter!");
					return NULL;
			}

			__PYTHON_EXCEPTION_GUARD_END_;
		}

		// ------------------------------------------
		PyObject* InputServiceBinder::create()
		{
			Object* object = construct(&msType);

			if (object != NULL)
			{
				object->mInstance = GET_SERVICE(InputService);
			}
			return (PyObject *)object;
		}

		// ------------------------------------------
		void InputServiceBinder::init(PyObject* module) {
			PythonInputMessage::init();

			publishType(module, &msType, msName);
		}
	}

} // namespace Opde
