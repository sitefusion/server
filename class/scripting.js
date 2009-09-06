

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


