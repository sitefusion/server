SiteFusion.Classes.FlashProxy = Class.create( SiteFusion.Classes.Browser, {
	sfClassName: 'XULFlashProxy',
	
	initialize: function( win ) {
		this.element = win.createElement( 'browser' );
		this.element.sfNode = this;
		this.element.setAttribute('disablehistory', true);
		var oThis = this;
		
		this.hostWindow = win;
		
		this.setEventHost( [
			'initialized',
			'ready',
			'flashReady'
		] );
		
		this.eventHost.initialized.msgType = 0;
		this.eventHost.ready.msgType = 0;
		this.eventHost.flashReady.msgType = 0;
	}
	
});