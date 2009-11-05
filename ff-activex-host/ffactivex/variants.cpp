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
 * Contributor:
 *                Ruediger Jungbeck <ruediger.jungbeck@rsj.de>
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

#include <atlbase.h>
#include <npapi.h>
#include <npfunctions.h>
#include <prtypes.h>

#include <npruntime.h>
#include "scriptable.h"
#include "variants.h"

extern NPNetscapeFuncs NPNFuncs;

BSTR 
Utf8StringToBstr(LPCSTR szStr, int iSize)
{ 
	BSTR bstrVal; 
    // Chars required for string
	int iReq;

	if ((iReq = MultiByteToWideChar(CP_UTF8, 0, szStr, iSize, 0, 0)) ==  0) {

		return (0); 
	}

	// Account for terminating 0. 
    if (iReq != -1) {

		++iReq; 
	}

    if ((bstrVal = ::SysAllocStringLen(0, iReq)) == 0) {

		return (0); 
	}

	memset(bstrVal, 0, iReq * sizeof(wchar_t));

	// Convert into the buffer. 
	if (MultiByteToWideChar(CP_UTF8, 0, szStr, iSize, bstrVal, iReq) == 0) {

		::SysFreeString(bstrVal); 
		return 0; 
	} 

	return (bstrVal); 
}

void
Variant2NPVar(const VARIANT *var, NPVariant *npvar, NPP instance)
{
	USES_CONVERSION;
	char *cStr = NULL;
	char *npStr = NULL;
	NPObject *obj = NULL;

	if (!var || !npvar) {

		return;
	}

	switch (var->vt)
	{
	case VT_EMPTY:
		VOID_TO_NPVARIANT((*npvar));
		break;

	case VT_LPSTR:
		STRINGZ_TO_NPVARIANT(var->pcVal, (*npvar));
		break;

	case VT_BSTR:
		cStr = OLE2A(var->bstrVal);
		// complete lack of documentation on Mozilla's part here, I have no
		// idea how this string is supposed to be freed
		npStr = (char *)NPNFuncs.memalloc(strlen(cStr) + 1);
		if (npStr) {

			memset(npStr, 0, strlen(cStr) + 1);
			memcpy(npStr, cStr, strlen(cStr));
			STRINGZ_TO_NPVARIANT(npStr, (*npvar));
		}
		else {

			STRINGZ_TO_NPVARIANT(NULL, (*npvar));
		}
		break;

	case VT_I1:
		INT32_TO_NPVARIANT((int32)var->cVal, (*npvar));
		break;

	case VT_I2:
		INT32_TO_NPVARIANT((int32)var->iVal, (*npvar));
		break;

	case VT_I4:
		INT32_TO_NPVARIANT((int32)var->lVal, (*npvar));
		break;

	case VT_UI1:
		INT32_TO_NPVARIANT((int32)var->bVal, (*npvar));
		break;

	case VT_UI2:
		INT32_TO_NPVARIANT((int32)var->uiVal, (*npvar));
		break;

	case VT_UI4:
		INT32_TO_NPVARIANT((int32)var->ulVal, (*npvar));
		break;

	case VT_BOOL:
		BOOLEAN_TO_NPVARIANT(var->boolVal == VARIANT_TRUE ? true : false, (*npvar));
		break;

	case VT_R4:
		DOUBLE_TO_NPVARIANT((double)var->fltVal, (*npvar));
		break;

	case VT_R8:
		DOUBLE_TO_NPVARIANT(var->dblVal, (*npvar));
		break;

	case VT_DISPATCH:
		obj = NPNFuncs.createobject(instance, &ScriptableNPClass);

		((Scriptable *)obj)->setControl(var->pdispVal);
		((Scriptable *)obj)->setInstance(instance);
		OBJECT_TO_NPVARIANT(obj, (*npvar));
		break;

	default:
		// Some unsupported type
		break;
	}
}

void
NPVar2Variant(const NPVariant *npvar, VARIANT *var)
{
	USES_CONVERSION;

	if (!var || !npvar) {

		return;
	}

	switch (npvar->type) {
	case NPVariantType_Void:
		var->vt = VT_VOID;
		var->ulVal = 0;
		break;

	case NPVariantType_Null:
		var->vt = VT_PTR;
		var->byref = NULL;
		break;

	case NPVariantType_Bool:
		var->vt = VT_BOOL;
		var->ulVal = npvar->value.boolValue;
		break;

	case NPVariantType_Int32:
		var->vt = VT_UI4;
		var->ulVal = npvar->value.intValue;
		break;

	case NPVariantType_Double:
		var->vt = VT_R8;
		var->dblVal = npvar->value.doubleValue;
		break;

	case NPVariantType_String:
		var->vt = VT_BSTR;
		var->bstrVal = Utf8StringToBstr(npvar->value.stringValue.UTF8Characters, npvar->value.stringValue.UTF8Length);
		break;

	case NPVariantType_Object:
		// An object of type NPObject - currently not supported.
		var->vt = VT_VOID;
		var->ulVal = 0;
		break;
	}
}

