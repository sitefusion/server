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


SiteFusion.Classes.KeySet = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULKeySet',
	
	initialize: function( win ) {
		this.element = win.createElement( 'keyset' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Key = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULKey',
	
	initialize: function( win ) {
		this.hostWindow = win;
		this.element = win.createElement( 'key' );
		this.element.sfNode = this;
		
		this.setEventHost();
		
		this.element.setAttribute( 'oncommand', 'return true;' );
	}
} );


SiteFusion.Classes.Trigger = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'Trigger',
	
	initialize: function( iter, delay ) {
		this.iterations = iter;
		this.delay = delay;
	
		this.element = document.createElement( 'box' );
	
		var oThis = this;
		
		if( iter == 0 )
			this.timer = setInterval( function() { oThis.fireEvent( 'yield' ); }, delay );
		else
			this.timer = setInterval( function() { oThis.fireEvent( 'yield' ); if( --oThis.iterations == 0 ) clearInterval(oThis.timer); }, delay );

		this.setEventHost( [ 'yield' ] );
	},
	
	cancel: function() {
		clearInterval( this.timer );		
	}
} );


SiteFusion.Classes.PrefService = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'PrefService',
	
	initialize: function( win ) {
		this.hostWindow = win;
		this.element = win.createElement( 'box' );
		this.element.hidden = true;
	
		this.prefService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);

		this.setEventHost( ['yield']);
		this.eventHost.yield.msgType = 1;
		this.setEventHost();
	},
	
	yield: function(prefTypes) {
		var ret = {};
		try {
			for (var pref in this.prefTypes) {
				switch(this.prefTypes[pref]) {
					case "int":
					var retval = this.getIntPref(pref);
					if (typeof ret === "number") {
						ret[pref] = retval;
					}
					break;
					case "string":
					var retval = this.getCharPref(pref);
					if (typeof retval === "string") {
						ret[pref] = retval;
					}
					break;
					case "bool":
					var retval = this.getBoolPref(pref);
					if (retval != -1) {
						ret[pref] = retval;
					}
					break;
				}
			}
		}
		catch (e) {
			alert('error: ' + e);	
		}
		this.fireEvent( 'yield', [ ret ] );
	},
	
		
	setIntPref: function(pref, value) {
		this.prefService.setIntPref(pref, value);
	},
	getIntPref: function(pref) {
		if( this.prefService.getPrefType(pref) == this.prefService.PREF_INT ) {
			return this.prefService.getIntPref(pref);
		}
		else return false;
	},
	setCharPref: function(pref, value) {
		this.prefService.setCharPref(pref, value);
	},
	getCharPref: function(pref) {
		if( this.prefService.getPrefType(pref) == this.prefService.PREF_STRING ) {
			return this.prefService.getCharPref(pref);
		}
		else return false;
		
	},
	setBoolPref: function(pref, value) {
		this.prefService.setBoolPref(pref, value);
	},
	getBoolPref: function(pref) {
		if( this.prefService.getPrefType(pref) == this.prefService.PREF_BOOL ) {
			return this.prefService.getBoolPref(pref);
		}
		else return -1;
	}
	
} );