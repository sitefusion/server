

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

