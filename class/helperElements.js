

SiteFusion.Classes.Grippy = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULGrippy',
	
	initialize: function( win ) {
		this.element = win.createElement( 'grippy' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ArrowScrollBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULArrowScrollBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'arrowscrollbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

