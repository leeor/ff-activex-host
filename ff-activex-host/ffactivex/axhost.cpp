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

#include <comdef.h>
#include "ffactivex.h"
#include "scriptable.h"
#include "axhost.h"

static LRESULT CALLBACK AxHostWinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;

	CAxHost *host = (CAxHost *)GetWindowLong(hWnd, GWL_USERDATA);

	if (!host) {

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	switch (msg)
	{
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
	case WM_SIZE:
		if (host->Site) {

			host->Site->OnDefWindowMessage(msg, wParam, lParam, &result);
			return result;
		}
		else {

			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

	// Window being destroyed
	case WM_DESTROY:
		break;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return true;
}

CAxHost::~CAxHost()
{
	if (Window){

		if (OldProc)
			::SetWindowLong(Window, GWL_WNDPROC, (LONG)OldProc);

		::SetWindowLong(Window, GWL_USERDATA, (LONG)NULL);
	}

	if (Sink) {

		Sink->UnsubscribeFromEvents();
        Sink->Release();
    }

    if (Site) {

		Site->Detach();
        Site->Release();
    }
}

CAxHost::CAxHost(NPP inst):
	instance(inst),
	ClsID(CLSID_NULL),
	isValidClsID(false),
	Sink(NULL),
	Site(NULL),
	OldProc(NULL),
	Props()
{
}

void 
CAxHost::setWindow(HWND win)
{
	if (win != Window) {

		if (win) {
			// subclass window so we can intercept window messages and
			// do our drawing to it
			OldProc = (WNDPROC)::SetWindowLong(win, GWL_WNDPROC, (LONG)AxHostWinProc);

			// associate window with our CAxHost object so we can access 
			// it in the window procedure
			::SetWindowLong(win, GWL_USERDATA, (LONG)this);
		}
		else {
			if (OldProc)
				::SetWindowLong(Window, GWL_WNDPROC, (LONG)OldProc);

			::SetWindowLong(Window, GWL_USERDATA, (LONG)NULL);
		}

		Window = win;
	}
}

HWND 
CAxHost::getWinfow()
{
	return Window;
}

void 
CAxHost::UpdateRect(RECT rcPos)
{
	HRESULT hr = -1;

	if (Site && Window) {

		if (Site->GetParentWindow() == NULL) {

			hr = Site->Attach(Window, rcPos, NULL);
			if (FAILED(hr)) {

				log(instance, "AxHost.UpdateRect: failed to attach control");
			}
        }
        else {

			Site->SetPosition(rcPos);
        }

        // Ensure clipping on parent to keep child controls happy
        ::SetWindowLong(Window, GWL_STYLE, ::GetWindowLong(Window, GWL_STYLE) | WS_CLIPCHILDREN);
	}
}

bool
CAxHost::verifyClsID(LPOLESTR oleClsID)
{
	CRegKey keyExplorer;
	if (ERROR_SUCCESS == keyExplorer.Open(HKEY_LOCAL_MACHINE, 
										  _T("SOFTWARE\\Microsoft\\Internet Explorer\\ActiveX Compatibility"), 
										  KEY_READ)) {
		CRegKey keyCLSID;
        if (ERROR_SUCCESS == keyCLSID.Open(keyExplorer, W2T(oleClsID), KEY_READ)) {

			DWORD dwType = REG_DWORD;
            DWORD dwFlags = 0;
            DWORD dwBufSize = sizeof(dwFlags);
            if (ERROR_SUCCESS == ::RegQueryValueEx(keyCLSID, 
												   _T("Compatibility Flags"), 
												   NULL, 
												   &dwType, 
												   (LPBYTE) 
												   &dwFlags, 
												   &dwBufSize)) {
                // Flags for this reg key
                const DWORD kKillBit = 0x00000400;
                if (dwFlags & kKillBit) {

					log(instance, "AxHost.verifyClsID: the control is marked as unsafe by IE kill bits");
					return false;
                }
            }
        }
    }

	return true;
}

bool 
CAxHost::setClsID(const char *clsid)
{
	HRESULT hr = -1;
	USES_CONVERSION;
	LPOLESTR oleClsID = A2OLE(clsid);

    // Check the Internet Explorer list of vulnerable controls
    if (oleClsID && verifyClsID(oleClsID)) {

		hr = CLSIDFromString(oleClsID, &ClsID);
		if (SUCCEEDED(hr) && !::IsEqualCLSID(ClsID, CLSID_NULL)) {

			isValidClsID = true;
			return true;
		}
    }

	log(instance, "AxHost.setClsID: failed to set the requested clsid");
	return false;
}

bool
CAxHost::setClsIDFromProgID(const char *progid)
{
	HRESULT hr = -1;
	CLSID clsid = CLSID_NULL;
	USES_CONVERSION;
	LPOLESTR oleClsID = NULL;
	LPOLESTR oleProgID = A2OLE(progid);

	hr = CLSIDFromProgID(oleProgID, &clsid);
	if (FAILED(hr)) {

		log(instance, "AxHost.setClsIDFromProgID: could not resolve PROGID");
		return false;
	}

	hr = StringFromCLSID(clsid, &oleClsID);

    // Check the Internet Explorer list of vulnerable controls
    if (   SUCCEEDED(hr) 
		&& oleClsID 
		&& verifyClsID(oleClsID)) {

		ClsID = clsid;
		if (!::IsEqualCLSID(ClsID, CLSID_NULL)) {

			isValidClsID = true;
			return true;
		}
    }

	log(instance, "AxHost.setClsIDFromProgID: failed to set the resolved CLSID");
	return false;
}

bool 
CAxHost::hasValidClsID()
{
	return isValidClsID;
}

bool
CAxHost::CreateControl()
{
	if (!isValidClsID) {

		log(instance, "AxHost.CreateControl: current location is not trusted");
		return false;
	}

	// Create the control site
	CControlSiteInstance::CreateInstance(&Site);
	if (Site == NULL) {

		log(instance, "AxHost.CreateControl: CreateInstance failed");
		return false;
	}

	Site->m_bSupportWindowlessActivation = FALSE;

	Site->SetSecurityPolicy(NULL);
	Site->m_bSafeForScriptingObjectsOnly = TRUE;

	Site->AddRef();

	// Create the object
	HRESULT hr;
	hr = Site->Create(ClsID, Props);

	if (FAILED(hr)) {

		log(instance, "AxHost.CreateControl: failed to create site");
		return false;
	}

	IUnknown *control;
	Site->GetControlUnknown(&control);

	// Create the event sink
	CControlEventSinkInstance::CreateInstance(&Sink);
	Sink->AddRef();
	Sink->instance = instance;
	hr = Sink->SubscribeToEvents(control);
	 
	if (FAILED(hr)) {

		log(instance, "AxHost.CreateControl: SubscribeToEvents failed");
		return false;
	}

	return true;
}

bool 
CAxHost::AddEventHandler(wchar_t *name, wchar_t *handler)
{
	HRESULT hr;
	DISPID id = 0;
	USES_CONVERSION;
	LPOLESTR oleName = name;

	if (!Sink) {

		log(instance, "AxHost.AddEventHandler: no valid sink");
		return false;
	}

	hr = Sink->m_spEventSinkTypeInfo->GetIDsOfNames(&oleName, 1, &id);
	if (FAILED(hr)) {

		log(instance, "AxHost.AddEventHandler: GetIDsOfNames failed to resolve event name");
		return false;
	}

	Sink->events[id] = handler;
	return true;
}

int16
CAxHost::HandleEvent(void *event)
{
	NPEvent *npEvent = (NPEvent *)event;
	LRESULT result = 0;

	if (!npEvent) {

		return 0;
	}

	// forward all events to the hosted control
	return (int16)Site->OnDefWindowMessage(npEvent->event, npEvent->wParam, npEvent->lParam, &result);
}

NPObject *
CAxHost::GetScriptableObject()
{
	IUnknown *unk;
	NPObject *obj = NPNFuncs.createobject(instance, &ScriptableNPClass);

	Site->GetControlUnknown(&unk);
	((Scriptable *)obj)->setControl(unk);
	((Scriptable *)obj)->setInstance(instance);

	return obj;
}
