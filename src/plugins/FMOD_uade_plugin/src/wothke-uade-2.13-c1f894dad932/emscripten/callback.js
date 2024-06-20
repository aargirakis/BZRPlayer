// additional emscripten *.js library must be in this dumbshit format..

// reminder: used EMSCRIPTEN compiler mode does not allow use of ES6 features here, also
// the stupid optimizer will garble the function names unless they are
// specified using string literals..
mergeInto(LibraryManager.library, {
	
// -- utilities used below (seems I finally figured out how this garbage works: The 
//    "__deps" construct has to be used to declare which function is using the 
//    utility and the actual function call has to be prefixed with "_".)
//    what a bloody dumbfuckery!
	
	rawBytes: function(module, ptr) {
		var ret = [];	
		while (1) {
			var ch = module["getValue"](ptr++, 'i8', true);
			if (!ch) return ret;

			ret["push"](ch & 0xff);
		}
	},
	
	getStringifiedLines: function(bytes) {
		var ret = [];
		
		var line = "";
		for (var i = 0; i < bytes["length"]; i++) {
			var c = bytes[i];
			if ((c == 13) || (c == 10))  // CR or LF
			{
				if (line["length"]) 
				{	
					ret["push"](line);	
					line = "";
				} 
				else 
				{
					// just ignore empty lines
				}
			} 
			else 
			{
				// this seems to be all that is needed.. see /Delitracker Custom/Chris Huelsbeck/ (copyright sign, etc)
				line = line + window["String"]["fromCharCode"](c);
			}
		}
		return ret;
	},

// -- APIs used from C++ code:
	
	// returns 0 means file is ready; -1 if file is not yet available
	ems_request_file: function(name) {
		var p = window["ScriptNodePlayer"]["getInstance"]();
		if (!p["isReady"]()) {
			window["console"]["log"]("error: ems_request_file not ready");
		}
		else 
		{
			return p["_fileRequestCallback"](name);
		}
	},
	
	ems_request_file_size: function(name) {
		var p = window["ScriptNodePlayer"]["getInstance"]();
		return p["_fileSizeRequestCallback"](name);
	},
	ems_cache_file: function(filename, buf, len) {
		var a = HEAPU8.subarray(buf, buf+len);
		var p = window["ScriptNodePlayer"]["getInstance"]();
		p['setFileData'](Pointer_stringify(filename), a);	// data must be Uint8Array
	},
	
	ems_notify_song_update__deps: [ 'rawBytes', 'getStringifiedLines' ],
	ems_notify_song_update: function(inf, min, max, curr) {
		var minText= Pointer_stringify(min);	
		var maxText= Pointer_stringify(max);	
		var currText= Pointer_stringify(curr);	
		
		// no idea how the utility logic can be inlined here or in pre.js without having the fucking
		// closure compiler messing everything up.. FUCKING POS

		var rawBytes = _rawBytes(Module, inf);	// Pointer_stringify would mess up non-ASCII data
		var arr = _getStringifiedLines(rawBytes);
		
		var infoText="";
		for (var j= 0; j<arr.length; j++) {
			infoText+= arr[j]+"<br>";
		}
		
		// title, prefix, modulename, authorname, specialinfo, version, credits, remarks
		// (the below logic works very badly for CUSTOM files..)
		// ---------------------- extract tagged infos ---------------------------------------
		var j, dic= new Object();
		var key= null;
		var section= null;
		for (j= 0; j<arr.length; j++) {
			var line= arr[j];
			// one liner's
				// .cus file special handling 
			if (startsWith(line, "Music:")) {
				var a= dic['authorname']? dic['authorname']: []
				a.unshift(line.substring(6).trim());				
				dic['authorname']= a;
			} else if (startsWith(line, "DeliCustom:")) {		// .cus hack
				var a= dic['specialinfo']? dic['specialinfo']: []
				a.unshift(line.substring(11).trim());				
				dic['specialinfo']= a;
			}  
				
				// "regular" file
			else if (startsWith(line, "File name:")) {
				var a= dic['title']? dic['title']: []
				a.unshift(line.substring(10).trim());				
				dic['title']= a;
			} else if (startsWith(line, "Song title:")) {
				var a= dic['title']? dic['title']: []
				a.unshift(line.substring(11).trim());				
				dic['title']= a;
			} else if (startsWith(line, "File prefix:")) {
				dic['prefix']= line.substring(12).trim();
			} else {
				// multi line
				// note for most modules: "MODULENAME" should be used as a replacement for the "title"
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
		
		// try to construct a 3 line info with what we've got
		
		// there is always a title and prefix
		if (dic['title'] && !('modulename' in dic))
			infoLines.push(dic['title'].shift() +" ("+ dic['prefix'] +")");
		
		while ((infoLines.length <= limit)) {
			if ('modulename' in dic) {
				infoLines.push(dic['modulename'].shift());
				delete dic['modulename'];
			} else if('authorname' in dic) {
				infoLines.push(dic['authorname'].shift());
				delete dic['authorname'];
			} else if('specialinfo' in dic) {
				infoLines.push(dic['specialinfo'].shift());
				delete dic['specialinfo'];
			} else if('version' in dic) {
				infoLines.push(dic['version'].shift());
				if (dic['version'].length == 0) {
					delete dic['version'];
				}
			} else if('credits' in dic) {
				infoLines.push(dic['credits'].shift());
				if (dic['credits'].length == 0) {
					delete dic['credits'];
				}
			} else if('remarks' in dic) {
				infoLines.push(dic['remarks'].shift());
				if (dic['remarks'].length == 0) {
					delete dic['remarks'];
				}
			} else {
				// nothing "known" left
				break;
			}
		}
		
		var info1= infoLines.length>0 ? infoLines[0]: "";
		var info2= infoLines.length>1 ? infoLines[1]: "";
		var info3= infoLines.length>2 ? infoLines[2]: "";
		
		// cannot use an object here because the optimizer will
		// rename the fields..
		var ret= new Object();	
		ret["info1"]= info1;
		ret["info2"]= info2;
		ret["info3"]= info3;
		ret["mins"]= minText;
		ret["maxs"]= maxText;
		ret["curs"]= currText;
		ret["infoText"]= infoText;
		
		var p = window["ScriptNodePlayer"]["getInstance"]();
		return p["_songUpdateCallback"](ret);
	},
	
	ems_notify_format_update: function(format) {
		var ret= new Object();	
		ret["format"]= Pointer_stringify(format).replace("type: ", "");
		
		var p = window["ScriptNodePlayer"]["getInstance"]();
		return p["_songUpdateCallback"](ret);
	},	
	
	ems_notify_player_update: function(player) {
		var ret= new Object();	
		ret["player"]= Pointer_stringify(player);
		
		var p = window["ScriptNodePlayer"]["getInstance"]();
		return p["_songUpdateCallback"](ret);
	},	
});