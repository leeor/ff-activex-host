/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is itstructures.com code.
 *
 * The Initial Developer of the Original Code is IT Structures.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <algorithm>
#include <string>
#include "GenericNPObject.h"

static NPObject*
AllocateGenericNPObject(NPP npp, NPClass *aClass)
{
	return new GenericNPObject(npp, false);
}

static NPObject*
AllocateMethodNPObject(NPP npp, NPClass *aClass)
{
	return new GenericNPObject(npp, true);
}

static void
DeallocateGenericNPObject(NPObject *obj)
{
	if (!obj) {

		return;
	}

	GenericNPObject *m = (GenericNPObject *)obj;
	delete m;
}

static void
InvalidateGenericNPObject(NPObject *obj)
{
	if (!obj) {

		return;
	}

	((GenericNPObject *)obj)->Invalidate();
}

NPClass GenericNPObjectClass = {
	/* version */		NP_CLASS_STRUCT_VERSION,
	/* allocate */		AllocateGenericNPObject,
	/* deallocate */	DeallocateGenericNPObject,
	/* invalidate */	InvalidateGenericNPObject,
	/* hasMethod */		GenericNPObject::_HasMethod,
	/* invoke */		GenericNPObject::_Invoke,
	/* invokeDefault */	GenericNPObject::_InvokeDefault,
	/* hasProperty */	GenericNPObject::_HasProperty,
	/* getProperty */	GenericNPObject::_GetProperty,
	/* setProperty */	GenericNPObject::_SetProperty,
	/* removeProperty */ GenericNPObject::_RemoveProperty,
	/* enumerate */		GenericNPObject::_Enumerate,
	/* construct */		NULL
};

NPClass MethodNPObjectClass = {
	/* version */		NP_CLASS_STRUCT_VERSION,
	/* allocate */		AllocateMethodNPObject,
	/* deallocate */	DeallocateGenericNPObject,
	/* invalidate */	InvalidateGenericNPObject,
	/* hasMethod */		GenericNPObject::_HasMethod,
	/* invoke */		GenericNPObject::_Invoke,
	/* invokeDefault */	GenericNPObject::_InvokeDefault,
	/* hasProperty */	GenericNPObject::_HasProperty,
	/* getProperty */	GenericNPObject::_GetProperty,
	/* setProperty */	GenericNPObject::_SetProperty,
	/* removeProperty */ GenericNPObject::_RemoveProperty,
	/* enumerate */		GenericNPObject::_Enumerate,
	/* construct */		NULL
};

// Some standard JavaScript methods

bool toString(void *object, const NPVariant *args, uint32_t argCount, NPVariant *result) {

	GenericNPObject *map = (GenericNPObject *)object;

	if (!map || map->invalid) return false;

	// no args expected or cared for...

	std::string out;
	std::vector<NPVariant>::iterator it;
	for (it = map->numeric_mapper.begin(); it < map->numeric_mapper.end(); ++it) {

		if (NPVARIANT_IS_VOID(*it)) {

			out += ",";
		}
		else if (NPVARIANT_IS_NULL(*it)) {

			out += ",";
		}
		else if (NPVARIANT_IS_BOOLEAN(*it)) {

			if ((*it).value.boolValue) {

				out += "true,";
			}
			else {

				out += "false,";
			}
		}
		else if (NPVARIANT_IS_INT32(*it)) {

			char tmp[50];
			memset(tmp, 0, sizeof(tmp));
			_snprintf(tmp, 49, "%d,", (*it).value.intValue);
			out += tmp;
		}
		else if (NPVARIANT_IS_DOUBLE(*it)) {

			char tmp[50];
			memset(tmp, 0, sizeof(tmp));
			_snprintf(tmp, 49, "%f,", (*it).value.doubleValue);
			out += tmp;
		}
		else if (NPVARIANT_IS_STRING(*it)) {

			out += (*it).value.stringValue.UTF8Characters;
			out += ",";
		}
		else if (NPVARIANT_IS_OBJECT(*it)) {

			out += "[object],";
		}
	}

	// calculate how much space we need
	std::string::size_type size = out.length();

	char *s = (char *)NPNFuncs.memalloc(size * sizeof(char));
	if (NULL == s) {

		return false;
	}

	memcpy(s, out.c_str(), size);
	s[size - 1] = 0; // overwrite the last ","
	STRINGZ_TO_NPVARIANT(s, (*result));
	return true;
}

// Some helpers

static void free_numeric_element(NPVariant elem) {

	NPNFuncs.releasevariantvalue(&elem);
}

static void free_alpha_element(std::pair<const char *, NPVariant> elem) {

	NPNFuncs.releasevariantvalue(&(elem.second));
}

// And now the GenericNPObject implementation

GenericNPObject::GenericNPObject(NPP instance, bool isMethodObj):
	invalid(false), defInvoker(NULL), defInvokerObject(NULL) {

	NPVariant val;

	INT32_TO_NPVARIANT(0, val);
	immutables["length"] = val;

	if (!isMethodObj) {

		NPObject *obj = NULL;
		obj = NPNFuncs.createobject(instance, &MethodNPObjectClass);
		if (NULL == obj) {

			throw NULL;
		}
		((GenericNPObject *)obj)->SetDefaultInvoker(&toString, this);

		OBJECT_TO_NPVARIANT(obj, val);
		immutables["toString"] = val;
	}
}

GenericNPObject::~GenericNPObject() {

	for_each(immutables.begin(), immutables.end(), free_alpha_element);
	for_each(alpha_mapper.begin(), alpha_mapper.end(), free_alpha_element);
	for_each(numeric_mapper.begin(), numeric_mapper.end(), free_numeric_element);
}

bool GenericNPObject::HasMethod(NPIdentifier name) {

	if (invalid) return false;

	if (NPNFuncs.identifierisstring(name)) {

		char *key = NPNFuncs.utf8fromidentifier(name);
		if (immutables.count(key) > 0) {

			if (NPVARIANT_IS_OBJECT(immutables[key])) {

				return (NULL != immutables[key].value.objectValue->_class->invokeDefault);
			}
		}
		else if (alpha_mapper.count(key) > 0) {

			if (NPVARIANT_IS_OBJECT(alpha_mapper[key])) {

				return (NULL != alpha_mapper[key].value.objectValue->_class->invokeDefault);
			}
		}
	}

	return false;
}

bool GenericNPObject::Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {

	if (invalid) return false;

	if (NPNFuncs.identifierisstring(name)) {

		char *key = NPNFuncs.utf8fromidentifier(name);
		if (immutables.count(key) > 0) {

			if (   NPVARIANT_IS_OBJECT(immutables[key]) 
				&& immutables[key].value.objectValue->_class->invokeDefault) {

				return immutables[key].value.objectValue->_class->invokeDefault(immutables[key].value.objectValue, args, argCount, result);
			}
		}
		else if (alpha_mapper.count(key) > 0) {

			if (   NPVARIANT_IS_OBJECT(alpha_mapper[key])
				&& alpha_mapper[key].value.objectValue->_class->invokeDefault) {

				return alpha_mapper[key].value.objectValue->_class->invokeDefault(alpha_mapper[key].value.objectValue, args, argCount, result);
			}
		}
	}

	return true;
}

bool GenericNPObject::InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result) {

	if (invalid) return false;

	if (defInvoker) {

		defInvoker(defInvokerObject, args, argCount, result);
	}

	return true;
}

// This method is also called before the JS engine attempts to add a new
// property, most likely it's trying to check that the key is supported.
// It only returns false if the string name was not found, or does not
// hold a callable object
bool GenericNPObject::HasProperty(NPIdentifier name) {

	if (invalid) return false;

	if (NPNFuncs.identifierisstring(name)) {

		char *key = NPNFuncs.utf8fromidentifier(name);
		if (immutables.count(key) > 0) {

			if (NPVARIANT_IS_OBJECT(immutables[key])) {

				return (NULL == immutables[key].value.objectValue->_class->invokeDefault);
			}
		}
		else if (alpha_mapper.count(key) > 0) {

			if (NPVARIANT_IS_OBJECT(alpha_mapper[key])) {

				return (NULL == alpha_mapper[key].value.objectValue->_class->invokeDefault);
			}
		}

		return false;
	}

	return true;
}

static bool CopyNPVariant(NPVariant *dst, const NPVariant *src)
{
	dst->type = src->type;
	if (NPVARIANT_IS_STRING(*src)) {

		NPUTF8 *str = (NPUTF8 *)NPNFuncs.memalloc((src->value.stringValue.UTF8Length + 1) * sizeof(NPUTF8));
		if (NULL == str) {

			return false;
		}
		dst->value.stringValue.UTF8Length = src->value.stringValue.UTF8Length;

		memcpy(str, src->value.stringValue.UTF8Characters, src->value.stringValue.UTF8Length);
		str[dst->value.stringValue.UTF8Length] = 0;

		dst->value.stringValue.UTF8Characters = str;
	}
	else if (NPVARIANT_IS_OBJECT(*src)) {

		NPNFuncs.retainobject(NPVARIANT_TO_OBJECT(*src));
		dst->value.objectValue = src->value.objectValue;
	}
	else {

		dst->value = src->value;
	}

	return true;
}

#define MIN(x, y)	((x) < (y)) ? (x) : (y)

bool GenericNPObject::GetProperty(NPIdentifier name, NPVariant *result) {

	if (invalid) return false;

	try {
		if (NPNFuncs.identifierisstring(name)) {

			char *key = NPNFuncs.utf8fromidentifier(name);
			if (immutables.count(key) > 0) {

				if (!CopyNPVariant(result, &(immutables[key]))) {

					return false;
				}
			}
			else if (alpha_mapper.count(key) > 0) {

				if (!CopyNPVariant(result, &(alpha_mapper[key]))) {

					return false;
				}
			}
		}
		else {
			// assume int...
			unsigned long key = (unsigned long)NPNFuncs.intfromidentifier(name);
			if (numeric_mapper.size() > key) {

				if (!CopyNPVariant(result, &(numeric_mapper[key]))) {

					return false;
				}
			}
		}
	}
	catch (...) {
	}

	return true;
}

bool GenericNPObject::SetProperty(NPIdentifier name, const NPVariant *value) {

	if (invalid) return false;

	try {
		if (NPNFuncs.identifierisstring(name)) {

			char *key = NPNFuncs.utf8fromidentifier(name);
			if (immutables.count(key) > 0) {
				// the key is already defined as immutable, check the new value type
				if (value->type != immutables[key].type) {

					return false;
				}

				// Seems ok, copy the new value
				if (!CopyNPVariant(&(immutables[key]), value)) {

					return false;
				}
			}
			else if (!CopyNPVariant(&(alpha_mapper[key]), value)) {

				return false;
			}
		}
		else {
			// assume int...
			NPVariant var;
			if (!CopyNPVariant(&var, value)) {

				return false;
			}

			unsigned long key = (unsigned long)NPNFuncs.intfromidentifier(name);
			if (key >= numeric_mapper.size()) {
				// there's a gap we need to fill
				NPVariant pad;
				VOID_TO_NPVARIANT(pad);
				numeric_mapper.insert(numeric_mapper.end(), key - numeric_mapper.size() + 1, pad);
			}
			numeric_mapper.at(key) = var;
			NPVARIANT_TO_INT32(immutables["length"])++;
		}
	}
	catch (...) {
	}

	return true;
}

bool GenericNPObject::RemoveProperty(NPIdentifier name) {

	if (invalid) return false;

	try {
		if (NPNFuncs.identifierisstring(name)) {

			char *key = NPNFuncs.utf8fromidentifier(name);
			if (alpha_mapper.count(key) > 0) {

				NPNFuncs.releasevariantvalue(&(alpha_mapper[key]));
				alpha_mapper.erase(key);
			}
		}
		else {
			// assume int...
			unsigned long key = (unsigned long)NPNFuncs.intfromidentifier(name);
			if (numeric_mapper.size() > key) {

				NPNFuncs.releasevariantvalue(&(numeric_mapper[key]));
				numeric_mapper.erase(numeric_mapper.begin() + key);
			}
			NPVARIANT_TO_INT32(immutables["length"])--;
		}
	}
	catch (...) {
	}

	return true;
}

bool GenericNPObject::Enumerate(NPIdentifier **identifiers, uint32_t *identifierCount) {

	if (invalid) return false;

	try {
		*identifiers = (NPIdentifier *)NPNFuncs.memalloc(sizeof(NPIdentifier) * numeric_mapper.size());
		if (NULL == *identifiers) {

			return false;
		}
		*identifierCount = 0;

		std::vector<NPVariant>::iterator it;
		unsigned int i = 0;
		char str[10] = "";
		for (it = numeric_mapper.begin(); it < numeric_mapper.end(); ++it, ++i) {

			// skip empty (padding) elements
			if (NPVARIANT_IS_VOID(*it)) continue;
			_snprintf(str, sizeof(str), "%u", i);
			(*identifiers)[(*identifierCount)++] = NPNFuncs.getstringidentifier(str);
		}
	}
	catch (...) {
	}

	return true;
}
