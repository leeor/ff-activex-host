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
 * The Original Code is RSJ Software GmbH code.
 *
 * The Initial Developer of the Original Code is
 * RSJ Software GmbH.
 * Portions created by the Initial Developer are Copyright (C) 2009
 * the Initial Developer. All Rights Reserved.
 *
 * Contributors:
 *                 Ruediger Jungbeck <ruediger.jungbeck@rsj.de>
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

#include <stdio.h>

#include <windows.h>
#include <winbase.h>

// ----------------------------------------------------------------------------

int main (int ArgC,
          char *ArgV[])
{
  const char *sourceName;  
  const char *targetName;
  HANDLE targetHandle;
  void *versionPtr;
  DWORD versionLen;
  int lastError;
  int ret = 0;

  if (ArgC < 3) {
    fprintf(stderr,
            "Usage: %s <source> <target>\n",
            ArgV[0]);

    exit (1);
    }

  sourceName = ArgV[1];
  targetName = ArgV[2];

  if ((versionLen = GetFileVersionInfoSize(sourceName,
                                           NULL)) == 0) {
    fprintf(stderr,
            "Could not retrieve version len from %s\n",
            sourceName);

    exit (2);
    }

  if ((versionPtr = calloc(1,
                           versionLen)) == NULL) {

    fprintf(stderr,
            "Error allocating temp memory\n");

    exit (3);
    }

  if (!GetFileVersionInfo(sourceName,
                          NULL,
                          versionLen,
                          versionPtr)) {

    fprintf(stderr,
            "Could not retrieve version info from %s\n",
            sourceName);

    exit (4);
    }

  if ((targetHandle = BeginUpdateResource(targetName,
                                          FALSE)) == INVALID_HANDLE_VALUE) {

    fprintf(stderr,
            "Could not begin update of %s\n",
            targetName);

    free(versionPtr);

    exit (5);
    }

 
  if (!UpdateResource(targetHandle,
                      RT_VERSION, 
                      MAKEINTRESOURCE(VS_VERSION_INFO),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
                      versionPtr,
                      versionLen)) {

    lastError = GetLastError();

    fprintf(stderr,
            "Error %d updating resource\n",
            lastError);

    ret = 6;
    }

  if (!EndUpdateResource(targetHandle,
                         FALSE)) {
    fprintf(stderr,
            "Error finishing update\n");

    ret = 7;
    }

  free(versionPtr);
    
  return (ret);
  }

