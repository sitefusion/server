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


SiteFusion.Classes.TabBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	},
	
	selectedTab: function( tab ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedTab = tab.element; } );
	},
	
	selectedPanel: function( panel ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedPanel = panel.element; } );
	}
} );


SiteFusion.Classes.Tabs = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabs',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabs' );
		this.element.sfNode = this;
		
		this.setEventHost();
	},
	
	selectedItem: function( tab ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedItem = tab.element; } );
	}
} );


SiteFusion.Classes.Tab = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTab',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tab' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TabPanels = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabPanels',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabpanels' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TabPanel = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabPanel',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabpanel' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.GroupBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULGroupBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'groupbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Caption = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULCaption',
	
	initialize: function( win ) {
		this.element = win.createElement( 'caption' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Spacer = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULSpacer',
	
	initialize: function( win ) {
		this.element = win.createElement( 'spacer' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );
