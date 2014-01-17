The contents of this file were written long ago. Most, if not all, of the
references to Firefox hold true for Chrome as well.

# Introduction

This plugin was written to try and reduce the gap between IE and FF when it comes to content that can be used in web applications. Sometimes, to create an ideal user experience you have to be able to use some native code. Many times that native code already exists in the form of an ActiveX control.

# Compatibility

This plugin has been tested with FF versions 3.0 through 3.1.`*`, and is compatible.

# Security

The lack of support for ActiveX controls in FF is definitely one of its strong point where security is concerned. This plugin does not aim to reduce the security of FF. To that end several measures have been taken:

* The plugin *will not* work on regular object tags that are used to embed ActiveX controls in IE.
* The plugin can be compiled with a list of trusted domains that will be allowed to use it. It will refuse to load ActiveX objects for any other domain.
* The plugin can be compiled with a list of trusted control CLSIDs/PROGIDs. Again, trying to use any other control will fail.

# Features

The plugin aims to provide full support for embedding ActiveX controls. Here's a list of the features provided:

* *Scriptable* - Fully scriptable from Javascript, calling methods, retrieving/setting property values, etc.
* *Parameters* - You can pass parameters to the embedded control.
* *Events* - You can bind Javascript function to events.
* *CAB Download* - You can specify a URL for downloading a CAB with the control if it does not exist.
  * The URL must match the list of trusted domain if one exists.

# Preparing the Plugin for Distribution

The source stored in the SVN repository here can be considered 'vanilla' code. It has no predefined lists of trusted sites/controls. It is provided for the mere purpose of testing and performing simple POCs. The provided XPI is not signed.

If you would like to use this plugin it is strongly recommended that you fill in these lists (trusted domains, controls), choose a unique MIME Type for your purposes, and sign the XPI. Optionally, you can also provide an auto update mechanism just like with any other FF plugin.

## Declaring the MIME Type

On windows, FF associates plugin with the MIME Type they have defined in their resource key named `MIMEType`. If you open the plugin's properties and go to the Version tab, you will have a list of resource items, `MIMEType` is one of those items.

To specify which MIME Type you want your version of this plugin to use, open ffactive.rc and edit the value of the `MIMEType` key.

## Providing a Trusted Domains List

In the file ffactivex.cpp you will find the following lines:
```
static const char *TrustedLocations[] = {NULL};
static const unsigned int numTrustedLocations = 0;
```
What they mean is that any and all sites will be allowed to use this plugin you are about to compile. Assuming you will sign your plugin and have your users install it, you do not want them complaining later when some other site takes advantage of it in any way you did not intend.

`TrustedLocations` is an array of simple C strings, each representing a domain name. A domain name may begin with an `*` as a wild card (e.g. `*`.google.com will match code.google.com). Otherwise, the complete domain name must match exactly a member of the `TrustedLocations` array.

## Providing a Well Known Control List

In the file axhost.cpp you will find the following lines:
```
static const char *WellKnownProgIds[] = {
	NULL
};

static const char *WellKnownClsIds[] = {
	NULL
};

static const bool AcceptOnlyWellKnown = false;
static const bool TrustWellKnown = false;
```

Same as with `TrustedLocations`, `WellKnownProgIds` and `WellKnownClsIds` are arrays of PROGIDs and CLSIDs respectively.

In addition, `TrustWellKnown` can be set to true in order to create the control using an unsafe for scripting interface. While this is a security issue, there might be a trade-off here between the user experience and security and it is left to you to decide how you want your plugin to behave.

The reasoning for this flag is that controls that are invoked through their safe for scripting interfaces sometimes prompt the user with different dialogues and therefore spoil an otherwise smooth and fluid user experience of your web application.

While these dialogues are important when the ActiveX control can be used from anywhere on the net, when its usage is restricted by following the previous guidelines, this trade-off might not be a significant one.

# Embedding an ActiveX in a Web Page

FF invokes plugins for rendering content by matching the MIME type of that content as specified in the `<object>` tag. Here's an example:
```
<object
	id="Control"
	TYPE="application/x-itst-activex"
	WIDTH="300" HEIGHT="300"
	clsid="{D27CDB6E-AE6D-11cf-96B8-444553540000}"
	progid="ShockwaveFlash.ShockwaveFlash"
	event_OnReadyStateChange="OnReady"
	param_src="http://www.youtube.com/v/53RdNYwImYc">
</object>
```

  * `type`: the MIME Type the plugin is associated with.
  * `clsid`: is the CLSID of the control you wish the plugin to load.
  * `progid`: is the PROGID of the control you with the plugin to load.
  * `event_XXX`: tells the plugin to bind the event `XXX` to the Javascript function given as value.
  * `param_XXX`: requests that the plugin will invoke the requested ActiveX control with the parameter named `XXX` and set its value accordingly.
  * `codeBaseURL`: may be used to provide a location from which the plugin will download and install a CAB containing the needed ActiveX control.

There is no need to provide both a `clsid` and a `progid`, the above is done only for the purpose of demonstration.

# Troubleshooting

First of all, get [Firebug](http://getfirebug.com/). Aside from being an invaluable tool for developing with FF, this plugin can print debug messages to Firebug's console.

## Error Message: failed to create site

Check that your control is marked as 'Safe for Scripting'. The only way to load a control that is not marked as safe for scripting is to make it 'well known' (refer to [Documentation#Providing_a_Well_Known_Control_List Providing a Well Known Control List]) and set the `TrustWellKnown` flag to true.

# Final Note

If you find this plugin useful and decide to use it, I'd love it if you drop me a line and tell me about it.
