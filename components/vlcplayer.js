SiteFusion.Classes.VlcPlayer = Class.create( SiteFusion.Classes.Embed, {
	sfClassName: 'VlcPlayer',
	
	initialize: function(win, width, height, src, target, mrl, filename, autoplay, allowfullscreen, mute, loop, toolbar, bgcolor) {
		this.element = document.createElementNS( 'http://www.w3.org/1999/xhtml', 'html:embed');
		this.element.setAttribute('type', 'application/x-vlc-plugin');
		this.element.setAttribute('pluginspage', 'http://www.videolan.org');

		this.element.setAttribute('width',width);
		this.element.setAttribute('height',height);
		
		if (src) {
			this.element.setAttribute('src',src);
		}
		if (mrl) {
			this.element.setAttribute('mrl',mrl);
		}
		if (filename) {
			this.element.setAttribute('filename',filename);
		}
		if (target) {
			this.element.setAttribute('target',target);
		}
		
		//bool
		this.element.setAttribute('autoplay',autoplay);
		
		this.element.setAttribute('allowfullscreen',allowfullscreen);
		this.element.setAttribute('mute', mute);
		this.element.setAttribute('loop',loop);
		this.element.setAttribute('toolbar',toolbar);
			
		if (bgcolor)
			this.element.setAttribute('bgcolor',bgcolor);
		
		this.win = win;
		this.initialized = false;
		this.setEventHost( [ 'stateChange'] );
		this.eventHost.stateChange.msgType = 0;
		this.prevState = -1;

	},
	
	afterinit: function(width, height) {
		
		var oThis = this;
		window.setTimeout(function () { 
					oThis.element.setAttribute('width',width-1);
					if (oThis.element.getAttribute('autoplay')) {
						//start cycle now
						oThis.cycleStatusCheck();
					}
		},100);
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
	
	add: function(src,fancyname,options) {
		var id = this.element.playlist.add(src,fancyname,options);
		this.element.playlist.playItem(id);
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
	}
	
});