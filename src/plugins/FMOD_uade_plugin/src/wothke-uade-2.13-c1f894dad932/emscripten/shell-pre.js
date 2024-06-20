/*
 backend_uade.js: UADE plugin for use in ScriptNodePlayer.

   version 1.2
   copyright (C) 2018-2023 Juergen Wothke


 Known limitation: seeking is not supported by UADE

 LICENSE

 This library is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2.1 of the License, or (at
 your option) any later version. This library is distributed in the hope
 that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
*/


// create separate namespace for all the Emscripten stuff.. otherwise naming clashes may occur especially when 
// optimizing using closure compiler..
window.spp_backend_state_UADE = {
	locateFile: function(path, scriptDirectory) { return (typeof window.WASM_SEARCH_PATH == 'undefined') ? path : window.WASM_SEARCH_PATH + path; },
	print: function(t) {
		// suppress annoying "source file" info
		setTimeout(console.log.bind(console, t));	// "lose" the original context
	},
	notReady: true,
	adapterCallback: function(){}	// overwritten later	
};
window.spp_backend_state_UADE["onRuntimeInitialized"] = function() {	// emscripten callback needed in case async init is used (e.g. for WASM)
	this.notReady= false;
	this.adapterCallback();
}.bind(window.spp_backend_state_UADE);

var backend_UADE = (function(Module) {