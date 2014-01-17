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

#pragma once

#include <map>
#include <vector>
#include <atlbase.h>
#include <comdef.h>
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>
#include "variants.h"

extern NPNetscapeFuncs NPNFuncs;
extern NPClass GenericNPObjectClass;

typedef bool (*DefaultInvoker)(void *object, const NPVariant *args, uint32_t argCount, NPVariant *result);

struct ltnum
{
	bool operator()(long n1, long n2) const
	{
		return n1 < n2;
	}
};

struct ltstr
{
	bool operator()(const char* s1, const char* s2) const
	{
		return strcmp(s1, s2) < 0;
	}
};

bool toString(void *object, const NPVariant *args, uint32_t argCount, NPVariant *result);

class GenericNPObject: public NPObject
{
private:
	GenericNPObject(const GenericNPObject &);

	bool invalid;
	DefaultInvoker defInvoker;
	void *defInvokerObject;

	std::vector<NPVariant> numeric_mapper;
	// the members of alpha mapper can be reassigned to anything the user wishes
	std::map<const char *, NPVariant, ltstr> alpha_mapper;
	// these cannot accept other types than they are initially defined with
	std::map<const char *, NPVariant, ltstr> immutables;

public:
	friend bool toString(void *, const NPVariant *, uint32_t, NPVariant *);

	GenericNPObject(NPP instance);
	GenericNPObject(NPP instance, bool isMethodObj);
	~GenericNPObject();

	void Invalidate() {invalid = true;}

	void SetDefaultInvoker(DefaultInvoker di, void *context) {

		defInvoker = di;
		defInvokerObject = context;
	}

	static bool _HasMethod(NPObject *npobj, NPIdentifier name) {

		return ((GenericNPObject *)npobj)->HasMethod(name);
	}

	static bool _Invoke(NPObject *npobj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) {

		return ((GenericNPObject *)npobj)->Invoke(methodName, args, argCount, result);
	}

	static bool _InvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result) {

		if (((GenericNPObject *)npobj)->defInvoker) {

			return (((GenericNPObject *)npobj)->InvokeDefault)(args, argCount, result);
		}
		else {

			return false;
		}
	}

	static bool _HasProperty(NPObject *npobj, NPIdentifier name) {

		return ((GenericNPObject *)npobj)->HasProperty(name);
	}

	static bool _GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {

		return ((GenericNPObject *)npobj)->GetProperty(name, result);
	}

	static bool _SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {

		return ((GenericNPObject *)npobj)->SetProperty(name, value);
	}

	static bool _RemoveProperty(NPObject *npobj, NPIdentifier name) {

		return ((GenericNPObject *)npobj)->RemoveProperty(name);
	}

	static bool _Enumerate(NPObject *npobj, NPIdentifier **identifiers, uint32_t *identifierCount) {

		return ((GenericNPObject *)npobj)->Enumerate(identifiers, identifierCount);
	}

	bool HasMethod(NPIdentifier name);
	bool Invoke(NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool InvokeDefault(const NPVariant *args, uint32_t argCount, NPVariant *result);
	bool HasProperty(NPIdentifier name);
	bool GetProperty(NPIdentifier name, NPVariant *result);
	bool SetProperty(NPIdentifier name, const NPVariant *value);
	bool RemoveProperty(NPIdentifier name);
	bool Enumerate(NPIdentifier **identifiers, uint32_t *identifierCount);
};
