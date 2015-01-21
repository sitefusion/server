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
//   Francesco Danti <info@oracoltech.com>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -

consolelog = function(msg) {
  Components.classes['@mozilla.org/consoleservice;1']
                  .getService(Components.interfaces.nsIConsoleService)
                  .logStringMessage(msg);
};

SiteFusion.Classes.Label = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULLabel';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Label.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Label.prototype.constructor = SiteFusion.Classes.Label;

    SiteFusion.Classes.Label.prototype.initialize = function( win, text ) {
        this.element = win.createElement( 'label' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.Description = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULDescription';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Description.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Description.prototype.constructor = SiteFusion.Classes.Description;

    SiteFusion.Classes.Description.prototype.initialize = function( win, text ) {
        this.element = win.createElement( 'description' );
        this.element.sfNode = this;
        this.hostWindow = win;

        this.setEventHost();
    };

    SiteFusion.Classes.Description.prototype.value = function( val ) {
        var text = this.hostWindow.createTextNode( val );
        for( var n = 0; n < this.element.childNodes.length; n++ ) {
            this.element.removeChild( this.element.childNodes[n] );
        }
        
        this.element.appendChild( text );
    };

SiteFusion.Classes.Image = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULImage';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Image.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Image.prototype.constructor = SiteFusion.Classes.Image;

    SiteFusion.Classes.Image.prototype.initialize = function( win, src, width, height ) {
        this.element = win.createElement( 'image' );
        this.element.sfNode = this;
        
        this.setEventHost();
        
        if( src )
            this.src( src );
        if( width )
            this.width( width );
        if( height )
            this.height( height );
    };


SiteFusion.Classes.HTMLVideo = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULHTMLVideo';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.HTMLVideo.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.HTMLVideo.prototype.constructor = SiteFusion.Classes.HTMLVideo;

    SiteFusion.Classes.HTMLVideo.prototype.initialize = function( win, src, width, height ) {
        this.element = win.createElementNS("http://www.w3.org/1999/xhtml", 'html:video');
        this.element.sfNode = this;
        
        this.setEventHost();
        
        if( src )
            this.src( src );
        if( width )
            this.width( width );
        if( height )
            this.height( height );
            
        this.element.setAttribute("controls", "true");
    };


SiteFusion.Classes.HTMLAudio = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULHTMLAudio';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.HTMLAudio.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.HTMLAudio.prototype.constructor = SiteFusion.Classes.HTMLAudio;

    SiteFusion.Classes.HTMLAudio.prototype.initialize = function( win, src, width ) {
        this.element = win.createElementNS("http://www.w3.org/1999/xhtml", 'html:audio');
        this.element.sfNode = this;
        
        this.setEventHost();
        
        if( src )
            this.src( src );
        if( width )
            this.width( width );

        this.element.setAttribute("controls", "true");
    };

    SiteFusion.Classes.HTMLAudio.prototype.width = function(val) {
        this.element.style.width = val;
        this.element.setAttribute( 'width', parseInt(val) );
    };


SiteFusion.Classes.HTMLEmbed = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULHTMLEmbed';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.HTMLEmbed.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.HTMLEmbed.prototype.constructor = SiteFusion.Classes.HTMLEmbed;

    SiteFusion.Classes.HTMLEmbed.prototype.initialize = function( win, src, width, height ) {
        this.element = win.createElementNS("http://www.w3.org/1999/xhtml", 'html:embed');
        this.element.sfNode = this;
        
        this.setEventHost();
        
        if( src )
            this.src( src );
        if( width )
            this.width( width );
        if( height )
            this.height( height );
            
        this.element.setAttribute("controls", "true");
    };


SiteFusion.Classes.DirectImage = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULDirectImage';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.DirectImage.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.DirectImage.prototype.constructor = SiteFusion.Classes.DirectImage;

    SiteFusion.Classes.DirectImage.prototype.initialize = function( win, width, height ) {
        this.element = win.createElement( 'image' );
        this.element.sfNode = this;
        
        this.setEventHost();
        
        if( width )
            this.width( width );
        if( height )
            this.height( height );
    };

    SiteFusion.Classes.DirectImage.prototype.recycle = function() {
        this.element.setAttribute( 'src', this.getSrc() );
    };

    SiteFusion.Classes.DirectImage.prototype.getSrc = function() {
        var d = new Date();
        return SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime();
    };


SiteFusion.Classes.FileSystemImage = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULFileSystemImage';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.FileSystemImage.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.FileSystemImage.prototype.constructor = SiteFusion.Classes.FileSystemImage;

    SiteFusion.Classes.FileSystemImage.prototype.initialize = function( win, width, height ) {
        this.element = win.createElement( 'image' );
        this.element.sfNode = this;
        
        this.setEventHost();
        
        if( width )
            this.width( width );
        if( height )
            this.height( height );
    };

    SiteFusion.Classes.FileSystemImage.prototype.load = function() {
        var d = new Date();
        this.element.setAttribute( 'src', SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime() );
    };


SiteFusion.Classes.ImageSvg = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULImageSvg';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ImageSvg.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ImageSvg.prototype.constructor = SiteFusion.Classes.ImageSvg;

    SiteFusion.Classes.ImageSvg.prototype.initialize = function( win, width, height ) {
        this.element = win.createElement( 'bbox' );
        this.element.sfNode = this;
        this.hostWindow = win;
      
        this.setEventHost();
      
        if( width ) {
            this.width( width );
        }
        if( height ) {
            this.height( height );
        }
    };

    SiteFusion.Classes.ImageSvg.prototype.load = function() {
        var d = new Date();
        var xmlDoc = new XMLHttpRequest();
        xmlDoc.open(
            "GET"
            , SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime()
            , false
        );
        xmlDoc.send(null);
      
        var eXml = xmlDoc.responseXML.getElementsByTagName('svg')[0];
      
        if(eXml.getAttributeNode("width") != null && eXml.getAttributeNode("height") != null) {
         
            var othisWidth = parseInt(this.element.getAttributeNode("width").nodeValue);
            var othisHeight = parseInt(this.element.getAttributeNode("height").nodeValue);

            var svgOriWidth = parseInt(eXml.getAttributeNode("width").nodeValue);
            var svgOriHeight = parseInt(eXml.getAttributeNode("height").nodeValue);

            var scaleFactorWidth =  othisWidth / svgOriWidth;
            var scaleFactorHeight = othisHeight / svgOriHeight;

            eXml.removeAttribute("width");
            eXml.removeAttribute("height");

            eXml.setAttribute("viewBox",'0 0 '+svgOriWidth+' '+svgOriHeight+'');

            var transform = xmlDoc.responseXML.createElement('g');
            transform.setAttribute('transform','scale('+scaleFactorWidth+', '+scaleFactorHeight+')');

            while(eXml.length > 0) {
                transform.appendChild(eXml.childNodes[0]);
                eXml.removeChild(eXml.childNodes[0]);      
            }

            eXml.appendChild(transform);
        }
      
        var node = this.hostWindow.windowObject.document.importNode(eXml, true);
        this.element.appendChild( node );
    };
