SiteFusion.Classes.Browser = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULBrowser';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Browser.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Browser.prototype.constructor = SiteFusion.Classes.Browser;
    
    SiteFusion.Classes.Browser.prototype.initialize = function( win ) {
        this.element = win.createElement( 'browser' );
        this.element.sfNode = this;
        this.element.hostWindow = win;
        this.element.setAttribute('disablehistory', true);
        
        this.setEventHost();
    };
    
    SiteFusion.Classes.Browser.prototype.setSrc = function( src ) {
        this.element.setAttribute( 'src', src );
    };
    
    SiteFusion.Classes.Browser.prototype.reload = function() {
        this.element.reload();
    };


SiteFusion.Classes.PrintBox = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULPrintBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.PrintBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.PrintBox.prototype.constructor = SiteFusion.Classes.PrintBox;

    SiteFusion.Classes.PrintBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'browser' );
        this.element.sfNode = this;
        this.element.setAttribute('disablehistory', true);
        this.element.setAttribute( 'src', 'about:blank' );
        
        this.setEventHost( [ 'ready' ] );
    };
    
    SiteFusion.Classes.PrintBox.prototype.setContent = function( html ) {
        if( ! this.element.contentDocument ) {
            var oThis = this;
            setTimeout( function() { oThis.setContent( html ); }, 10 );
            return;
        }
        
        this.element.contentDocument.open();
        this.element.contentDocument.write( html );
        this.element.contentDocument.close();
    };
    
    SiteFusion.Classes.PrintBox.prototype.print = function( silent ) {
        var wbp = this.element.contentWindow.QueryInterface(Ci.nsIInterfaceRequestor).getInterface(Ci.nsIWebBrowserPrint);
        var oThis = this;
        
        var printProgressListener = {
            done: false,
            onLocationChange: function( prog, req, loc ) {},
            onProgressChange: function( prog, req, selfprog, maxselfprog, totalprog, maxtotalprog ) {
                if( totalprog == maxtotalprog && !this.done ) {
                    setTimeout( function() { oThis.fireEvent( 'ready' ); }, 1000 );
                    this.done = true;
                }
            },
            onSecurityChange: function( prog, req, state ) {},
            onStateChange: function( prog, req, stateFlags, status ) {},
            onStatusChange: function( prog, req, status, message ) {}
        };
        
        var ps = this.getPrintSettings();
        ps.printSilent = silent;
        ps.headerStrRight = '';
        ps.footerStrLeft = '';
        ps.footerStrRight = '';
        
        try {
            wbp.print( ps, printProgressListener );
        }
        catch ( ex ) {}
    };
    
    SiteFusion.Classes.PrintBox.prototype.setPrinterDefaultsForSelectedPrinter = function (aPrintService, aPrintSettings) {
        if (!aPrintSettings.printerName)
            aPrintSettings.printerName = aPrintService.defaultPrinterName;
        
        // First get any defaults from the printer 
        aPrintService.initPrintSettingsFromPrinter(aPrintSettings.printerName, aPrintSettings);
        // now augment them with any values from last time
        aPrintService.initPrintSettingsFromPrefs(aPrintSettings, true, aPrintSettings.kInitSaveAll);
    };
    
    SiteFusion.Classes.PrintBox.prototype.getPrintSettings = function () {
        var pref = Cc["@mozilla.org/preferences-service;1"].getService(Ci.nsIPrefBranch);
        if (pref) {
            var gPrintSettingsAreGlobal = pref.getBoolPref("print.use_global_printsettings", false);
            var gSavePrintSettings = pref.getBoolPref("print.save_print_settings", false);
        }
        
        var printSettings;
        try {
            var printService = Cc["@mozilla.org/gfx/printsettings-service;1"].getService(Ci.nsIPrintSettingsService);
            if (gPrintSettingsAreGlobal && navigator.platform.match( /win32/i ) ) {
                printSettings = printService.globalPrintSettings;
                this.setPrinterDefaultsForSelectedPrinter(printService, printSettings);
            } else {
                printSettings = printService.newPrintSettings;
            }
        } catch (e) {
            SiteFusion.Error("getPrintSettings: "+e+"\n");
        }
        return printSettings;
    };


SiteFusion.Classes.Statusbar = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULStatusbar';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Statusbar.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Statusbar.prototype.constructor = SiteFusion.Classes.Statusbar;

    SiteFusion.Classes.Statusbar.prototype.initialize = function( win ) {
        this.element = win.createElement( 'statusbar' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.StatusbarPanel = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULStatusbarPanel';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.StatusbarPanel.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.StatusbarPanel.prototype.constructor = SiteFusion.Classes.StatusbarPanel;

    SiteFusion.Classes.StatusbarPanel.prototype.initialize = function( win ) {
        this.element = win.createElement( 'statusbarpanel' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.DialogHeader = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULDialogHeader';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.DialogHeader.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.DialogHeader.prototype.constructor = SiteFusion.Classes.DialogHeader;

    SiteFusion.Classes.DialogHeader.prototype.initialize = function( win ) {
        this.element = win.createElement( 'dialogheader' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };