

SiteFusion.Classes.TabBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	},
	
	selectedTab: function( tab ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedTab = tab.element; } );
	},
	
	selectedPanel: function( panel ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedPanel = panel.element; } );
	}
} );


SiteFusion.Classes.Tabs = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabs',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabs' );
		this.element.sfNode = this;
		
		this.setEventHost();
	},
	
	selectedItem: function( tab ) {
		var oThis = this;
		SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedItem = tab.element; } );
	}
} );


SiteFusion.Classes.Tab = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTab',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tab' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TabPanels = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabPanels',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabpanels' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.TabPanel = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULTabPanel',
	
	initialize: function( win ) {
		this.element = win.createElement( 'tabpanel' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.GroupBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULGroupBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'groupbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Caption = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULCaption',
	
	initialize: function( win ) {
		this.element = win.createElement( 'caption' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Spacer = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULSpacer',
	
	initialize: function( win ) {
		this.element = win.createElement( 'spacer' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );
