SiteFusion.Classes.TabBox = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULTabBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TabBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TabBox.prototype.constructor = SiteFusion.Classes.TabBox;
    
    SiteFusion.Classes.TabBox.prototype.initialize = function( win ) {
        SiteFusion.consoleMessage('tabbox');
        
        this.element = win.createElement( 'tabbox' );
        this.element.sfNode = this;
        
        this.setEventHost(['yield']);
        this.eventHost.yield.msgType = 1;
    };
    
    SiteFusion.Classes.TabBox.prototype.selectedTab = function( tab ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedTab = tab.element; } );
    };
    
    SiteFusion.Classes.TabBox.prototype.selectedPanel = function( panel ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedPanel = panel.element; } );
    };
    
    SiteFusion.Classes.TabBox.prototype.yield = function() {
        var oThis = this;
        this.fireEvent( 'yield', [ oThis.element.selectedIndex, oThis.element.selectedTab.sfNode, oThis.element.selectedPanel.sfNode ] );
    };


SiteFusion.Classes.Tabs = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULTabs';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Tabs.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Tabs.prototype.constructor = SiteFusion.Classes.Tabs;
    
    SiteFusion.Classes.Tabs.prototype.initialize = function( win ) {
        this.element = win.createElement( 'tabs' );
        this.element.sfNode = this;
        
        this.setEventHost(['yield', 'afterAdvanceSelectedTab']);
        this.eventHost.yield.msgType = 1;
        this.eventHost.afterAdvanceSelectedTab.msgType = 1;
    };
    
    SiteFusion.Classes.Tabs.prototype.selectedItem = function( tab ) {
        var oThis = this;
        SiteFusion.Interface.DeferredCallbacks.push( function() { oThis.element.selectedItem = tab.element; } );
    };
    
    SiteFusion.Classes.Tabs.prototype.advanceSelectedTab = function(dir, wrap) {
        this.element.advanceSelectedTab(dir, wrap);
        this.fireEvent('afterAdvanceSelectedTab');
    };
    
    SiteFusion.Classes.Tabs.prototype.yield = function() {
        var oThis = this;
        this.fireEvent( 'yield', [ oThis.element.selectedItem.sfNode, oThis.element.selectedIndex ] );
    };


SiteFusion.Classes.Tab = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULTab';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Tab.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Tab.prototype.constructor = SiteFusion.Classes.Tab;
    
    SiteFusion.Classes.Tab.prototype.initialize = function( win ) {
        this.element = win.createElement( 'tab' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.TabPanels = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULTabPanels';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TabPanels.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TabPanels.prototype.constructor = SiteFusion.Classes.TabPanels;

    SiteFusion.Classes.TabPanels.prototype.initialize = function( win ) {
        this.element = win.createElement( 'tabpanels' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.TabPanel = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULTabPanel';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.TabPanel.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.TabPanel.prototype.constructor = SiteFusion.Classes.TabPanel;

    SiteFusion.Classes.TabPanel.prototype.initialize = function( win ) {
        this.element = win.createElement( 'tabpanel' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.GroupBox = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULGroupBox';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.GroupBox.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.GroupBox.prototype.constructor = SiteFusion.Classes.GroupBox;
    
    SiteFusion.Classes.GroupBox.prototype.initialize = function( win ) {
        this.element = win.createElement( 'groupbox' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.Caption = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULCaption';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Caption.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Caption.prototype.constructor = SiteFusion.Classes.Caption;
    
    SiteFusion.Classes.Caption.prototype.initialize = function( win ) {
        this.element = win.createElement( 'caption' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.Separator = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULSeparator';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Separator.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Separator.prototype.constructor = SiteFusion.Classes.Separator;
    
    SiteFusion.Classes.Separator.prototype.initialize = function( win ) {
        this.element = win.createElement( 'separator' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };


SiteFusion.Classes.Spacer = function() {
    SiteFusion.Classes.Node.call(this);

    this.sfClassName = 'XULSpacer';

    this.initialize.apply(this, arguments);
};
SiteFusion.Classes.Spacer.prototype = Object.create(SiteFusion.Classes.Node.prototype);
SiteFusion.Classes.Spacer.prototype.constructor = SiteFusion.Classes.Spacer;
    
    SiteFusion.Classes.Spacer.prototype.initialize = function( win ) {
        this.element = win.createElement( 'spacer' );
        this.element.sfNode = this;
        
        this.setEventHost();
    };
