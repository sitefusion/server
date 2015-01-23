SiteFusion.Classes.KeySet = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULKeySet';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.KeySet.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.KeySet.prototype.constructor = SiteFusion.Classes.KeySet;

    SiteFusion.Classes.KeySet.prototype.initialize = function( win ) {
        this.element = win.createElement( 'keyset' );
        this.element.sfNode = this;

        this.setEventHost();
    };


SiteFusion.Classes.Key = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULKey';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Key.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Key.prototype.constructor = SiteFusion.Classes.Key;

    SiteFusion.Classes.Key.prototype.initialize = function( win ) {
        this.hostWindow = win;
        this.element = win.createElement( 'key' );
        this.element.sfNode = this;

        this.setEventHost();

        this.element.setAttribute( 'oncommand', 'return true;' );
    };


SiteFusion.Classes.Trigger = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'Trigger';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Trigger.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Trigger.prototype.constructor = SiteFusion.Classes.Trigger;

    SiteFusion.Classes.Trigger.prototype.initialize = function( iter, delay ) {
        this.iterations = iter;
        this.delay = delay;

        this.element = document.createElement( 'box' );

        var oThis = this;

        if ( iter == 0 ) {
            this.timer = setInterval( function() {
                oThis.fireEvent( 'yield' );
            }, delay );
        } else {
            this.timer = setInterval( function() {
                oThis.fireEvent( 'yield' );
                if( --oThis.iterations == 0 ) clearInterval(oThis.timer);
            }, delay );
        }

        this.setEventHost( [ 'yield' ] );
    };

    SiteFusion.Classes.Trigger.prototype.cancel = function() {
        clearInterval( this.timer );
    };


SiteFusion.Classes.PrefService = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'PrefService';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.PrefService.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.PrefService.prototype.constructor = SiteFusion.Classes.PrefService;

    SiteFusion.Classes.PrefService.prototype.initialize = function( win ) {
        this.hostWindow = win;
        this.element = win.createElement( 'box' );
        this.element.hidden = true;

        this.prefService = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);

        this.setEventHost( ['yield']);
        this.eventHost.yield.msgType = 1;
        this.setEventHost();
    };

    SiteFusion.Classes.PrefService.prototype.yield = function(prefTypes) {
        var ret = {};
        try {
            for (var pref in this.prefTypes) {
                switch(this.prefTypes[pref]) {
                    case "int":
                        var retval = this.getIntPref(pref);
                        if (typeof retval === "number") {
                            ret[pref] = retval;
                        }
                    break;
                    case "string":
                        var retval = this.getCharPref(pref);
                        if (typeof retval === "string") {
                            ret[pref] = retval;
                        }
                    break;
                    case "bool":
                        var retval = this.getBoolPref(pref);
                        if (retval != -1) {
                            ret[pref] = retval;
                        }
                    break;
                }
            }
        } catch (e) {
            alert('error: ' + e);
        }
        this.fireEvent( 'yield', [ ret ] );
    };

    SiteFusion.Classes.PrefService.prototype.setIntPref = function(pref, value) {
        this.prefService.setIntPref(pref, value);
    };

    SiteFusion.Classes.PrefService.prototype.getIntPref = function(pref) {
        if ( this.prefService.getPrefType(pref) == this.prefService.PREF_INT ) {
            return this.prefService.getIntPref(pref);
        } else {
            return false;
        }
    };

    SiteFusion.Classes.PrefService.prototype.setCharPref = function(pref, value) {
        this.prefService.setCharPref(pref, value);
    };

    SiteFusion.Classes.PrefService.prototype.getCharPref = function(pref) {
        if( this.prefService.getPrefType(pref) == this.prefService.PREF_STRING ) {
            return this.prefService.getCharPref(pref);
        } else {
            return false;
        }
    };
    
    SiteFusion.Classes.PrefService.prototype.setBoolPref = function(pref, value) {
        this.prefService.setBoolPref(pref, value);
    };

    SiteFusion.Classes.PrefService.prototype.getBoolPref = function(pref) {
        if( this.prefService.getPrefType(pref) == this.prefService.PREF_BOOL ) {
            return this.prefService.getBoolPref(pref);
        } else {
            return -1;
        }
    };


SiteFusion.Classes.JScriptService = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'JScriptService';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.JScriptService.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.JScriptService.prototype.constructor = SiteFusion.Classes.JScriptService;

    SiteFusion.Classes.JScriptService.prototype.initialize = function( win ) {
        Components.utils.import("resource://gre/modules/NetUtil.jsm");

        this.hostWindow = win;
        this.element = win.createElement( 'box' );
        this.element.hidden = true;

        this.setEventHost( ['yield']);
        this.eventHost.yield.msgType = 1;
        this.setEventHost();

        this.converter = Components.classes["@mozilla.org/intl/scriptableunicodeconverter"].createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
        this.converter.charset = "UTF-8";
    };

    SiteFusion.Classes.JScriptService.prototype.runJScript = function(inputCode, id, hasReturnValue) {
        var exportCode = 'function exportJSON(input){var objFSO=WScript.CreateObject("Scripting.FileSystemObject");var objFile=objFSO.OpenTextFile("%EXPORTFILE%",2,true);objFile.Write(JSON.stringify(input))}';

        var codeFile = FileUtils.getFile("TmpD", ["SFTempJS_" + id + ".js"]);
        var exportFile = FileUtils.getFile("TmpD", ["SFTempJS_" + id + "_Export.txt"]);

        var exportFilePath = exportFile.path.replace(/\\/g, "\\\\");

        exportCode = exportCode.replace("%EXPORTFILE%", exportFilePath);
        if(hasReturnValue) {
            inputCode += "\n" + exportCode;
        }

        var ostream = FileUtils.openSafeFileOutputStream(codeFile);
        var istream = this.converter.convertToInputStream(inputCode);

        var oThis = this;

        // The last argument (the callback) is optional.
        NetUtil.asyncCopy(istream, ostream, function(status) {
            if (!Components.isSuccessCode(status)) {
                return;
            }

            var process = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
            var wScript = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
            wScript.initWithPath("C:\\Windows\\System32\\wscript.exe");
            process.init(wScript);

            var args = [codeFile.path];
            var observer = {
                observe: function( subject, topic, data ) {
                    if(hasReturnValue) {
                        NetUtil.asyncFetch(exportFile, function(inputStream, status) {
                            if (!Components.isSuccessCode(status)) {
                                oThis.fireEvent('scriptFinished', [id, null]);
                                window.setTimeout(function(){
                                    codeFile.remove(false);
                                }, 1000);
                                
                                if(hasReturnValue) {
                                    window.setTimeout(function(){
                                        exportFile.remove(false);
                                    }, 1000);
                                }
                                return;
                            }

                            var data = NetUtil.readInputStreamToString(inputStream, inputStream.available());
                            var returnValue = data.evalJSON();
                            oThis.fireEvent('scriptFinished', [id, returnValue]);

                            window.setTimeout(function(){
                                codeFile.remove(false);
                                exportFile.remove(false);
                            }, 1000);
                        });
                    } else {
                        oThis.fireEvent('scriptFinished', [id, null]);
                        window.setTimeout(function(){
                            codeFile.remove(false);
                        }, 1000);
                    }
                }
            };

            process.runAsync(args, args.length, observer);
        });
    };


SiteFusion.Classes.TerminalCommandService = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'TerminalCommandService';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TerminalCommandService.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TerminalCommandService.prototype.constructor = SiteFusion.Classes.TerminalCommandService;
    
    SiteFusion.Classes.TerminalCommandService.prototype.initialize = function() {
        Components.utils.import('resource://gre/modules/NetUtil.jsm');
        Components.utils.import('resource://gre/modules/FileUtils.jsm');

        this.setEventHost( ['yield']);
        this.eventHost.yield.msgType = 1;
        this.setEventHost();

        this.converter = Components.classes['@mozilla.org/intl/scriptableunicodeconverter'].createInstance(Components.interfaces.nsIScriptableUnicodeConverter);
        this.converter.charset = 'UTF-8';
    };
    
    SiteFusion.Classes.TerminalCommandService.prototype.execute = function( code, filenames, id ) {
        var scriptFile = FileUtils.getFile('TmpD', ['SFTC_Temp-' + id +  '.command']);

        var ostream = FileUtils.openSafeFileOutputStream(scriptFile);
        var istream = this.converter.convertToInputStream(code);

        var oThis = this;

        NetUtil.asyncCopy(istream, ostream, function (aResult) {
            if (!Components.isSuccessCode(aResult)) {
                oThis.fireEvent('scriptFinished', [id, false, []]);
                return;
            }

            scriptFile.permissions = 0777;
            
            var args = [scriptFile.path];

            var process = Components.classes['@mozilla.org/process/util;1'].createInstance(Components.interfaces.nsIProcess);
            var wScript = Components.classes['@mozilla.org/file/local;1'].createInstance(Components.interfaces.nsILocalFile);
            wScript.initWithPath('/bin/sh');
            process.init(wScript);

            process.runAsync(args, args.length, function (aSubject, aTopic) {

                var outputFiles = [];
                var success = (aTopic == 'process-finished');

                for (var i = filenames.length - 1; i >= 0; i--) {
                    var file = new FileUtils.File(filenames[i]);
                    if (!file.exists()) {
                        SiteFusion.consoleMessage('Output file "' + filenames[i] + '" not found!');
                        outputFiles[i] = false;
                        success = false;
                        continue;
                    }

                    var fstream = Cc['@mozilla.org/network/file-input-stream;1'].createInstance(Ci.nsIFileInputStream);
                    var sstream = Cc['@mozilla.org/scriptableinputstream;1'].createInstance(Ci.nsIScriptableInputStream);
                    fstream.init(file, -1, 0, 0);
                    sstream.init(fstream); 

                    var response = '';
                    var str = sstream.read(4096);
                    while (str.length > 0) {
                        response += str;
                        str = sstream.read(4096);
                    }

                    sstream.close();
                    fstream.close();

                    outputFiles[i] = response;

                    file.remove(false);
                }

                scriptFile.remove(false);
                oThis.fireEvent('scriptFinished', [id, success, outputFiles]);
            });
        });
    };
