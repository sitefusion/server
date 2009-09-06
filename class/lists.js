

SiteFusion.Classes.ListBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULListBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'listbox' );
		this.element.sfNode = this;
		
		this.setEventHost( [ 'yield' ] );
		
		this.eventHost.yield.msgType = 1;
	},
	
	yield: function() {
		var items = this.element.selectedItem ? new Array( this.element.selectedItem.widgetObj ) : new Array( null );
		
		if( this.element.selectedItem ) {
			var selectedItems = this.element.selectedItems;
			
			for( var t = 0; t < selectedItems.length; t++ ) {
				items.push( selectedItems[t].widgetObj );
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

