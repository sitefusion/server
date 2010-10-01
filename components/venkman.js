function toOpenWindowByType(inType, uri) {
  var winopts = "chrome,extrachrome,menubar,resizable,scrollbars,status,toolbar";
  window.open(uri, "_blank", winopts);
}

SiteFusion.Classes.Venkman = {
	Venkman: function() {
		var windowDS = Components.classes["@mozilla.org/rdf/datasource;1?name=window-mediator"].getService(Components.interfaces.nsIWindowDataSource);
		var tmpNameSpace = {};                         
		var sl = Components.classes["@mozilla.org/moz/jssubscript-loader;1"].createInstance(Components.interfaces.mozIJSSubScriptLoader);
		sl.loadSubScript("chrome://venkman/content/venkman-overlay.js", tmpNameSpace);
		tmpNameSpace.start_venkman();
	}
}