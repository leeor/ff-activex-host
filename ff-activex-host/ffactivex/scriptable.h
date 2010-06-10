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

#include <atlbase.h>
#include <comdef.h>
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>
#include "variants.h"

extern NPNetscapeFuncs NPNFuncs;
extern NPClass ScriptableNPClass;

class Scriptable: public NPObject
{
private:
	Scriptable(const Scriptable &);

	// This method iterates all members of the current interface, looking for the member with the 
	// id of member_id. If not found within this interface, it will iterate all base interfaces
	// recursively, until a match is found, or all the hierarchy was searched.
	bool find_member(ITypeInfoPtr info, TYPEATTR *attr, DISPID member_id, unsigned int invKind) {

		bool found = false;
		unsigned int i = 0;

		FUNCDESC *fDesc;

		for (i = 0; 
			 (i < attr->cFuncs) 
				&& !found; 
			 ++i) {

			HRESULT hr = info->GetFuncDesc(i, &fDesc);
			if (   SUCCEEDED(hr) 
				&& fDesc 
				&& (fDesc->memid == member_id)) {

				if (invKind & fDesc->invkind)
					found = true;
			}
			info->ReleaseFuncDesc(fDesc);
		}

		if (!found && (invKind & ~INVOKE_FUNC)) {

			VARDESC *vDesc;

			for (i = 0; 
				 (i < attr->cVars) 
					&& !found; 
				 ++i) {

				HRESULT hr = info->GetVarDesc(i, &vDesc);
				if (   SUCCEEDED(hr) 
					&& vDesc 
					&& (vDesc->memid == member_id)) {

					found = true;
				}
				info->ReleaseVarDesc(vDesc);
			}
		}

		if (!found) {
			// iterate inherited interfaces
			HREFTYPE refType = NULL;

			for (i = 0; (i < attr->cImplTypes) && !found; ++i) {

				ITypeInfoPtr baseInfo;
				TYPEATTR *baseAttr;

				if (FAILED(info->GetRefTypeOfImplType(0, &refType))) {

					continue;
				}

				if (FAILED(info->GetRefTypeInfo(refType, &baseInfo))) {

					continue;
				}

				baseInfo->AddRef();
				if (FAILED(baseInfo->GetTypeAttr(&baseAttr))) {

					continue;
				}

				found = find_member(baseInfo, baseAttr, member_id, invKind);
				baseInfo->ReleaseTypeAttr(baseAttr);
			}
		}

		return found;
	}

	DISPID ResolveName(NPIdentifier name, unsigned int invKind) {

		bool found = false;
		DISPID dID = -1;
		USES_CONVERSION;

		if (!name || !invKind) {

			return -1;
		}

		if (!NPNFuncs.identifierisstring(name)) {

			return -1;
		}

		NPUTF8 *npname = NPNFuncs.utf8fromidentifier(name);
		LPOLESTR oleName = A2W(npname);
		//free(npname);
		//npname = NULL;

		IDispatchPtr disp = control.GetInterfacePtr();
		if (!disp) {

			return -1;
		}

		disp->AddRef();

		disp->GetIDsOfNames(IID_NULL, &oleName, 1, LOCALE_SYSTEM_DEFAULT, &dID);
		if (dID != -1) {

			ITypeInfoPtr info;
			disp->GetTypeInfo(0, LOCALE_SYSTEM_DEFAULT, &info);
			if (!info) {

				return -1;
			}

			info->AddRef();

			TYPEATTR *attr;
			if (FAILED(info->GetTypeAttr(&attr))) {

				return -1;
			}

			found = find_member(info, attr, dID, invKind);
			info->ReleaseTypeAttr(attr);
		}

		return found ? dID : -1;
	}

	bool InvokeControl(DISPID id, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult) {

		IDispatchPtr disp = control.GetInterfacePtr();
		if (!disp) {

			return false;
		}

		disp->AddRef();

		HRESULT hr = disp->Invoke(id, IID_NULL, LOCALE_SYSTEM_DEFAULT, wFlags, pDispParams, pVarResult, NULL, NULL);

		return (SUCCEEDED(hr)) ? true : false;
	}

	IUnknownPtr control;
	NPP instance;
	bool invalid;

public:
	Scriptable():
		invalid(false),
		control(NULL),
		instance(NULL) {
	}

	~Scriptable() {control->Release();}

	void setControl(IUnknown *unk) {control = unk;}
	void setControl(IDispatch *disp) {disp->QueryInterface(IID_IUnknown, (void **)&control);}
	void setInstance(NPP inst) {instance = inst;}

	void Invalidate() {invalid = true;}

	static bool _HasMethod(NPObject *npobj, NPIdentifier name) {

		return ((Scriptable *)npobj)->HasMethod(name);
	}

	static bool _Invoke(NPObject *npobj, NPIdentifier name,
						const NPVariant *args, uint32_t argCount,
						NPVariant *result) {

		return ((Scriptable *)npobj)->Invoke(name, args, argCount, result);
	}

	static bool _HasProperty(NPObject *npobj, NPIdentifier name) {

		return ((Scriptable *)npobj)->HasProperty(name);
	}

	static bool _GetProperty(NPObject *npobj, NPIdentifier name, NPVariant *result) {

		return ((Scriptable *)npobj)->GetProperty(name, result);
	}

	static bool _SetProperty(NPObject *npobj, NPIdentifier name, const NPVariant *value) {

		return ((Scriptable *)npobj)->SetProperty(name, value);
	}

	bool HasMethod(NPIdentifier name) {

		if (invalid) return false;

		DISPID id = ResolveName(name, INVOKE_FUNC);
		return (id != -1) ? true : false;
	}

	bool Invoke(NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {

		if (invalid) return false;

		DISPID id = ResolveName(name, INVOKE_FUNC);
		if (-1 == id) {

			return false;
		}

		VARIANT *vArgs = NULL;
		if (argCount) {

			vArgs = new VARIANT[argCount];
			if (!vArgs) {

				return false;
			}

			for (unsigned int i = 0; i < argCount; ++i) {

				// copy the arguments in reverse order
				NPVar2Variant(&args[i], &vArgs[argCount - i - 1], instance);
			}
		}

		DISPPARAMS params = {NULL, NULL, 0, 0};

		params.cArgs = argCount;
		params.cNamedArgs = 0;
		params.rgdispidNamedArgs = NULL;
		params.rgvarg = vArgs;

		VARIANT vResult;

		bool rc = InvokeControl(id, DISPATCH_METHOD, &params, &vResult);
		if (vArgs) delete[] vArgs;

		if (!rc) {

			return false;
		}

		Variant2NPVar(&vResult, result, instance);
		return true;
	}

	bool HasProperty(NPIdentifier name) {

		if (invalid) return false;

		DISPID id = ResolveName(name, INVOKE_PROPERTYGET | INVOKE_PROPERTYPUT);
		return (id != -1) ? true : false;
	}

	bool GetProperty(NPIdentifier name, NPVariant *result) {

		if (invalid) return false;

		DISPID id = ResolveName(name, INVOKE_PROPERTYGET);
		if (-1 == id) {

			return false;
		}

		DISPPARAMS params;

		params.cArgs = 0;
		params.cNamedArgs = 0;
		params.rgdispidNamedArgs = NULL;
		params.rgvarg = NULL;

		VARIANT vResult;

		if (!InvokeControl(id, DISPATCH_PROPERTYGET, &params, &vResult)) {

			return false;
		}

		Variant2NPVar(&vResult, result, instance);
		return true;
	}

	bool SetProperty(NPIdentifier name, const NPVariant *value) {

		if (invalid) return false;

		DISPID id = ResolveName(name, INVOKE_PROPERTYPUT);
		if (-1 == id) {

			return false;
		}

		VARIANT val;
		NPVar2Variant(value, &val, instance);

		DISPPARAMS params;
		// Special initialization needed when using propery put.
		DISPID dispidNamed = DISPID_PROPERTYPUT;
		params.cNamedArgs = 1;
		params.rgdispidNamedArgs = &dispidNamed;
		params.cArgs = 1;
		params.rgvarg = &val;

		VARIANT vResult;
		if (!InvokeControl(id, DISPATCH_PROPERTYPUT, &params, &vResult)) {

			return false;
		}

		return true;
	}
};

