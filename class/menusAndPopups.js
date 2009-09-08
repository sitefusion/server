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


SiteFusion.Classes.MenuBar = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuBar',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menubar' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuPopup = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuPopup',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menupopup' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuSeparator = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuSeparator',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menuseparator' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Menu = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenu',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menu' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuItem = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuItem',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menuitem' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.PopupSet = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPopupSet',
	
	initialize: function( win ) {
		this.element = win.createElement( 'popupset' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Popup = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPopup',
	
	initialize: function( win ) {
		this.element = win.createElement( 'popup' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ToolTip = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULToolTip',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tooltip' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Panel = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPanel',
	
	initialize: function( win ) {
		this.element = win.createElement( 'panel' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


