SiteFusion.Classes.SFImageLib = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'SFImageLib',
	
	initialize: function( win ) {
		this.element = win.createElement( 'label' );
		this.element.sfNode = this;
		try {
	    	this.obj = Components.classes["@sitefusion/sfimagelib;1"].createInstance();
	    	this.obj = this.obj.QueryInterface(Components.interfaces.ISFImageLib);
	    
			this.hostWindow = win;
			this.storedSelection = [];
			
			this.setEventHost( [
				'afterImageLoaded',
				'SFImageError',
				'afterImageResampled',
				'afterImageSaved',
				'setImageDetails',
				'createTempPath'
				] );
		
			this.eventHost.afterImageLoaded.msgType = 0;
			this.eventHost.SFImageError.msgType = 0;
			this.eventHost.afterImageResampled.msgType = 0;
			this.eventHost.afterImageSaved.msgType = 0;
			this.eventHost.setImageDetails.msgType = 1;
		}
		catch (e) {
			var em = Components.classes["@mozilla.org/extensions/manager;1"].createInstance(Components.interfaces.nsIExtensionManager);
			var PromptService = Cc["@mozilla.org/embedcomp/prompt-service;1"].getService(Ci.nsIPromptService);
			em.disableItem( 'sfimagelib@sitefusion.org' );
			PromptService.alert( window, "Client-side error", 'An error occured while loading a client-side extension. This application will now disable the extension and restart.' );
			System.Restart()
		}
	},
	
	getTempFilePath: function()
	{
		path = this.path;
		var suffix = path.slice(path.lastIndexOf(".")).toLowerCase();
    var b = path.replace(/^.*[\/\\]/g, '');
    if (typeof(suffix) == 'string' && b.substr(b.length-suffix.length).toLowerCase() == suffix) {
        b = b.substr(0, b.length-suffix.length);
    }
    
    basename = b;
    
		var file = Components.classes["@mozilla.org/file/directory_service;1"].
                     getService(Components.interfaces.nsIProperties).
                     get("TmpD", Components.interfaces.nsIFile);
                     
		file.append(basename);
		file.createUnique(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 0666);

		var ret = file.path;
 		var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    file.initWithPath(ret);

		if (file.exists())
		{
			file.remove(false);
		}
		return ret;
	},
	
	cleanTemp: function ()
	{
 		var file = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    file.initWithPath(this.temppath);

		if (file.exists())
		{
			file.remove(false);
		}
	},
	
	loadImage: function (path)
	{
		if (!this.obj)
			return;

		if (this.obj.Load(path))
		{
			this.retrieveImageDetails('afterImageLoaded');
			this.fireEvent('createTempPath');
		}
		else this.fireEvent ( 'SFImageError', ['loadImage'] );
		
	},
	
	resample: function(width, height, mode)
	{
		if (!this.obj)
			return;
			
		if (!mode)
			mode = 4;
			
		if (this.obj.Resample(width, height, mode))
		{
			this.retrieveImageDetails('afterImageResampled');
		}
		else this.fireEvent ( 'SFImageError', ['resample'] );
	},
	
	saveImage: function(path, type)
	{ 
		if (!this.obj)
			return;
			
		if (this.obj.Save(path, type))
		{
			this.retrieveImageDetails('afterImageSaved');
		}
		else this.fireEvent ( 'SFImageError', ['saveImage'] );
	},
	
	retrieveImageDetails: function (event)
	{
		this.fireEvent ( 'setImageDetails', [event, this.obj.GetWidth(), this.obj.GetHeight(), this.obj.GetFormat() ] );
	}
	
});