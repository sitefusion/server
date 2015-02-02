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
// theFrontDoor.
// Portions created by the Initial Developer are Copyright (C) 2009
// the Initial Developer. All Rights Reserved.
//
// Contributor(s):
//   Nikki Auburger <nikki@thefrontdoor.nl> (original author)
//   Tom Peeters <tom@thefrontdoor.nl>
//   Pieter Janssen <pieter.janssen@thefrontdoor.nl>
//
// - - - - - - - - - - - - - - END LICENSE BLOCK - - - - - - - - - - - - -

SiteFusion.Classes.ToolBarSeparator = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULToolBarSeparator';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ToolBarSeparator.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ToolBarSeparator.prototype.constructor = SiteFusion.Classes.ToolBarSeparator;
    
    SiteFusion.Classes.ToolBarSeparator.prototype.initialize = function( win ) {
        this.element = win.createElement( 'toolbarseparator' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ToolBox = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULToolBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ToolBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ToolBox.prototype.constructor = SiteFusion.Classes.ToolBox;
    
    SiteFusion.Classes.ToolBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'toolbox' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ToolBar = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULToolBar';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ToolBar.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ToolBar.prototype.constructor = SiteFusion.Classes.ToolBar;
    
    SiteFusion.Classes.ToolBar.prototype.initialize = function( win ) {
        this.element = win.createElement( 'toolbar' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.ToolBarButton = function() {
    SiteFusion.Classes.Node.apply(this, arguments);

    this.sfClassName = 'XULToolBarButton';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.ToolBarButton.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.ToolBarButton.prototype.constructor = SiteFusion.Classes.ToolBarButton;
    
    SiteFusion.Classes.ToolBarButton.prototype.initialize = function( win ) {
        this.element = win.createElement( 'toolbarbutton' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };