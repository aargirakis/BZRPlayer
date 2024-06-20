// create separate namespace for all the Emscripten stuff.. otherwise naming clashes may occur especially when 
// optimizing using closure compiler..
window.spp_backend_state_UADE= {
	notReady: true,
	adapterCallback: function(){}	// overwritten later	
};
window.spp_backend_state_UADE["onRuntimeInitialized"] = function() {	// emscripten callback needed in case async init is used (e.g. for WASM)
	this.notReady= false;
	this.adapterCallback();
}.bind(window.spp_backend_state_UADE);

// HACK
window.getStringifiedLines= function(bytes) {
	var ret= [];
	
	var line = "";
	for (var i= 0; i<bytes.length; i++) {
		var c= bytes[i];
		if ((c == 13) || (c == 10)) { // CR or LF
			if (line.length) {	
				ret.push(line);	
				line= "";
			} else {
				// just ignore empty lines
			}
		} else {
			// this seems to be all that is needed.. see /Delitracker Custom/Chris Huelsbeck/ (copyright sign, etc)
			line = line + String.fromCharCode(c);
		}
	}
	return ret;
}

window.rawBytes= function(module, ptr) {
	var ret= [];	
	while (1) {
		var ch = module.getValue(ptr++, 'i8', true);
		if (!ch) return ret;

		ret.push(ch & 0xff);
	}
}

var backend_UADE = (function(Module) {