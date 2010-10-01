SiteFusion.Classes.VlcPlayer = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'VlcPlayer',
	
	initialize: function(win, url) {
		this.element = document.createElementNS( 'http://www.w3.org/1999/xhtml', 'html:embed');
		this.element.setAttribute('type', 'application/x-vlc-plugin');
		this.element.setAttribute('version', 'VideoLAN.VLCPlugin.2');
		this.win = win;
		this.url = url;
		this.initialized = false;
		this.setEventHost( [ 'stateChange'] );
		this.eventHost.stateChange.msgType = 0;
		
		this.prevState = -1;
		
	},
	afterinit: function() {
		if (this.url)
		{
			this.add(this.url);
			var oThis = this;
			window.setTimeout(function () { oThis.play();}, 100);
		}
	},
	play: function() {
		this.element.playlist.play();
		if (this.prevState == -1)
			this.cycleStatusCheck();
	},
	
	togglePause: function()	{
		this.element.playlist.togglePause();
	},
	stop: function() {
		this.element.playlist.stop();
	},
	
	add: function(src) {
		this.element.playlist.add(src);
	},
	
	cycleStatusCheck: function() {
		var oThis = this;
		if (this.win.isClosing) return;
		
		try {
			if (this.prevState != this.element.input.state)
			{
				this.prevState = this.element.input.state;
				this.fireEvent("stateChange", [this.element.input.state]);
			}
		
			window.setTimeout(function () { oThis.cycleStatusCheck();}, 100);
		}
		catch(e) {alert(e);}
	},
	
	sizeToContent: function()	{
			//we can only size when there's video
			if (this.element.input.hasVout)
			{
				this.element.width = this.element.video.width;
				this.element.height = this.element.video.height;
				
				return true;
			}
			else return false;
	}
	
});