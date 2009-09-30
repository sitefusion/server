// - - - - - - - - - - - - - BEGIN LICENSE BLOCK - - - - - - - - - - - - -
// Version: MPL 1.1/GPL 2.0/LGPL 2.1
//
// The contents of this file are subject to the Mozilla Public License Version
// 1.1 (the "License"); you may not use this file except in compliance with
// the License. You may obtain a copy of the License at
// http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an "AS IS" basis,
// WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
// for the specific language governing rights and limitations under the
// License.
//
// The Original Code is sitefusion.sourceforge.net code.
//
// The Initial Developer of the Original Code is
// FrontDoor Media Group.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -


SiteFusion.Registry = [];
SiteFusion.Classes = {};
SiteFusion.ClientComponents = {};
SiteFusion.CommandLineArguments = {};


SiteFusion.Initialize = function() {
	SiteFusion.RootWindow = new SiteFusion.Classes.Window( window );
	
	SiteFusion.RootWindow.createEvent( 'clientInit', SiteFusion.Comm.MSG_QUEUE );
	SiteFusion.RootWindow.createEvent( 'clientComponentsInit', SiteFusion.Comm.MSG_QUEUE );
	
	SiteFusion.InitializeClient();
	SiteFusion.InitializeClientComponents();
	
	SiteFusion.RootWindow.fireEvent( 'initialized' );
};


SiteFusion.InitializeClient = function() {
	var platformInfo = {
		appCodeName: navigator.appCodeName,
		appName: navigator.appName,
		appVersion: navigator.appVersion,
		buildID: navigator.buildID,
		language: navigator.language,
		oscpu: navigator.oscpu,
		platform: navigator.platform,
		vendor: navigator.vendor,
		vendorSub: navigator.vendorSub
	};
	
	var cmdlineArgs = {};
	var query = location.search.substr(1);
	
	if( query.length ) {
		var cmdline = query.split('&');
		for( var n = 0; n < cmdline.length; n++ ) {
			var arg = cmdline[n].split('=');
			cmdlineArgs[arg[0]] = (arg[1] == 'true' ? true:arg[1]);
		}
	}
	
	SiteFusion.CommandLineArguments = cmdlineArgs;
	
	SiteFusion.RootWindow.fireEvent( 'clientInit', [ cmdlineArgs, platformInfo ] );
};


SiteFusion.InitializeClientComponents = function() {
	var compProperties = {};
	
	for( id in SiteFusion.ClientComponents ) {
		var component = SiteFusion.ClientComponents[id];
		if( !(typeof(component) == 'object' && typeof(component.AssertSelf) == 'function') ) {
			delete SiteFusion.ClientComponents[id];
			continue;
		}
		
		var status = component.AssertSelf();
		
		compProperties[id] = status;
	}
	
	SiteFusion.RootWindow.fireEvent( 'clientComponentsInit', [ compProperties ] );
};


SiteFusion.Interface = {
	ChildWindows: [],
	DeferredCallbacks: [],
	
	HandleDeferredChildAdditions: function() {
		var root = SiteFusion.Registry[0];
		this.HandleDeferredChildAdditionsRecursive( root );
		this.HandleDeferredCallbacks();
	},

	HandleDeferredChildAdditionsRecursive: function( node ) {
		node.isPainted = true;
		
		for( var n = 0; n < node.deferredSFChildren.length; n++ ) {
			var item = node.deferredSFChildren[n];
			this.HandleDeferredChildAdditionsRecursive( item[1] );

			if( item[0] == 'addChild' )
				node.directAddChild( item[1] );
			else if( item[0] == 'addChildBefore' )
				node.directAddChildBefore( item[1], item[2] );
		}

		for( var n = 0; n < node.sfChildren.length; n++ ) {
			this.HandleDeferredChildAdditionsRecursive( node.sfChildren[n] );
		}

		for( var n = 0; n < node.deferredSFChildren.length; n++ ) {
			var item = node.deferredSFChildren[n];

			if( item[0] == 'addChild' )
				node.sfChildren.push( item[1] );
			else if( item[0] == 'addChildBefore' ) {
				for( var m = 0; m < node.sfChildren.length; m++ ) {
					if( node.sfChildren[m] == item[2] ) {
						node.sfChildren.splice( m, 0, item[1] );
						break;
					}
				}
			}
		}

		node.deferredSFChildren = new Array();
	},

	HandleDeferredCallbacks: function() {
		for( var n = 0; n < this.DeferredCallbacks.length; n++ ) {
			this.DeferredCallbacks[n]();
		}

		this.DeferredCallbacks = [];
	},
	
	CursorBusy: function() {
		document.documentElement.style.cursor = 'wait';
		
		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] ) {
				if( SiteFusion.Interface.ChildWindows[n].document ) {
					if( SiteFusion.Interface.ChildWindows[n].document.documentElement )
						SiteFusion.Interface.ChildWindows[n].document.documentElement.style.cursor = 'wait';
				}
			}
		}
	},

	CursorIdle: function() {
		document.documentElement.style.cursor = '';

		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] ) {
				if( SiteFusion.Interface.ChildWindows[n].document ) {
					if( SiteFusion.Interface.ChildWindows[n].document.documentElement )
						SiteFusion.Interface.ChildWindows[n].document.documentElement.style.cursor = '';
				}
			}
		}
	},
	
	RegisterChildWindow: function( win ) {
		SiteFusion.Interface.ChildWindows.push( win );
	},
	
	UnregisterChildWindow: function( win ) {
		for( var n = 0; n < SiteFusion.Interface.ChildWindows.length; n++ ) {
			if( SiteFusion.Interface.ChildWindows[n] === win ) {
				SiteFusion.Interface.ChildWindows.splice( n, 1 );
				return;
			}
		}
	}
};

