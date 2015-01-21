SiteFusion.Classes.FlashProxy = function() {
    SiteFusion.Classes.Browser.call(this);

    this.sfClassName = 'XULFlashProxy';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.FlashProxy.prototype = Object.create(SiteFusion.Classes.Browser.prototype);
SiteFusion.Classes.FlashProxy.prototype.constructor = SiteFusion.Classes.FlashProxy;

    SiteFusion.Classes.FlashProxy.prototype.initialize = function( win ) {
        this.element = win.createElement( 'browser' );
        this.element.sfNode = this;
        this.element.setAttribute('disablehistory', true);
        this.element.setAttribute('type', 'chrome');
        this.element.setAttribute('src',"about:blank");
        var oThis = this;
        
        this.hostWindow = win;
        
        this.setEventHost([
            'initialized',
            'ready',
            'flashReady'
        ]);
        
        this.eventHost.initialized.msgType = 0;
        this.eventHost.ready.msgType = 0;
        this.eventHost.flashReady.msgType = 0;
                
        setTimeout(function() { oThis.fireEvent('ready'); }, 100);
    };
    
    SiteFusion.Classes.FlashProxy.prototype.prepareLoadEvent = function() {
        
    };