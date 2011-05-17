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


SiteFusion.Classes.Tree = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTree',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tree' );
		this.element.sfNode = this;
		this.setEventHost( [ 'yield' ] );
		var oThis = this;
	    win.windowObject.setTimeout(function() {oThis.element.columns.restoreNaturalOrder();},100);
	    
		this.eventHost.yield.msgType = 1;
	},
	
	yield: function() {
		var rows = new Array();
		var tree = this.element;
		var start = new Object();
		var end = new Object();
		
		var numRanges = tree.view.selection.getRangeCount();
		
		for( var t = 0; t < numRanges; t++ ) {
			tree.view.selection.getRangeAt( t, start, end );
			
			for( var v = start.value; v <= end.value; v++ ) {
				if( v < 0 ) v = 0;
				rows.push( tree.view.getItemAtIndex(v).sfNode );
			}
		}
		
		this.fireEvent( 'yield', rows );
	},

	select: function( item ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.view.selection.select( oThis.element.view.getIndexOfItem( item.element ) ); } );
	}
} );


SiteFusion.Classes.TreeCols = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeCols',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treecols' );
		this.element.sfNode = this;
		this.setEventHost();
	}
} );


SiteFusion.Classes.TreeCol = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeCol',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treecol' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TreeChildren = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeChildren',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treechildren' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TreeItem = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeItem',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treeitem' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

SiteFusion.Classes.TreeRow = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeRow',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treerow' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TreeCell = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeCell',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treecell' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TreeSeparator = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTreeSeparator',
	
	initialize: function( win ) {
		this.element = win.createElement( 'treeseparator' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

