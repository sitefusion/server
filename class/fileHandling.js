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


SiteFusion.Classes.FilePicker = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULFilePicker',
	
	initialize: function ( win, title, mode, defaultString ) {
		this.element = win.createElement( 'label' );
		this.setEventHost( [ 'yield' ] );
		this.eventHost.yield.msgType = 0;
		
		var nsIFilePicker = Ci.nsIFilePicker;

		switch ( mode ) {
			case 'open': this.mode = nsIFilePicker.modeOpen; break;
			case 'save': this.mode = nsIFilePicker.modeSave; break;
			case 'getfolder': this.mode = nsIFilePicker.modeGetFolder; break;
			case 'openmultiple': this.mode = nsIFilePicker.modeOpenMultiple; break;
		}

		this.filters = new Array();
		this.title = title;
		this.defaultString = defaultString;
		this.win = win;
	},
	
	open: function() {
		var nsIFilePicker = Ci.nsIFilePicker;
		
		var fp = Cc["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
		fp.defaultString = this.defaultString;

		for (var n=0; n < this.filters.length; n++)
		{
			fp.appendFilter(this.filters[n][0], this.filters[n][1]);
		}
		fp.init(this.win.windowObject, this.title, this.mode);
		var res = fp.show();
		
		if( res == nsIFilePicker.returnCancel ) {
			this.fireEvent( 'yield', [ 'cancel', null ] );
		}
		if( res == nsIFilePicker.returnOK ) {
			if( this.mode == nsIFilePicker.modeOpenMultiple ) {
				var files = [ 'ok' ];
				var enum = fp.files;
				
				while( enum.hasMoreElements() ) {
					var file = enum.getNext().QueryInterface(Ci.nsIFile);
					files.push( file.path );
				}
				
				this.fireEvent( 'yield', files );
			}
			else {
				this.fireEvent( 'yield', [ 'ok', fp.file.path ] );
			}
		}
		if( res == nsIFilePicker.returnReplace ) {
			this.fireEvent( 'yield', [ 'replace', fp.file.path ] );
		}
	},
	
	addFilter: function (description, value) {
		this.filters.push([description,value]);
	}
} );


SiteFusion.Classes.FileUploader = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'FileUploader',
	
	initialize: function() {
		this.element = document.createElement( 'label' );
	
		this.file = null;
		this.binary = null;
		this.httpRequest = null;
		
		this.setEventHost( [ 'started', 'failed', 'finished', 'cycle', 'cancelled' ] );
		
		this.eventHost.cycle.msgType = 0;
		this.eventHost.cancelled.msgType = 0;
		this.eventHost.finished.msgType = 0;
	},
	
	startUpload: function( path ) {
		// request local file read permission
		try {
			netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
		} catch (e) {
			SiteFusion.Error("Can't get read permissions");
		}
		
		this.path = path;
		
		// open the local file
		var file = Components.classes["@mozilla.org/file/local;1"]
			.createInstance(Components.interfaces.nsILocalFile);
		file.initWithPath( path );
		var stream = Components.classes["@mozilla.org/network/file-input-stream;1"]
			.createInstance(Components.interfaces.nsIFileInputStream);
		stream.init(file,	0x01, 00004, null);
		var bstream =  Components.classes["@mozilla.org/network/buffered-input-stream;1"]
			.getService();
		bstream.QueryInterface(Components.interfaces.nsIBufferedInputStream);
		bstream.init(stream, 1000);
		bstream.QueryInterface(Components.interfaces.nsIInputStream);
		this.binary = Components.classes["@mozilla.org/binaryinputstream;1"]
			.createInstance(Components.interfaces.nsIBinaryInputStream);
		this.binary.setInputStream (stream);
		
		this.fileSize = this.binary.available();
		this.progress = 0;
		this.cancelled = false;
		
		if( this.fileSize < 100000 )
			this.chunkSize = 10240;
		else if( this.fileSize < 500000 )
			this.chunkSize = 51200;
		else if( this.fileSize < 1000000 )
			this.chunkSize = 102400;
		else
			this.chunkSize = 512000;
		
		this.cycle = 0;
		
		this.fireEvent( 'started', [ path ] );
	
		var oThis = this;
		
		this.timer = window.setTimeout(
			function() {
				oThis._uploadCycle();
			},
		100 );
	},

	_uploadCycle: function() {
		if( this.cancelled ) {
			this.binary.close();
			return;
		}
		
		// request more permissions
		try {
			netscape.security.PrivilegeManager.enablePrivilege("UniversalXPConnect");
		} catch (e) {
			alert("Kan geen rechten verkrijgen voor het lezen van bestanden.");
		}
		
		var data, len;
		
		if( this.binary.available() < this.chunkSize )
			len = this.binary.available();
		else
			len = this.chunkSize;
	
		data = btoa(this.binary.readBytes(len));
		this.progress += len;
		this.cycle++;
		
		var transmission = this.fireEvent( 'cycle', [ this.fileSize, this.progress, this.cycle, data ] );
		
		if( this.binary.available() ) {
			var oThis = this;
			transmission.onstatechange = function() { if( this.state == this.STATE_FINISHED ) oThis._uploadCycle(); };
		}
		else {
			this.binary.close();
			this.fireEvent( 'finished', [ this.path ] );
		}
	},

	cancelUpload: function() {
		if( this.timer )
			window.clearTimeout( this.timer );
		
		this.cancelled = true;
		this.fireEvent( 'cancelled', [ this.path ] );
	}
} );


SiteFusion.Classes.FileDownloader = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'FileDownloader',
	
	initialize: function() {
		this.element = document.createElement( 'label' );
	
		this.setEventHost();
	},
	
	startDownload: function( localPath ) {
		var d = new Date();
		var httpLoc = SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime();
	
		try {
			//new obj_URI object
			var obj_URI = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService).newURI(httpLoc, null, null);
	
			//new file object
			var obj_TargetFile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
	
			//set file with path
			obj_TargetFile.initWithPath( localPath );
			//if file doesn't exist, create
			if(!obj_TargetFile.exists()) {
				obj_TargetFile.create(0x00,0644);
			}
			//new persitence object
			var obj_Persist = Cc["@mozilla.org/embedding/browser/nsWebBrowserPersist;1"].createInstance(Ci.nsIWebBrowserPersist);
	
			//save file to target
			obj_Persist.saveURI(obj_URI,null,null,null,null,obj_TargetFile);
		} catch (e) {
			SiteFusion.Error(e);
		}
	}
} );
