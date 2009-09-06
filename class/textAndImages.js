

SiteFusion.Classes.Label = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULLabel',
	
	initialize: function( win, text ) {
		this.element = win.createElement( 'label' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Description = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULDescription',
	
	initialize: function( win, text ) {
		this.element = win.createElement( 'description' );
		this.element.sfNode = this;
		this.hostWindow = win;
	
		this.setEventHost();
	},
	
	value: function( val ) {
		var text = this.hostWindow.createTextNode( val );
		for( var n = 0; n < this.element.childNodes.length; n++ ) {
			this.element.removeChild( this.element.childNodes[n] );
		}
		
		this.element.appendChild( text );
	}
} );


SiteFusion.Classes.Image = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULImage',
	
	initialize: function( win, src, width, height ) {
		this.element = win.createElement( 'image' );
		this.element.sfNode = this;
		
		this.setEventHost();
		
		if( src )
			this.src( src );
		if( width )
			this.width( width );
		if( height )
			this.height( height );
	}
} );


SiteFusion.Classes.DirectImage = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULDirectImage',
	
	initialize: function( win, width, height ) {
		this.element = win.createElement( 'image' );
		this.element.sfNode = this;
		
		this.setEventHost();
		
		if( width )
			this.width( width );
		if( height )
			this.height( height );
	},
	
	recycle: function() {
		this.element.setAttribute( 'src', this.getSrc() );
	},
	
	getSrc: function() {
		var d = new Date();
		return SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime();
	}
} );


SiteFusion.Classes.FileSystemImage = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULFileSystemImage',
	
	initialize: function( win, width, height ) {
		this.element = win.createElement( 'image' );
		this.element.sfNode = this;
		
		this.setEventHost();
		
		if( width )
			this.width( width );
		if( height )
			this.height( height );
	},
	
	load: function() {
		var d = new Date();
		this.element.setAttribute( 'src', SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime() );
	}
} );

