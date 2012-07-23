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


SiteFusion.Classes.ListBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listbox' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	selectItem: function( items ) {
		this.element.clearSelection();
		var oThis = this;
		for( var n = 0; n < items.length; n++ ) {
			var item = items[n];
			//tommyfix 23-07-2012
			if (item)
				window.setTimeout(function(){oThis.element.addItemToSelection( item.element );}, 100);
		}
	},
	
	yield: function() {
		var items = this.element.selectedItem ? new Array( this.element.selectedItem.sfNode ) : new Array( null );
		
		if( this.element.selectedItem ) {
			var selectedItems = this.element.selectedItems;
			
			for( var t = 0; t < selectedItems.length; t++ ) {
				items.push( selectedItems[t].sfNode );
			}
		}
		
		this.fireEvent( 'yield', items );
	}
} );


SiteFusion.Classes.ListItem = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListItem',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listitem' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ListCols = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListCols',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listcols' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ListCol = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListCol',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listcol' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ListHead = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListHead',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listhead' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ListHeader = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListHeader',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listheader' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ListCell = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListCell',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listcell' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.RichListBox = Class.create( SiteFusion.Classes.ListBox, {
	sfClassName: 'XULRichListBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'richlistbox' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
		
		this.eventHost.yield.msgType = 1;
	}
} );


SiteFusion.Classes.RichListItem = Class.create( SiteFusion.Classes.ListItem, {
	sfClassName: 'XULRichListItem',
	
	initialize: function( win ) {
		this.element = win.createElement( 'richlistitem' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

