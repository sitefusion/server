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


SiteFusion.Classes.Grippy = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULGrippy',
	
	initialize: function( win ) {
		this.element = win.createElement( 'grippy' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.ArrowScrollBox = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULArrowScrollBox',
	
	initialize: function( win ) {
		this.element = win.createElement( 'arrowscrollbox' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );


SiteFusion.Classes.Dropmarker = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULDropmarker',
	
	initialize: function( win ) {
		this.element = win.createElement( 'dropmarker' );
		this.element.sfNode = this;
		
		this.setEventHost();
	}
} );

SiteFusion.Classes.Sound = Class.create( SiteFusion.Classes.Node, {
	sfClassName: 'XULSound',
	
	initialize: function( win ) {
		this.element = win.createElement( 'label' );
		this.element.sfNode = this;
		this.ioService = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
		this.sound = Components.classes["@mozilla.org/sound;1"].createInstance(Components.interfaces.nsISound);
		this.setEventHost();
	},
		
	load: function() {
		var d = new Date();
		var strUrl = SiteFusion.Address + '/filestream.php?app=' + SiteFusion.Application + '&args=' + SiteFusion.Arguments + '&sid=' + SiteFusion.SID + '&ident=' + SiteFusion.Ident + '&cid=' + this.cid + '&cycle=' + d.getTime();
		this.url = this.ioService.newURI(strUrl, null, null);
		this.sound.init();
	},
	
	beep: function() {
		
	},
	
	play: function() {
		if (this.url)
			this.sound.play(this.url);
	},
	
	playSystemSound: function(soundAlias) {
		this.sound.playSystemSound(soundAlias);
	}
} );


