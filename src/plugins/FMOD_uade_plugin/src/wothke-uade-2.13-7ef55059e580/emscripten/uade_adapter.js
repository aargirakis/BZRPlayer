/*
 uade_adapter.js: Adapts UADE backend to generic WebAudio/ScriptProcessor player.
 
 Known limitation: seeking is not supported by UADE
 
 version 1.01 with added support for "outside files"
 
 	Copyright (C) 2018 Juergen Wothke

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
UADEBackendAdapter = (function(){ var $this = function (basePath, modlandMode) { 
	$this.base.call(this, backend_UADE.Module, 2);
	
		// aka dumshit ftp.modland.com mode:
		this.modlandMode= (typeof modlandMode != 'undefined') ? modlandMode : 0;
		this.originalFile= "";
		this.modlandMap= {};	// mapping of weird shit filenames used on modland 

		this.basePath= basePath;
		this.isReady= false;
		
		if (!backend_UADE.Module.notReady) {
			// in sync scenario the "onRuntimeInitialized" has already fired before execution gets here,
			// i.e. it has to be called explicitly here (in async scenario "onRuntimeInitialized" will trigger
			// the call directly)
			this.doOnAdapterReady();
		}		
	}; 
	// UADE's sample buffer contains 2-byte integer sample data (i.e. must be rescaled) 
	// of 2 interleaved channels
	extend(EmsHEAP16BackendAdapter, $this, {
		doOnAdapterReady: function() {
			// called when runtime is ready (e.g. asynchronously when WASM is loaded)
			this.Module.ccall('emu_prepare', 'number', ['string'], [this.basePath]);	// init virtual FS
			this.isReady = true;
		},
		isAdapterReady: function() { 
			return this.isReady;
		},		
		
		getAudioBuffer: function() {
			var ptr=  this.Module.ccall('emu_get_audio_buffer', 'number');			
			// make it a this.Module.HEAP16 pointer
			return ptr >> 1;	// 2 x 16 bit samples			
		},
		getAudioBufferLength: function() {
			var len= this.Module.ccall('emu_get_audio_buffer_length', 'number') >>2;
			return len;
		},
		computeAudioSamples: function() {
			var status= this.Module.ccall('emu_compute_audio_samples', 'number');
			
			if (status == -1)  {
				return status;	// waiting for some file
			} else if (status > 0) {
				// song is done or error (file can not be loaded with no hope of recovery)

				var isExit= this.Module.ccall('emu_is_exit', 'number');
				if ( isExit) {
					return 2;		// error
				} else {
					return 1;		// end
				}
			}
			return 0;	
		},
		mapUrl: function(filename) {			
			// used transform the "internal filename" to a valid URL
			var uri= this.mapFs2Uri(filename);
			var p= uri.indexOf("@");	// cut off "basePath" for "outside" files
			if (p >= 0) {
				uri= uri.substring(p+1);
			}
			
			if (this.modlandMode) {
				if (uri.indexOf("/TFMX ST/")>0) {
					// restore original/incorect name for retrieval
					uri= uri.replace("/mdst.", "/mdat.");
				}
				if (uri.indexOf("/Zoundmonitor/")>0) {
					// restore original/incorect name for retrieval
					uri= uri.replace(".zm", ".sng");
					
					// modland idiots changed the layout and now put the sample files
					// into a separate "top" level folder.."
					uri= uri.replace("/Samples/", "/../Samples/");
				}
			}
			return uri;
		},
		/*
		* Creates the URL used to retrieve the song file.
		*/
		mapInternalFilename: function(overridePath, basePath, filename) {
			// the problem is that in UADE there is only one "basePath" and this specifies 
			// where to look for *any* files, i.e. uade prefixes this path to whatever
			// files it is tying to load (config/music - doesn't matter), i.e. a correct 
			// outside URL CANNOT be passed through UADE without being messed up in the process
			
			// solution: use a special marker for "outside" URLs and later just substitute 
			// whatever garbage path information UADE is adding (see mapUrl() above)
			
			// map URL to some valid FS path (must not contain "//", ":" or "?")
			// input e.g. "@mod_proxy.php?mod=Fasttracker/4-Mat/bonus.mod" or
			// "@ftp://foo.com/foo/bar/file.mod" (should avoid name clashes)
			
			filename= this.mapUri2Fs("@" + filename);	// treat all songs as "from outside"

			var f= ((overridePath)?overridePath:basePath) + filename;	// this._basePath ever needed?

			if (this.modlandMode) {
				f= decodeURI(f);	// e.g. see TFMX songs Huelsbeck with spaces				
				this.originalFile= f;
				
				// these files need the correct prefix for UADE (or any other player) to handle them
				if (f.indexOf("/TFMX ST/")>0) {
					f= f.replace("/mdat.", "/mdst.");
				}
				if (f.indexOf("/Zoundmonitor/")>0) {
					f= f.replace(".sng", ".zm");
					
					// modland idiots changed the layout and now put the sample files
					// into a separate "top" level folder..".. 
					f= f.replace("/../Samples/", "/Samples/");
				}			
			}
			return f;
		},
		getPathAndFilename: function(fullFilename) {
			// input is path+filename combined: base for "registerFileData" & "loadMusicData"
			
			if (fullFilename.substring(0, this.basePath.length) == this.basePath) {
				fullFilename= fullFilename.substring(this.basePath.length);
			}
			// since uade needs the basePath to *ALWAYS* point to the folder where the various config 
			// files can be found, the filename returned here is actually still a path 
			return [this.basePath, fullFilename];
		},
		mapCacheFileName: function (name) {
			// problem: in some folders, modland uses one "shared lib" file (e.g. "smp.set") that is 
			// accessed via different "alias names". when the next alias name is used the file 
			// will already be found in the cache => but for the song to work the respective file data
			// must be registered in the FS under the additional alias (was solved by adding "alias" 
			// support in the scriptnode player)
			
			return name;
		},
		mapModlandShit: function (input) {
			var input= decodeURI(input);	// replace escape sequences... 
			
			// modland uses wrong (lower) case for practically all sample file names.. damn jerks
			var output= input.replace(".adsc.AS", ".adsc.as");	// AudioSculpture
			output= output.replace("/SMP.", "/smp.");	// Dirk Bialluch, Dynamic Synthesizer, Jason Page, Magnetic Fields Packer, Quartet ST, Synth Dream, Thomas Hermann 
			output= output.replace(".SSD", ".ssd");		// Paul Robotham 
			output= output.replace(".INS", ".ins");	// Richard Joseph  
			output= output.replace("/mcS.", "/mcs.");	// Mark Cooksey Old  
			
			var o= this.originalFile.substr(this.originalFile.lastIndexOf("/")+1);
			var ot= output.substr(output.lastIndexOf("/")+1);

			if (this.originalFile.endsWith(".soc") && output.endsWith(".so")) {	// Hippel ST COSO 
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (this.originalFile.endsWith(".pap") && output.endsWith(".pa")) { // Pierre Adane Packer 
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (this.originalFile.endsWith(".osp") && output.endsWith(".os")) { // Synth Pack  
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (o.startsWith("sdr.") && ot.startsWith("smp.")) { // Synth Dream  (always use the "set".. other songs don't seem to play properly anyway - even in uade123)
				output= output.substr(0, output.lastIndexOf("/")) + "/smp.set";
			} else if (this.originalFile.endsWith(".ymst") && output.endsWith("replay")) { // YMST 
				var idx= output.lastIndexOf("/");
				var fn= output.substr(idx).toLowerCase().replace(" ", "_");	// see inconsistent zout-game.ymst 
				output= output.substr(0, idx) + fn;
			} else if (ot.startsWith("smpl.mdst.")) { // TFMX_ST 
				output= output.replace("smpl.mdst.", "smpl.");
			} else if (output.startsWith("mdat.") && (output.indexOf("/TFMX ST/")>0)) {
				// these ST files need the correct prefix for UADE (or any other player) to handle them
				output= output.replace("/mdat.", "/mdst.");				
			} else if (output.endsWith(".sng") && (output.indexOf("/Zoundmonitor/")>0)) {
				// needed ext for UADE to handle them
				output= output.replace(".sng", ".zm");
			}  else if (this.originalFile.endsWith(".sng") && (output.indexOf("/Samples/")>0)) {
				// libs on modland are all lowercase				
				output= output.substr(0, output.lastIndexOf("/")) + output.substr(output.lastIndexOf("/")).toLowerCase();
			}
			// map:  actual file name -> "wrong" name used on emu side
			// e.g.  "smp.set"  ->      "dyter07 ongame01.os"
			if (input != output)	// remember the filename mapping (path is the same)
				this.modlandMap[output.replace(/^.*[\\\/]/, '')]= input.replace(/^.*[\\\/]/, '');	// needed to create FS expected by "amiga"
			
			return output;
		},		
		mapBackendFilename: function (name) {
			// triggered by UADE whenever it tries to load a file, the input "name" is
			// composed of UADE's basePath combined with the file that uade is looking for:
			// the "name" is what UADE later WILL USE for fopen (which is NOT affected by any mappings 
			// that might be done here.. i.e. in order to function the file must be installed in the FS
			// under exactly that path/name!)
			var input= this.Module.Pointer_stringify(name);
			
			if (input.indexOf("WantedTeam.bin") >= 0) {
				this.wantedOrig = input;							// e.g. uade/@songs/JO/WantedTeam.bin
				input= this.basePath + "players/WantedTeam.bin";	// always use local file since the file often seems to be missing
				
			} else if (input.indexOf("stonepacker.library") >= 0) {
				this.stoned = input;							// e.g. uade/@songs/JO/WantedTeam.bin
				input= this.basePath + "players/stonepacker.library";	// always use local file since the file often seems to be missing
				
				
			} else if (this.modlandMode) input= this.mapModlandShit(input);

			return input;
		},
		splitPath: function(url) {
			var arr= url.split("/");

			var path= "";
			for(var i= 0; i<arr.length-1; i++) {
				if (i>0)
					path += "/";
				path += arr[i];
			}
			var filename= arr[arr.length-1];
			return [path, filename];
		},
		registerFileData: function(pathFilenameArray, data) {
			// NOTE: UADE uses the "C fopen" based impl to access all files, i.e. the files 
			// MUST BE properly provided within the FS (the local cache within the player is nothing more than a 
			// convenience shortcut to detect if data is  already available - but irrelevant to UADE)
			
			if (pathFilenameArray[1].indexOf("WantedTeam.bin") >= 0) {	// make sure it "saves" under the original name that the backend expects
				if (typeof this.wantedOrig != 'undefined') pathFilenameArray = this.splitPath(this.wantedOrig);			
			}			
			if (pathFilenameArray[1].indexOf("stonepacker.library") >= 0) {	// make sure it "saves" under the original name that the backend expects
				if (typeof this.stoned != 'undefined') pathFilenameArray = this.splitPath(this.stoned);		// XXX		
			}			
			// input: the path is fixed to the basePath & the filename is actually still a path+filename
			var path= pathFilenameArray[0];
			var filename= pathFilenameArray[1];
			
			// MANDATORTY to move any path info still present in the "filename" to "path"
			var tmpPathFilenameArray = new Array(2);	// do not touch original IO param			
			var p= filename.lastIndexOf("/");
			if (p > 0) {
				tmpPathFilenameArray[0]= path + filename.substring(0, p);
				tmpPathFilenameArray[1]= filename.substring(p+1);
			} else  {
				tmpPathFilenameArray[0]= path;
				tmpPathFilenameArray[1]= filename;
			}

			if (this.modlandMode) {
				if (typeof this.modlandMap[tmpPathFilenameArray[1]] != 'undefined') {
					tmpPathFilenameArray[1]= this.modlandMap[tmpPathFilenameArray[1]];	// reverse map
					
					// e.g. saves the file under the name "dyter07 ongame01.os" (but
					// with the "URL-PATH". e.g. proxy.phpÿmod=Synth Pack/Karsten Obarski/Dyter-07)
				}
			}
			// setup data in our virtual FS (the next access should then be OK)
			return this.registerEmscriptenFileData(tmpPathFilenameArray, data);
		},
		loadMusicData: function(sampleRate, path, filename, data, options) {
			if (path.substr(-1) === "/") path= path.substring(0, path.length-1);
			
			var ret = this.Module.ccall('emu_init', 'number', 
								['number', 'string', 'string'], 
								[sampleRate, path, filename]);
				
			if (ret == 0) {
				// UADE's maximum sample rate is SOUNDTICKS_NTSC 3579545 which 
				// should never be a relevant limitation here..
				var inputSampleRate = sampleRate;
				this.resetSampleRate(sampleRate, inputSampleRate); 
			}
			return ret;
		},
		setPanning: function(value) {
			var pan= Math.min(Math.max(-1.0, value), 1.0);
			this.Module.ccall('emu_set_panning', 'number', ['number'], [pan]);
		},
		evalTrackOptions: function(options) {
			if ((typeof options.timeout != 'undefined') && (options.timeout >0)) {
				ScriptNodePlayer.getInstance().setPlaybackTimeout(options.timeout*1000);
			}			
			if ((typeof options.pan != 'undefined')) {
				this.setPanning(options.pan);
			}
			if ((typeof options.track === 'undefined')) {
				options.track= -1;	// use lowest or default (if available in format)
			}
			return this.Module.ccall('emu_set_subsong', 'number', ['number'], [options.track]);
		},
		teardown: function() {
//			this.Module.ccall('emu_teardown', 'number'); for some reason wasn't used in the old version
		},		
		
		initSongAttributes: function() {
			this.songInfo= new Object();
			this.songInfo.info1= "";
			this.songInfo.info2= "";
			this.songInfo.info3= "";;
			this.songInfo.mins= "";
			this.songInfo.maxs= "";
			this.songInfo.curs= "";			
			this.songInfo.infoText= "";			
		},
		getSongInfoMeta: function() {
			return {info1: String,
					info2: String,
					info3: String,
					mins: String,
					maxs: String,
					curs: String,
					infoText: String 
					};
		},
		updateSongInfo: function(filename, result) {
			if (typeof this.songInfo == 'undefined') this.initSongAttributes()
				
			result.info1= this.songInfo.info1;
			result.info2= this.songInfo.info2;
			result.info3= this.songInfo.info3;;			
			result.mins= this.songInfo.mins;
			result.maxs= this.songInfo.maxs;
			result.curs= this.songInfo.curs;
			result.infoText= this.songInfo.infoText;
		},
		getNumberTraceStreams: function() {
			return this.Module.ccall('emu_get_number_trace_streams', 'number');			
		},
		getTraceStreams: function() {
			var result= [];
			var n= this.getNumberTraceStreams();

			var ret = this.Module.ccall('emu_get_trace_streams', 'number');			
			var array = this.Module.HEAP32.subarray(ret>>2, (ret>>2)+n);
			
			for (var i= 0; i<n; i++) {
				result.push(array[i] >> 1);	// pointer to int16 array
			}
			return result;
		},
		enableScope: function(on) {
			// hack: always on
		},
		readFloatTrace: function(buffer, idx) {
			return (this.Module.HEAP16[buffer+idx])/0x8000;
		},
		
		// --------------------------- async file loading stuff -------------------------
		handleBackendSongAttributes: function(backendAttr, target) {
			this.initSongAttributes();
			// UADE "asynchronously" pushes a respective update..
			this.songInfo.info1= backendAttr.info1;
			this.songInfo.info2= backendAttr.info2;
			this.songInfo.info3= backendAttr.info3;;		
			this.songInfo.mins= backendAttr.minText;
			this.songInfo.maxs= backendAttr.maxText;
			this.songInfo.curs= backendAttr.currText;
			this.songInfo.infoText= backendAttr.infoText;
			
			this.updateSongInfo("", target);		
		},
		
	});	return $this; })();
