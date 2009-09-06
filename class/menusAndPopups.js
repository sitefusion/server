

SiteFusion.Classes.MenuBar = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuBar',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menubar' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuPopup = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuPopup',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menupopup' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuSeparator = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuSeparator',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menuseparator' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Menu = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenu',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menu' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.MenuItem = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULMenuItem',
	
	initialize: function( win ) {
		this.element = win.createElement( 'menuitem' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.PopupSet = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPopupSet',
	
	initialize: function( win ) {
		this.element = win.createElement( 'popupset' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Popup = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPopup',
	
	initialize: function( win ) {
		this.element = win.createElement( 'popup' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ToolTip = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULToolTip',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tooltip' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Panel = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULPanel',
	
	initialize: function( win ) {
		this.element = win.createElement( 'panel' );
		this.element.setAttribute( 'type', 'created' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


