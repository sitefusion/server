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

