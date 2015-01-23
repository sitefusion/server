SiteFusion.Classes.Grippy = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULGrippy';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Grippy.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Grippy.prototype.constructor = SiteFusion.Classes.Grippy;

    SiteFusion.Classes.Grippy.prototype.initialize = function( win ) {
        this.element = win.createElement( 'grippy' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ArrowScrollBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULArrowScrollBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ArrowScrollBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ArrowScrollBox.prototype.constructor = SiteFusion.Classes.ArrowScrollBox;
    
    SiteFusion.Classes.ArrowScrollBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'arrowscrollbox' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.Dropmarker = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULDropmarker';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Dropmarker.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Dropmarker.prototype.constructor = SiteFusion.Classes.Dropmarker;
    
    SiteFusion.Classes.Dropmarker.prototype.initialize = function( win ) {
        this.element = win.createElement( 'dropmarker' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };

SiteFusion.Classes.Sound = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULSound';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Sound.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Sound.prototype.constructor = SiteFusion.Classes.Sound;
    
    SiteFusion.Classes.Sound.prototype.initialize = function( win ) {
        this.element = win.createElement( 'label' );
        this.element.sfNode = this;
        this.ioService = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
        this.sound = Components.classes["@mozilla.org/sound;1"].createInstance(Components.interfaces.nsISound);
        this.setEventHost();
    };
        
    SiteFusion.Classes.Sound.prototype.load = function() {
        var d = new Date();
        var strUrl = SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime();
        this.url = this.ioService.newURI(strUrl, null, null);
        this.sound.init();
    };
    
    SiteFusion.Classes.Sound.prototype.beep = function() {
        
    };
    
    SiteFusion.Classes.Sound.prototype.play = function() {
        if (this.url) {
            this.sound.play(this.url);
        }
    };
    
    SiteFusion.Classes.Sound.prototype.playSystemSound = function(soundAlias) {
        this.sound.playSystemSound(soundAlias);
    };