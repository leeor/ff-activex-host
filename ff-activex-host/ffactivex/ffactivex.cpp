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

// ffactivex.cpp : Defines the exported functions for the DLL application.
//

#include "ffactivex.h"
#include "common/stdafx.h"
#include "axhost.h"
#include "atlutil.h"

// A list of trusted domains
// Each domain name may start with a '*' to specify that sub domains are 
// trusted as well
// Note that a '.' is not enforced after the '*'
static const char *TrustedLocations[] = {
	"try.itstructures.com",
	"webintg",
	"webintg2",
};
static const unsigned int numTrustedLocations = sizeof(TrustedLocations) / sizeof(char *);

static const char *LocalhostName = "localhost";
static const bool TrustLocalhost = true;

void *
ffax_calloc(unsigned int size)
{
	void *ptr = NULL;

	ptr = NPNFuncs.memalloc(size);
	if (ptr) {

		memset(ptr, 0, size);
	}

	return ptr;
}

void
ffax_free(void *ptr)
{
	if (ptr)
		NPNFuncs.memfree(ptr);
}

//
// Gecko API
//

void
log(NPP instance, char *message)
{
	NPVariant varConsole;
	NPVariant result;
	NPVariant args;
	NPObject *globalObj = NULL;
	bool rc = false;
	NPNFuncs.getvalue(instance, NPNVWindowNPObject, &globalObj);

	NPIdentifier console = NPNFuncs.getstringidentifier("console");
	rc = NPNFuncs.getproperty(instance, globalObj, console, &varConsole);
	NPNFuncs.releaseobject(globalObj);
	if (!rc){

		return;
	}

	// Get a pointer to the "location" object.
	NPObject *consoleObj = varConsole.value.objectValue;
	// Create a "href" identifier.
	NPIdentifier identifier = NPNFuncs.getstringidentifier("log");

	STRINGZ_TO_NPVARIANT(message, args);

	bool success = NPNFuncs.invoke(instance, consoleObj, identifier, &args, 1, &result);
	NPNFuncs.releasevariantvalue(&varConsole);
}

static bool
VerifySiteLock(NPP instance)
{
	USES_CONVERSION;
	NPObject *globalObj = NULL;
	NPIdentifier identifier;
	NPVariant varLocation;
	NPVariant varHref;
	bool rc = false;
	CUrl url;

	if (!numTrustedLocations) {

		return true;
	}

	// Get the window object.
	NPNFuncs.getvalue(instance, NPNVWindowNPObject, &globalObj);
	// Create a "location" identifier.
	identifier = NPNFuncs.getstringidentifier("location");

	// Get the location property from the window object (which is another object).
	rc = NPNFuncs.getproperty(instance, globalObj, identifier, &varLocation);
	NPNFuncs.releaseobject(globalObj);
	if (!rc){

		log(instance, "AxHost.VerifySiteLock: could not get the location from the global object");
		return false;
	}

	// Get a pointer to the "location" object.
	NPObject *locationObj = varLocation.value.objectValue;
	// Create a "href" identifier.
	identifier = NPNFuncs.getstringidentifier("href");
	// Get the location property from the location object.
	rc = NPNFuncs.getproperty(instance, locationObj, identifier, &varHref);
	NPNFuncs.releasevariantvalue(&varLocation);
	if (!rc) {

		log(instance, "AxHost.VerifySiteLock: could not get the href from the location property");
		return false;
	}

	rc = url.CrackUrl(A2W(varHref.value.stringValue.utf8characters), ATL_URL_DECODE);
	NPNFuncs.releasevariantvalue(&varHref);

	if (!rc) {

		log(instance, "AxHost.VerifySiteLock: failed to parse the current location URL");
		return false;
	}

	if (   (url.GetScheme() == ATL_URL_SCHEME_FILE) 
		|| (!strncmp(LocalhostName, W2A(url.GetHostName()), strlen(LocalhostName)))){

		return TrustLocalhost;
	}

	for (unsigned int i = 0; i < numTrustedLocations; ++i) {

		if (TrustedLocations[i][0] == '*') {
			// sub domains are trusted
			unsigned int len = strlen(TrustedLocations[i]);
			bool match = 0;

			if (url.GetHostNameLength() < len) {
				// can't be a match
				continue;
			}

			--len; // skip the '*'
			match = strncmp(W2A(url.GetHostName()) + (url.GetHostNameLength() - len),	// anchor the comparison to the end of the domain name
							TrustedLocations[i] + 1,									// skip the '*'
							len) == 0 ? true : false;
			if (match) {

				return true;
			}
		}
		else if (!strncmp(W2A(url.GetHostName()), TrustedLocations[i], url.GetHostNameLength())) {

			return true;
		}
	}

	log(instance, "AxHost.VerifySiteLock: current location is not trusted");
	return false;
}

/* 
 * Create a new plugin instance, most probably through an embed/object HTML 
 * element.
 *
 * Any private data we want to keep for this instance can be saved into 
 * [instance->pdata].
 * [saved] might hold information a previous instance that was invoked from the
 * same URL has saved for us.
 */ 
NPError 
NPP_New(NPMIMEType pluginType,
        NPP instance, uint16 mode,
        int16 argc, char *argn[],
        char *argv[], NPSavedData *saved)
{
	NPError rc = NPERR_NO_ERROR;
	NPObject *browser = NULL;
	CAxHost *host = NULL;
	PropertyList events;
	int16 i = 0;
	USES_CONVERSION;

	if (!instance) {

		return NPERR_INVALID_PARAM;
	}

	// Verify that we're running from a trusted location
	if (!VerifySiteLock(instance)) {

		return NPERR_GENERIC_ERROR;
	}

	instance->pdata = NULL;

	// TODO: Check the pluginType to make sure we're being called with the rigth MIME Type

	do {
		// Create a plugin instance, the actual control will be created once we 
		// are given a window handle
		host = new CAxHost(instance);
		if (!host) {

			rc = NPERR_OUT_OF_MEMORY_ERROR;
			log(instance, "AxHost.NPP_New: failed to allocate memory for a new host");
			break;
		}

		// Iterate over the arguments we've been passed
		for (i = 0; 
			 (i < argc) && (NPERR_NO_ERROR == rc); 
			 ++i) {

			// search for any needed information: clsid, event handling directives, etc.
			if (0 == strnicmp(argn[i], PARAM_CLSID, strlen(PARAM_CLSID))) {
				// The class id of the control we are asked to load
				host->setClsID(argv[i]);
			}
			else if (0 == strnicmp(argn[i], PARAM_PROGID, strlen(PARAM_PROGID))) {
				// The class id of the control we are asked to load
				host->setClsIDFromProgID(argv[i]);
			}
			else if (0 == strnicmp(argn[i], PARAM_ONEVENT, strlen(PARAM_ONEVENT))) {
				// A request to handle one of the activex's events in JS
				events.AddOrReplaceNamedProperty(A2W(argn[i] + strlen(PARAM_ONEVENT)), CComVariant(A2W(argv[i])));
			}
			else if (0 == strnicmp(argn[i], PARAM_PARAM, strlen(PARAM_PARAM))) {

				CComBSTR paramName = argn[i] + strlen(PARAM_PARAM);
				CComBSTR paramValue(A2W(argv[i]));

				// Check for existing params with the same name
				BOOL bFound = FALSE;
				for (unsigned long j = 0; j < host->Props.GetSize(); j++) {

					if (wcscmp(host->Props.GetNameOf(j), (BSTR) paramName) == 0) {

						bFound = TRUE;
						break;
					}
				}
				// If the parameter already exists, don't add it to the
				// list again.
				if (bFound) {

					continue;
				}

				// Add named parameter to list
				CComVariant v(paramValue);
				host->Props.AddNamedProperty(paramName, v);
			}
		}

		if (NPERR_NO_ERROR != rc)
			break;

		// Make sure we have all the information we need to initialize a new instance
		if (!host->hasValidClsID()) {

			rc = NPERR_INVALID_PARAM;
			log(instance, "AxHost.NPP_New: no valid CLSID or PROGID");
			break;
		}

		instance->pdata = host;

		if (!host->CreateControl()) {

			rc = NPERR_GENERIC_ERROR;
			log(instance, "AxHost.NPP_New: failed to create the control");
			break;
		}

		for (unsigned int j = 0; j < events.GetSize(); j++) {

			if (!host->AddEventHandler(events.GetNameOf(j), events.GetValueOf(j)->bstrVal)) {

				//rc = NPERR_GENERIC_ERROR;
				//break;
			}
		}

		if (NPERR_NO_ERROR != rc)
			break;
	} while (0);

	if (NPERR_NO_ERROR != rc) {

		delete host;
	}

	return rc;
}

/*
 * Destroy an existing plugin instance.
 *
 * [save] can be used to save information for a future instance of our plugin
 * that'll be invoked by the same URL.
 */
NPError 
NPP_Destroy(NPP instance, NPSavedData **save)
{
	if (!instance || !instance->pdata) {

		return NPERR_INVALID_PARAM;
	}

	CAxHost *host = (CAxHost *)instance->pdata;
	delete host;
	instance->pdata = NULL;

	return NPERR_NO_ERROR;
}

/*
 * Sets an instance's window parameters.
 */
NPError 
NPP_SetWindow(NPP instance, NPWindow *window)
{
	CAxHost *host = NULL;
	RECT rcPos;

	if (!instance || !instance->pdata) {

		return NPERR_INVALID_PARAM;
	}

	host = (CAxHost *)instance->pdata;

	host->setWindow((HWND)window->window);

	rcPos.left = window->clipRect.left;
	rcPos.top = window->clipRect.top;
	rcPos.right = window->clipRect.right;
	rcPos.bottom = window->clipRect.bottom;
	host->UpdateRect(rcPos);

	return NPERR_NO_ERROR;
}
