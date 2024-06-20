// additional emscripten *.js library must be in this dumbshit format..

mergeInto(LibraryManager.library, {
	// returns 0 means file is ready; -1 if file is not yet available
	uade_request_file: function(name) {
		return window['fileRequestCallback'](name);
	},	
	uade_request_file_size: function(name) {
		return window['fileSizeRequestCallback'](name);
	},	
	uade_notify_song_update: function(i, min, max, curr) {
		var infoText= Pointer_stringify(i);	
		var minText= Pointer_stringify(min);	
		var maxText= Pointer_stringify(max);	
		var currText= Pointer_stringify(curr);	

		var arr= infoText.split("\n");// title, prefix, modulename, authorname, specialinfo, version, credits, remarks
		// ---------------------- extract tagged infos ---------------------------------------
		var i, dic= new Object();
		var key= null;
		var section= null;
		for (i= 0; i<arr.length; i++) {
			var line= arr[i];
			// one liner's
			if (startsWith(line, "File name:")) {
				var a= dic['title']? dic['title']: []
				a.push(line.split("/").pop().trim());				
				dic['title']= a;
			} else if (startsWith(line, "Song title:")) {
				var a= dic['title']? dic['title']: []
				a.unshift(line.substring(11).trim());				
				dic['title']= a;
			} else if (startsWith(line, "File prefix:")) {
				dic['prefix']= line.substring(12).trim();
			} else {
				// multi line
				var k= startsWith2(line, ["MODULENAME:", "AUTHORNAME:", "SPECIALINFO:", "VERSION:", "CREDITS:", "Remarks:"]);
				if (k) {
					// new section
					if (key && section) {
						dic[key]= section;
						section = null;
					}
					key= k.substring(0, k.length-1).toLowerCase();
				} else {
					// only consider content of recognized sections
					line= line.trim();
					if (line.length && key) {
						if (!section) section= [];
						section.push(line);
					}
				}
			}
		}
		if (key && section) {
			dic[key]= section;
		}
				
		// ---------------------- try to make sense of structured info -----------------------
		var limit= 3
		var infoLines= [];
		var keysLeft= Object.keys(dic).length - 2;	// there is always a title and prefix - which are handled later
		
		// try to construct a 3 line info with what we've got
		while ((infoLines.length<=limit) && (keysLeft >0)) {
			if ('modulename' in dic) {
				infoLines.push(dic['modulename'].shift());
				delete dic['modulename']; keysLeft--;
			} else if('authorname' in dic) {
				infoLines.push(dic['authorname'].shift());
				delete dic['authorname']; keysLeft--;
			} else if('specialinfo' in dic) {
				infoLines.push(dic['specialinfo'].shift());
				delete dic['specialinfo']; keysLeft--;
			} else if('version' in dic) {
				infoLines.push(dic['version'].shift());
				if (dic['version'].length == 0) {
					delete dic['version']; keysLeft--; 
				}
			} else if('credits' in dic) {
				infoLines.push(dic['credits'].shift());
				if (dic['credits'].length == 0) {
					delete dic['credits']; keysLeft--; 
				}
			} else if('remarks' in dic) {
				infoLines.push(dic['remarks'].shift());
				if (dic['remarks'].length == 0) {
					delete dic['remarks']; keysLeft--;
				}
			} 
		}
		if (infoLines.length < limit) {	// last resort
			infoLines.push(dic['title'].shift() +" ("+ dic['prefix'] +")");
		}
		
		var info1= infoLines.length>0 ? infoLines[0]: "";
		var info2= infoLines.length>1 ? infoLines[1]: "";
		var info3= infoLines.length>2 ? infoLines[2]: "";
		
		return window['songUpdateCallback'](info1, info2, info3, minText, maxText, currText);
	},
});