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


