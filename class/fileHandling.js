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

Components.utils.import("resource://gre/modules/FileUtils.jsm");

const PR_UINT32_MAX = 0xffffffff;

SiteFusion.Classes.FilePicker = Class.create( SiteFusion.Classes.Node, {
    sfClassName: 'XULFilePicker',

    initialize: function ( win, title, mode, defaultString, defaultExtension ) {
        this.element = win.createElement( 'label' );
        this.setEventHost( [ 'yield' ] );
        this.eventHost.yield.msgType = 0;

        var nsIFilePicker = Ci.nsIFilePicker;

        switch ( mode ) {
            case 'open':
                this.mode = nsIFilePicker.modeOpen;
                break;
            case 'save':
                this.mode = nsIFilePicker.modeSave;
                break;
            case 'getfolder':
                this.mode = nsIFilePicker.modeGetFolder;
                break;
            case 'openmultiple':
                this.mode = nsIFilePicker.modeOpenMultiple;
                break;
        }

        this.filters = new Array();
        this.title = title;
        this.defaultString = defaultString;
        this.defaultExtension = defaultExtension;
        this.win = win;
    },

    open: function() {
        var nsIFilePicker = Ci.nsIFilePicker;

        var fp = Cc["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
        fp.defaultString = this.defaultString;
        fp.defaultExtension = this.defaultExtension;

        for (var n=0; n < this.filters.length; n++)
        {
            if (this.filters[n].length == 2) {
                fp.appendFilter(this.filters[n][0], this.filters[n][1]);
            }
            else {
                fp.appendFilters(this.filters[n][0]);
            }
        }

        fp.init(this.win.windowObject, this.title, this.mode);
        var res = fp.show();

        if( res == nsIFilePicker.returnCancel ) {
            this.fireEvent( 'yield', [ 'cancel', null, null ] );
        }
        var selectedFilter = false;
        if (this.filters.length) {
            selectedFilter = this.filters[fp.filterIndex];
        }

        if( res == nsIFilePicker.returnOK ) {
            if( this.mode == nsIFilePicker.modeOpenMultiple ) {
                var files = [];
                var fEnum = fp.files;

                while( fEnum.hasMoreElements() ) {
                    var file = fEnum.getNext().QueryInterface(Ci.nsIFile);
                    files.push( file.path );
                }

                this.fireEvent( 'yield', ['ok', files, selectedFilter] );
            }
            else {
                this.fireEvent( 'yield', [ 'ok', fp.file.path, selectedFilter ] );
            }
        }
        if( res == nsIFilePicker.returnReplace ) {
            this.fireEvent( 'yield', [ 'replace', fp.file.path, selectedFilter ] );
        }
    },

    addFilter: function (description, value) {
        this.filters.push([description,value]);
    },

    addSystemFilter: function(filter) {
        this.filters.push([eval(filter)]);
    }

} );


SiteFusion.Classes.FileUploader = Class.create( SiteFusion.Classes.Node, {
    sfClassName: 'FileUploader',

    initialize: function(win) {
        this.element = document.createElement( 'label' );
        this.win = win;

        this.file = null;
        this.binary = null;
        this.httpRequest = null;

        this.setEventHost( [ 'started', 'failed', 'finished', 'cycle', 'cancelled' ] );

        this.eventHost.cycle.msgType = 0;
        this.eventHost.cancelled.msgType = 0;
        this.eventHost.finished.msgType = 0;
    },

    startUpload: function( path ) {
        this.path = path;

        // open the local file
        var file = Components.classes["@mozilla.org/file/local;1"]
        .createInstance(Components.interfaces.nsILocalFile);
        file.initWithPath( this.path );
        var stream = Components.classes["@mozilla.org/network/file-input-stream;1"]
        .createInstance(Components.interfaces.nsIFileInputStream);
        stream.init(file, 0x01, 00004, null);
        var bstream =  Components.classes["@mozilla.org/network/buffered-input-stream;1"]
        .getService();
        bstream.QueryInterface(Components.interfaces.nsIBufferedInputStream);
        bstream.init(stream, 1000);
        bstream.QueryInterface(Components.interfaces.nsIInputStream);
        this.binary = Components.classes["@mozilla.org/binaryinputstream;1"]
        .createInstance(Components.interfaces.nsIBinaryInputStream);
        this.binary.setInputStream (stream);

        this.fileSize = file.fileSize;
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
        /**
         * BUGFIX 29-7-2013: Changed window.setTimeout() to this.win.windowObject.setTimeout().
         * Fixes the bug on modal dialogs not firing the event cycle for uploading
         */
        this.timer = this.win.windowObject.setTimeout(
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

        var data, len;

        if( (this.fileSize - this.progress) < this.chunkSize )
            len = (this.fileSize - this.progress);
        else
            len = this.chunkSize;

        data = btoa(this.binary.readBytes(len));
        this.progress += len;
        this.cycle++;

        var transmission = this.fireEvent( 'cycle', [ this.fileSize, this.progress, this.cycle, data ] );

        if( this.fileSize > this.progress ) {
            var oThis = this;
            transmission.onstatechange = function() {
                if( this.state == this.STATE_FINISHED ) oThis._uploadCycle();
            };
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

        this.setEventHost( [ 'started', 'failed', 'finished', 'cycle', 'cancelled' ] );
    },

    startDownload: function( localPath, localFileIsEmpty ) {
        this.localPath = localPath;


        var progressListener = {
            stateIsRequest: false,
            lastCycle: 0,
            cycleCount: 0,
            done: false,
            QueryInterface : function(aIID) {
                if (aIID.equals(Components.interfaces.nsIWebProgressListener2) ||
                    aIID.equals(Components.interfaces.nsISupportsWeakReference) ||
                    aIID.equals(Components.interfaces.nsISupports))
                    return this;
                throw Components.results.NS_NOINTERFACE;
            },
            onStateChange: function( webProgress, request, stateFlags, status ) {},
            onProgressChange64: function( webProgress, request, curSelfProgress, maxSelfProgress, curTotalProgress, maxTotalProgress ) {
                if( curSelfProgress == maxSelfProgress ) {
                    this.done = true;
                    this.cycleCount++;
                    this.downloader.fireEvent( 'cycle', [ maxSelfProgress, curSelfProgress, this.cycleCount ] );
                    this.downloader.fireEvent( 'finished', [ this.downloader.localPath ] );
                }
                else {
                    var now = Date.now();
                    if( now - this.lastCycle > 500 ) {
                        this.cycleCount++;
                        this.downloader.fireEvent( 'cycle', [ maxSelfProgress, curSelfProgress, this.cycleCount ] );
                        this.lastCycle = now;
                    }
                }
            },
            onLocationChange: function( webProgress, request, location ) {},
            onStatusChange: function( webProgress, request, status, message ) {},
            onSecurityChange: function( webProgress, request, state ) {}
        };

        progressListener.downloader = this;

        var d = new Date();
        var httpLoc = SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime();
        try {
            this.targetFile = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
            this.targetFile.initWithPath( localPath );

            if(this.targetFile.exists() && this.targetFile.isFile() && this.targetFile.isWritable()) {
                this.targetFile.remove(false);
            }
            this.targetFile.create(0x00,0644);

            if (!localFileIsEmpty) {
                var uri = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService).newURI(httpLoc, null, null);
                this.persistObject = Cc["@mozilla.org/embedding/browser/nsWebBrowserPersist;1"].createInstance(Ci.nsIWebBrowserPersist);

                this.persistObject.progressListener = progressListener;
                var nsIWBP = Ci.nsIWebBrowserPersist;

                this.persistObject.persistFlags = nsIWBP.PERSIST_FLAGS_REPLACE_EXISTING_FILES |
                nsIWBP.PERSIST_FLAGS_BYPASS_CACHE |
                nsIWBP.PERSIST_FLAGS_FAIL_ON_BROKEN_LINKS |
                nsIWBP.PERSIST_FLAGS_CLEANUP_ON_FAILURE;

                var privacyContext = window.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                .getInterface(Components.interfaces.nsIWebNavigation)
                .QueryInterface(Components.interfaces.nsILoadContext);

                this.persistObject.saveURI(uri,null,null,null,null,this.targetFile,privacyContext);
            }
        } catch (e) {
            this.fireEvent( 'failed', [ this.localPath, e.name ] );
            return;
        }

        this.fireEvent( 'started', [ this.localPath ] );
        if (localFileIsEmpty) {
            this.fireEvent( 'finished', [ this.localPath ] );
        }
    },

    cancelDownload: function() {
        if( ! this.persistObject )
            return;

        if( (!this.persistObject.progressListener) || this.persistObject.progressListener.done )
            return;

        this.persistObject.cancelSave();
        try {
            this.targetFile.remove(false);
        }
        catch ( e ) {}

        this.fireEvent( 'cancelled', [ this.localPath ] );
    }
} );


SiteFusion.Classes.FileService = Class.create( SiteFusion.Classes.Node, {
    sfClassName: 'FileService',

    initialize: function ( win ) {
        this.element = win.createElement( 'label' );
        this.setEventHost( [ 'result' ] );
        this.eventHost.yield.msgType = 0;

        this.hostWindow = win;

        this.monitors = [];
    },

    getDirectory: function( path, retrieveContents ) {
        var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        file.initWithPath(path);
        if( (!file.exists()) || (!file.isDirectory()) ) {
            this.fireEvent( 'result', [ 'list', path, false, file.path ] );
            return;
        }

        var entries = file.directoryEntries;
        var base = this.resultFromFile( file );
        if (retrieveContents) {
            var array = [];
            while(entries.hasMoreElements()) {
                var entry = entries.getNext();
                entry.QueryInterface(Components.interfaces.nsIFile);
                array.push( this.resultFromFile( entry ) );
            }

            this.fireEvent( 'result', [ 'list', path, true, file.path, base, array ] );
        }
        else {
            this.fireEvent( 'result', [ 'list', path, true, file.path, base ] );
        }
    },

    getSpecialDirectory: function( id, retrieveContents ) {
        try {
            var file = Cc["@mozilla.org/file/directory_service;1"].getService(Ci.nsIProperties).get(id, Ci.nsIFile);
            file.initWithPath(file.path);
            
            if( (!file.exists()) || (!file.isDirectory()) ) {
                this.fireEvent( 'result', [ 'list', id, false, file.path ] );
                return;
            }
            var entries = file.directoryEntries;
            var base = this.resultFromFile( file );

            if (retrieveContents) {
                var array = [];
                while(entries.hasMoreElements()) {
                    var entry = entries.getNext();
                    entry.QueryInterface(Components.interfaces.nsIFile);
                    array.push( this.resultFromFile( entry ) );
                }

                this.fireEvent( 'result', [ 'list', id, true, file.path, base, array ] );
            }
            else {
                this.fireEvent( 'result', [ 'list', id, true, file.path, base ] );
            }
        }
        catch ( e ) {
            this.fireEvent( 'result', [ 'list', id, false, null ] );
        }
    },

    createDirectory: function( path ) {
        try {
            var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
            file.initWithPath(path);
            file.create( Ci.nsIFile.DIRECTORY_TYPE, 0755 );
            var base = this.resultFromFile( file );
            this.fireEvent( 'result', [ 'createDirectory', path, true, path, base ] );
        }
        catch ( e ) {
            this.fireEvent( 'result', [ 'createDirectory', path, false, path, null ] );
        }
    },

    removeDirectory: function( path, recursive ) {
        try {
            var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
            file.initWithPath(path);
            if( file.exists() && file.isDirectory() ) {
                file.remove(recursive);
                this.fireEvent( 'result', [ 'removeDirectory', path, true, path ] );
                return;
            }
        }
        catch ( e ) {}

        this.fireEvent( 'result', [ 'removeDirectory', path, false, path ] );
    },

    removeFile: function( path ) {
        try {
            var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
            file.initWithPath(path);
            if( file.exists() && file.isFile() ) {
                file.remove(false);
                this.fireEvent( 'result', [ 'removeFile', path, true, path ] );
                return;
            }
        }
        catch ( e ) {}

        this.fireEvent( 'result', [ 'removeFile', path, false, path ] );
    },

    monitorFile: function( path ) {
        var oThis = this;
        this.monitors.push( new SiteFusion.Classes.FileService.FileMonitor( this, path ) );
    },

    cancelMonitorFile: function( path ) {
        for( var n = 0; n < this.monitors.length; n++ ) {
            if (this.monitors[n].path == path) {
                this.monitors[n].cancel();
            }
        }
    },

    cancelAllMonitors: function() {
        for( var n = 0; n < this.monitors.length; n++ ) {
            this.monitors[n].cancel();
        }
    },

    executeFile: function( path, args, async ) {
        var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        file.initWithPath(path);

        args = (args == null ? [] : args);

        if( file.exists() ) {
            var process = Cc["@mozilla.org/process/util;1"].createInstance(Ci.nsIProcess);
            process.init( file );
            if( async ) {
                var oThis = this;
                var observer = {
                    observe: function( subject, topic, data ) {
                        oThis.fireEvent( 'result', [ 'executeFile', path, (topic == 'process-finished'), path ] );
                    }
                };
                process.runAsync( args, args.length, observer );
            }
            else {
                process.run( true, args, args.length );
                this.fireEvent( 'result', [ 'executeFile', path, true, path ] );
            }
            return;
        }
        this.fireEvent( 'result', [ 'executeFile', path, false, path ] );
    },

    openFileWithNativeProtocolHandler: function (path) {
        var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        var ioService = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
        file.initWithPath(path);

        var uri = ioService.newFileURI(file);
        var protocolSvc = Cc["@mozilla.org/uriloader/external-protocol-service;1"].getService(Ci.nsIExternalProtocolService);
        protocolSvc.loadURI(uri);

    },

    openURIWithNativeProtocolHandler: function (strUri) {

        var ioService = Cc["@mozilla.org/network/io-service;1"].getService(Ci.nsIIOService);
        var uri = ioService.newURI(strUri, null, null);

        var protocolSvc = Cc["@mozilla.org/uriloader/external-protocol-service;1"].getService(Ci.nsIExternalProtocolService);
        protocolSvc.loadUrl(uri);

    },

    getFileInfo: function (path) {

        var file = new FileUtils.File(path);
        if (!file.exists()) {
            this.fireEvent( 'result', [ 'getFileInfo', path, null, path ] );
            return;
        }
        if(file.isDirectory()) {
            this.fireEvent( 'result', [ 'getFileInfo', path, false, path ] );
            return;
        }

        this.fireEvent( 'result', [ 'getFileInfo', path, true, this.resultFromFile( file ) ] );

    },

    getCryptoHash: function (path, hashType) {

        var file = new FileUtils.File(path);
        if( !file.exists() || file.isDirectory()) {
            this.fireEvent( 'result', [ 'getCryptoHash', path, false, '' ] );
            return;
        }

        var istream = Components.classes["@mozilla.org/network/file-input-stream;1"].createInstance(Components.interfaces.nsIFileInputStream);
        istream.init(file, 0x01, 0444, 0);

        var ch = Components.classes["@mozilla.org/security/hash;1"].createInstance(Components.interfaces.nsICryptoHash);
        ch.init(hashType);
        ch.updateFromStream(istream, PR_UINT32_MAX);
        var hash = ch.finish(true);

        for (var i = 0, bin = atob(hash.replace(/[ \r\n]+$/, "")), hex = []; i < bin.length; ++i) {
            var tmp = bin.charCodeAt(i).toString(16);
            if (tmp.length === 1) tmp = "0" + tmp;
            hex[hex.length] = tmp;
        }
        var output = hex.join('');

        istream.close();
        this.fireEvent( 'result', [ 'getCryptoHash', path, true, output ] );

    },

    renameFile: function (path, targetPath) {

        var file = new FileUtils.File(path);
        if( !file.exists() || file.isDirectory()) {
            this.fireEvent( 'result', [ 'renameFile', path, false, path ] );
            return;
        }

        var parentDir = new FileUtils.File(targetPath.match(/^.*[\\\/]/)[0]);
        if( !parentDir.exists() || !parentDir.isDirectory()) {
            this.fireEvent( 'result', [ 'renameFile', path, false, path ] );
            return;
        }
        
        file.moveTo(parentDir, targetPath.replace(/^.*[\\\/]/, ''));

        this.fireEvent( 'result', [ 'renameFile', path, true, targetPath ] );

    },

    copyFile: function (path, targetPath) {

        var file = new FileUtils.File(path);
        if( !file.exists() || file.isDirectory()) {
            this.fireEvent( 'result', [ 'renameFile', path, false, path ] );
            return;
        }

        var parentDir = new FileUtils.File(targetPath.match(/^.*[\\\/]/)[0]);
        if( !parentDir.exists() || !parentDir.isDirectory()) {
            this.fireEvent( 'result', [ 'renameFile', path, false, path ] );
            return;
        }
        
        file.copyTo(parentDir, targetPath.replace(/^.*[\\\/]/, ''));

        this.fireEvent( 'result', [ 'renameFile', path, true, targetPath ] );

    },

    resultFromFile: function( file ) {
        return [
        file.leafName,
        file.isDirectory(),
        file.isReadable(),
        file.isWritable(),
        file.isExecutable(),
        file.isHidden(),
        (file.isDirectory() ? null : file.fileSize),
        Math.round(file.lastModifiedTime/1000)
        ];
    }
} );


SiteFusion.Classes.FileService.FileMonitor = Class.create( {
    exists: null,
    modificationTime: null,
    size: null,
    timer: null,
    fileService: null,

    initialize: function( fileService, path ) {
        this.path = path;
        this.fileService = fileService;

        var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        file.initWithPath(path);
        this.exists = file.exists();
        this.modificationTime = (this.exists ? file.lastModifiedTime : null);
        this.size = (this.exists ? file.fileSize : null);

        var oThis = this;
        this.timer = setTimeout( function() {
            oThis.check();
        }, 500 );
    },

    check: function() {
        var file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsILocalFile);
        file.initWithPath(this.path);

        var exists = file.exists();
        var modificationTime = (exists ? file.lastModifiedTime : null);
        var size = (exists ? file.fileSize : null);

        if( exists != this.exists || modificationTime != this.modificationTime || size != this.size ) {
            this.fileService.fireEvent( 'result', [ 'fileChanged', this.path, exists, Math.round(modificationTime/1000), size ] );
        }

        this.exists = exists;
        this.modificationTime = modificationTime;
        this.size = size;

        var oThis = this;
        this.timer = setTimeout( function() {
            oThis.check();
        }, 500 );
    },

    cancel: function() {
        clearTimeout( this.timer );
    }
} );


