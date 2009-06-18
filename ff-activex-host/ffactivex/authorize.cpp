#include <stdio.h>

#include <windows.h>
#include <winreg.h>

#include "ffactivex.h"
#include "common/stdafx.h"
#include "axhost.h"
#include "atlutil.h"

#include "authorize.h"

// ----------------------------------------------------------------------------
#define SUB_KEY L"SOFTWARE\\MozillaPlugins\\@itstructures.com/ffactivex\\MimeTypes\\application/x-itst-activex"

// ----------------------------------------------------------------------------

BOOL TestExplicitAuthorizationUTF8 (const char *DocumentUrl,
								    const char *ProgramId);

BOOL TestExplicitAuthorization (const wchar_t *DocumentUrl,
                                const wchar_t *ProgramId);


BOOL WildcardMatch (const wchar_t *Mask,
                    const wchar_t *Value);

// ----------------------------------------------------------------------------

BOOL TestAuthorization (NPP Instance,
						int16 ArgC,
						char *ArgN[],
						char *ArgV[])
{
  BOOL ret = FALSE;
  NPObject *globalObj = NULL;
  NPIdentifier identifier;
  NPVariant varLocation;
  NPVariant varHref;
  bool rc = false;
  int16 i;

#ifdef NDEF
  _asm{int 3};
#endif
  if (Instance == NULL) {
    return (FALSE);
    }

  // Determine owning document
  // Get the window object.
  NPNFuncs.getvalue(Instance, 
  	                NPNVWindowNPObject, 
  				    &globalObj);

  // Create a "location" identifier.
  identifier = NPNFuncs.getstringidentifier("location");

  // Get the location property from the window object (which is another object).
  rc = NPNFuncs.getproperty(Instance, 
  	                        globalObj, 
						    identifier, 
						    &varLocation);

  NPNFuncs.releaseobject(globalObj);
 
  if (!rc){
    log(Instance, "AxHost.VerifySiteLock: could not get the location from the global object");
    return false;
    }

  // Get a pointer to the "location" object.
  NPObject *locationObj = varLocation.value.objectValue;

  // Create a "href" identifier.
  identifier = NPNFuncs.getstringidentifier("href");

  // Get the location property from the location object.
  rc = NPNFuncs.getproperty(Instance, 
  	                        locationObj, 
						    identifier, 
						    &varHref);

  NPNFuncs.releasevariantvalue(&varLocation);

  if (!rc) {

    log(Instance, "AxHost.VerifySiteLock: could not get the href from the location property");
    return false;
    }

  ret = TRUE;

  for (i = 0; 
       i < ArgC; 
       ++i) {

	// search for any needed information: clsid, event handling directives, etc.
	if (0 == strnicmp(ArgN[i], PARAM_CLSID, strlen(PARAM_CLSID))) {
		    
	  ret &= TestExplicitAuthorizationUTF8(varHref.value.stringValue.utf8characters,
	                                    ArgV[i]);

	  } else if (0 == strnicmp(ArgN[i], PARAM_PROGID, strlen(PARAM_PROGID))) {
	  // The class id of the control we are asked to load
	  ret &= TestExplicitAuthorizationUTF8(varHref.value.stringValue.utf8characters,
	  	                                   ArgV[i]);
	  } else if( 0  == strnicmp(ArgN[i], PARAM_CODEBASEURL, strlen(PARAM_CODEBASEURL))) {
	  ret &= TestExplicitAuthorizationUTF8(varHref.value.stringValue.utf8characters,
		                                   ArgV[i]);		
	  }
	}
  NPNFuncs.releasevariantvalue(&varHref);
  return (ret);
  }

// ----------------------------------------------------------------------------

BOOL TestExplicitAuthorizationUTF8(const char *DocumentUrl,
								   const char *ProgramId)
{
  USES_CONVERSION;
  BOOL ret;
  
  ret = TestExplicitAuthorization(A2W(DocumentUrl),
	                              A2W(ProgramId));

  return (ret);
  }

// ----------------------------------------------------------------------------

BOOL TestExplicitAuthorization (const wchar_t *DocumentUrl,
                                const wchar_t *ProgramId)
{
  HKEY hKey;
  HKEY hSubKey;
  BOOL ret = FALSE;
  ULONG i;
  ULONG j;
  ULONG keyNameLen;
  ULONG valueNameLen;
  wchar_t keyName[_MAX_PATH];
  wchar_t valueName[_MAX_PATH];

#ifndef NO_REGISTRY_AUTHORIZE
  
  if (DocumentUrl == NULL) {
    return (FALSE);
    }
  
  if (ProgramId == NULL) {
    return (FALSE);
    }
  
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                   SUB_KEY,
                   0,
                   KEY_ENUMERATE_SUB_KEYS,
                   &hKey) == ERROR_SUCCESS) {
                   
    for (i = 0;
         !ret;
         i++) {
         
      keyNameLen = sizeof(keyName);
         
      if (RegEnumKey(hKey,
                     i,
                     keyName,
                     keyNameLen) == ERROR_SUCCESS) {
                     
        if (WildcardMatch(keyName,
                          DocumentUrl)) {
                  
          if (RegOpenKeyEx(hKey,
                           keyName,
                           0,
                           KEY_QUERY_VALUE,
                           &hSubKey) == ERROR_SUCCESS) {
            for (j = 0;
                 ;
                 j++) {
         
              valueNameLen = sizeof(valueName);
                 
              if (RegEnumValue(hSubKey,  
                               j,
                               valueName,
                               &valueNameLen,
                               NULL,
                               NULL,
                               NULL,
                               NULL) != ERROR_SUCCESS) {
                break;                               
                }     
              if (WildcardMatch(valueName,
                                ProgramId)) {
                ret = TRUE;
                break;
                }                        
                 
              }                 
            RegCloseKey(hSubKey);
            }
          if (ret) {
            break;
            }
          }
	    } else {
	    break;
	    }
         
      }      
    RegCloseKey(hKey);
    }                   
#endif
  
  return (ret);
  }
// ----------------------------------------------------------------------------

BOOL WildcardMatch (const wchar_t *Mask,
                    const wchar_t *Value)
{
  size_t i;
  size_t j = 0;
  size_t maskLen;
  size_t valueLen;

  maskLen = wcslen(Mask);
  valueLen = wcslen(Value);
  
  for (i = 0;
       i < maskLen + 1;
       i++) {
          
    if (Mask[i] == '?') {
      j++;
      continue;
      }
    
    if (Mask[i] == '*') {
      for (;
           j < valueLen + 1;
           j++) {
       if (WildcardMatch(Mask + i + 1,
                         Value + j)) {
         return (TRUE);
         }
       }
      return (FALSE);
      }
    
    if ((j <= valueLen) &&
        (Mask[i] == tolower(Value[j]))) {
      j++;
      continue;
      }
    
    return (FALSE);
    }
   return (TRUE);
   }  