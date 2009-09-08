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


SiteFusion.Classes.ToolBarSeparator = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULToolBarSeparator',
	
	initialize: function( win ) {
		this.element = win.createElement( 'toolbarseparator' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ToolBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULToolBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'toolbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ToolBar = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULToolBar',
	
	initialize: function( win ) {
		this.element = win.createElement( 'toolbar' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ToolBarButton = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULToolBarButton',
	
	initialize: function( win ) {
		this.element = win.createElement( 'toolbarbutton' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

