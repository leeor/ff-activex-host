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

#pragma once

#include "npapi.h"
#include <npfunctions.h>
#include <prtypes.h>

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlctl.h>
#include <varargs.h>

#include "variants.h"

#include "common/PropertyList.h"
#include "common/PropertyBag.h"
#include "common/ItemContainer.h"
#include "common/ControlSite.h"
#include "common/ControlSiteIPFrame.h"
#include "common/ControlEventSink.h"

extern NPNetscapeFuncs NPNFuncs;

//#define NO_REGISTRY_AUTHORIZE 

static const char PARAM_CLSID[] = "clsid";
static const char PARAM_PROGID[] = "progid";
static const char PARAM_DEBUG[] = "debugLevel";
static const char PARAM_LOGGER[] = "logger";
static const char PARAM_CODEBASEURL [] = "codeBaseUrl";
static const char PARAM_ONEVENT[] = "Event_";
static const char PARAM_PARAM[] = "PARAM_";

void *ffax_calloc(unsigned int size);
void ffax_free(void *ptr);
void log(NPP instance, unsigned int level, char *message, ...);
NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char *argn[], char *argv[], NPSavedData *saved);
NPError NPP_Destroy(NPP instance, NPSavedData **save);
NPError NPP_SetWindow(NPP instance, NPWindow *window);

