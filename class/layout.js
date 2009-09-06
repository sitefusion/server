

SiteFusion.Classes.Box = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'box' );
		this.element.sfNode = this;
	
		this.setEventHost();
	}
} );


SiteFusion.Classes.HBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULHBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'hbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.VBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULVBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'vbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.BBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULBBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'bbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ScrollBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULScrollBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'scrollbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	},
	
	scrollTo: function( x, y ) {
		var xpcomInterface = this.element.boxObject.QueryInterface(Ci.nsIScrollBoxObject);
		xpcomInterface.scrollTo( x, y );
	},
	
	ensureElementIsVisible: function( node ) {
		var xpcomInterface = this.element.boxObject.QueryInterface(Ci.nsIScrollBoxObject);
		xpcomInterface.ensureElementIsVisible(node.element);
	}
} );


SiteFusion.Classes.HTMLBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULHTMLBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'browser' );
		this.element.sfNode = this;
		this.element.setAttribute( 'src', 'about:blank' );
		
		this.setEventHost();
	},
	
	setContent: function( html ) {
		if( ! this.element.contentDocument ) {
			var oThis = this;
			setTimeout( function() { oThis.setContent( html ); }, 10 );
			return;
		}
		
		this.element.contentDocument.open();
		this.element.contentDocument.write( html );
		this.element.contentDocument.close();
	}
} );


SiteFusion.Classes.Stack = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULStack',
	
	initialize: function( win ) {
		this.element = win.createElement( 'stack' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Deck = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULDeck',
	
	initialize: function( win ) {
		this.element = win.createElement( 'deck' );
		this.element.sfNode = this;
		
		this.setEventHost();
	},
	
	selectedPanel: function( panel ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedPanel = panel.element; } );
	}
} );


SiteFusion.Classes.Grid = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULGrid',
	
	initialize: function( win ) {
		this.element = win.createElement( 'grid' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Columns = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULColumns',
	
	initialize: function( win ) {
		this.element = win.createElement( 'columns' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Rows = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULRows',
	
	initialize: function( win ) {
		this.element = win.createElement( 'rows' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Column = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULColumn',
	
	initialize: function( win ) {
		this.element = win.createElement( 'column' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Row = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULRow',
	
	initialize: function( win ) {
		this.element = win.createElement( 'row' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

