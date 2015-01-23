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