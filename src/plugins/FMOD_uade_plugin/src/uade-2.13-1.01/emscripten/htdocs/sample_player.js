/**
* sample_player.js
*
*   Version 1.0
* 	Copyright (C) 2014 Juergen Wothke
*
* Generic music player used to feed WebAudio ScriptProcessorNode. Allows to interact with Emscripten generated 
* "backends". In order to be usable by this player a "backend" has to adapt to the interface required by this 
* player, i.e. it must implement/export the following functions:
*		emu_init
*		emu_set_subsong
*		emu_teardown
*		emu_is_exit
*		emu_compute_audio_samples
*		emu_get_audio_buffer
*		emu_get_audio_buffer_length 
*
* This player provides different "*Callback" functions which can be used from the C code / "backend" side 
* for the purpose of asynchronously loading files:
*	window.fileRequestCallback
*	window.fileSizeRequestCallback
*	window.songUpdateCallback
*
* Known limitations: The player does not perform any resampling, i.e. if the sampleRate used by the AudioContext
* is higher than the maximum supported by the backend then the music will play too fast.
*
* Terms of Use: This software is licensed under a CC BY-NC-SA 
* (http://creativecommons.org/licenses/by-nc-sa/4.0/).
*/

var sampleRate;
try {
	window.AudioContext = window.AudioContext||window.webkitAudioContext;
	sampleRate = new AudioContext().sampleRate;
} catch(e) {
	alert('Web Audio API is not supported in this browser (get Chrome 18 or Firefox 26)');
}

// utils
String.prototype.startsWith = function(str) {
	return (this.match("^"+str)==str)
}
String.prototype.startsWith2 = function(arr) {
	var i;
	for(i= 0; i<arr.length; i++) {
		var k= arr[i];
		if (this.match("^"+k)==k)
			return k;
	}
	return null;
}

var fetchSamples= function(e) { 	
	// it seems that it is necessary to keep this explicit reference to the event-handler
	// in order to pervent the dumbshit Chrome GC from detroying it eventually
	
	var f= window.player['generateSamples'].bind(window.player); // need to re-bind the instance.. after all this 
																	// joke language has no real concept of OO	
	f(e);
}

SamplePlayer = function(basePath, sampleRate, onUpdate, onEnd, onError) {
	this.sampleRate= sampleRate;

	this.info1= "";
	this.info2= "";
	this.info3= "";
	this.mins;
	this.maxs;
	this.curs;
	
	this.currentModule= 0;
	this.currentSubSong= 0;
	this.currentTimeout= -1;
	this.currentPlaytime= 0;	// measures in number of samples
	
	this.basePath= basePath;

	this.isPaused= false;
	this.isWaitingForFile= false;
	this.initInProgress=false;
	this.initialized= false;
	this.isPaused= false;
	this.isWaitingForFile= false;
	this.initInProgress=false;
	this.initialized= false;

	this.onEnd= onEnd;
	this.onError= onError;
	this.onUpdate= onUpdate;
	
	this.sourceBuffer;
	this.sourceBufferLen;

	this.numberOfSamplesRendered= 0;
	this.numberOfSamplesToRender= 0;
	this.sourceBufferIdx=0;
	
	this.binaryFileMap = {};	// cache for loaded "file" binaries
	this.pendingFileMap = {};	
		
	// setup callback for "replay" retrieval
	window.fileRequestCallback= this.fileRequestCallback.bind(this);
	window.fileSizeRequestCallback= this.fileSizeRequestCallback.bind(this);
	window.songUpdateCallback= this.songUpdateCallback.bind(this);
	
	this.SAMPLES_PER_BUFFER = 8192;	// allowed: buffer sizes: 256, 512, 1024, 2048, 4096, 8192, 16384
	
	window.player= this;
};

SamplePlayer.prototype = {
	createScriptProcessor: function(audioCtx) {
		var prepareFunc =  Module.cwrap('emu_prepare', 'number', ['string']);
		var status = prepareFunc(this.basePath);	

		var scriptNode = audioCtx.createScriptProcessor(this.SAMPLES_PER_BUFFER, 0, 2);
		scriptNode.onaudioprocess = fetchSamples;
	//	scriptNode.onaudioprocess = player.generateSamples.bind(player);	// doesn't work with dumbshit Chrome GC
		return scriptNode;
	},
	getSongInfo: function () {
		return [this.info1, this.info2, this.info3];
	},
	setPauseMode: function (pauseOn) {
		if ((!this.isWaitingForFile) && (!this.initInProgress) && this.initialized)
			this.isPaused= pauseOn;
	},
	playTmpFile: function (file) {
		var filename="songs/"+file.name;	// format detection may depend on prefixes and postfixes..
		var filenameFull= this.basePath+"/"+filename;
		
		var reader = new FileReader();
		reader.onload = function() {
		
			var d= new Uint8Array(reader.result);	
			var f= Module.FS_createDataFile("/", filenameFull, d, true, true);
			this.binaryFileMap[filenameFull]= f;
			this.setupSongData(filename);
			
		}.bind(this);
		reader.readAsArrayBuffer(file);
	},

	songUpdateCallback:function(line1, line2, line3, min, max, curr) {
		// notification that emu has updated infos regarding the currently played song..
		this.info1= line1;
		this.info2= line2;
		this.info3= line3;;
		
		this.mins= min;
		this.maxs= max;
		this.curs= curr;
	
		this.onUpdate();
	},
	// callback invoked by emscripten code 
	fileSizeRequestCallback: function (name) {
		var filename= Module.Pointer_stringify(name);
		var f= this.binaryFileMap[filename];	// this API is only called after the file has actually loaded
		
		// HACK to workaround optimizer 
		if (typeof f.contents == 'undefined') {
			if (typeof f.u == 'undefined') 
				alert("fatal error: optimizer removed variable needed to access Node.contents");
			f.contents= f.u;
		}
		return f.contents.length;
	},
	
	// emu attempts to read some file using this function: if the file is available (i.e. its binary 
	// data has already been loaded the function signals the success by returning 0.. if the file has not 
	// yet been loaded the function returns -1	
	fileRequestCallback: function (name) {
		var filename= Module.Pointer_stringify(name);
		return this.loadFile(filename, function() {
//						console.log("loaded file ["+filename+"]");
						this.initIfNeeded();	// kick off next attempt to initialize
					}.bind(this));	
	},
	loadFile: function (input, onLoadedHandler) {
		var filename= this.stripSongname(input);
		if (filename in this.binaryFileMap)	{
			// the respective file has already been setup
			if (this.binaryFileMap[filename]) {	
			
				return 0;
			}
			return 1;	// error file does not exist
		}

		// Emscripten will be stuck without this file and we better make 
		// sure to not use it before it has been properly reinitialized
		this.isPaused= true;
		this.isWaitingForFile= true;
		this.initialized= false;
		
		
		// requested data not available.. we better load it for next time
		if (!(filename in this.pendingFileMap)) {		// avoid duplicate loading
		
			this.pendingFileMap[filename] = 1;

			var oReq = new XMLHttpRequest();
			oReq.open("GET", filename, true);
			oReq.responseType = "arraybuffer";

			oReq.onload = function (oEvent) {
				var arrayBuffer = oReq.response;
				if (arrayBuffer) {
					console.log("loaded file: "+filename);

				// setup data in our virtual FS (the next access should then be OK)
					var d= new Uint8Array(arrayBuffer);	
					var f= Module.FS_createDataFile("/", filename, d, true, true);
					this.binaryFileMap[filename]= f;
					
					// now that we have an additional file loaded we can retry the initialization
					this.isWaitingForFile= false;

					onLoadedHandler();
				}
				delete this.pendingFileMap[filename]; 
			}.bind(this);
			oReq.onreadystatuschange = function (oEvent) {
			  if (oReq.readyState==4 && oReq.status==404) {
				this.binaryFileMap[filename]= 0;	// file not found
				this.isWaitingForFile= false;
			  }
			}.bind(this);
			oReq.onerror  = function (oEvent) {
				this.binaryFileMap[filename]= 0;	// marker for non existing file
				this.isWaitingForFile= false;
			}.bind(this);

			oReq.send(null);
		}
		return -1;	
	},
	
	string2buf: function( str ) {
		var bytes = new Uint8Array(str.length+1);	// we dont have any unicode here
		var i;
		for (i = 0; i < str.length; i++){ 
			var c= str.charCodeAt(i);
			bytes[i]= c & 0xff;
		}
		bytes[i]= 0;
		return bytes;	
	},
	initIfNeeded: function() {
		// no point in retrying if pending file is not ready yet
		if(!this.initInProgress && !this.initialized && !this.isWaitingForFile) {	
			this.initInProgress= true;
				
			var resetFunc =  Module.cwrap('emu_init', 'number', ['number', 'string', 'string']);
			var status = resetFunc(this.sampleRate, this.basePath, this.currentModule);	
			
			if (status <0) {
				this.initialized= false;
				this.isWaitingForFile= true;
				this.initInProgress= false;
			} else if(status == 0) {
				this.isPaused= false;		
				this.isWaitingForFile= false;		
				this.initialized= true;
				this.currentPlaytime= 0;
				console.log("successfully completed init");				
				this.initInProgress= false;
				
				if(this.currentSubSong >0) {
					// songs without multiple subsongs seem to take this very badly..
					var setSubSongFunc =  Module.cwrap('emu_set_subsong', 'number', ['number']);
					setSubSongFunc(this.currentSubSong);	
				}
			} else {
				this.initInProgress= false;
				// error that cannot be resolved.. (e.g. file not exists)
				if (this.onError) this.onError();
			}
		}
	},
	setupSongData: function(filename) {
		// 'filename' may contain additional track and timeout info
		var arr= filename.split(";");	
		this.currentSubSong= arr.length>1?parseInt(arr[1]):0;
		this.currentTimeout= arr.length>2?parseInt(arr[2]):-1;

		this.initialized= false;
		this.isWaitingForFile= false;
		this.isPaused= true;
		this.currentModule = arr[0];
		
		this.initIfNeeded();	// this first call will typically fail but trigger some file load
		
		this.isPaused= false;
	},
	
	preload: function(files, id) {
		if (id == 0) {
			var file= files[0].substring(this.basePath.length+1);	// remove basepath for regular loading..
			this.setupSongData(file);	// we are done preloading
		} else {
			id--;
			var funcCompleted= function() {this.preload(files, id);}.bind(this); // trigger next load
			return this.loadFile(files[id], funcCompleted);	
		}
	},
	stripSongname: function(input) {
		// input: "foo/bar/fi.le;0;29999"	i.e. filename, subsong id, timeout in sec
		var i= input.indexOf(";");
		if (i == -1)	return input;
		
		return input.substring(0, i);
	},
	preloadSongData: function(files) {
		// avoid the async trial&error loading for those
		// files that we already know we'll be needing
		this.isPaused= true;
		this.preload(files, files.length);
	},
	generateSamples: function(event) {
		var output1 = event.outputBuffer.getChannelData(0);
		var output2 = event.outputBuffer.getChannelData(1);

		if ((!this.initialized) || this.isWaitingForFile || this.isPaused) {
			var i;
			for (i= 0; i<output1.length; i++) {
				output1[i]= 0;
				output2[i]= 0;
			}		
		} else {
			var outSize= output1.length;
			
			this.numberOfSamplesRendered = 0;		

			while (this.numberOfSamplesRendered < outSize)
			{
				if (this.numberOfSamplesToRender == 0) {
					var status = Module.ccall('emu_compute_audio_samples', 'number');
					if (status <0) {					
						// emu just discovered that we need to load another file 
						this.isPaused= true;
						this.initialized= false; 		// previous init is invalid
						this.isWaitingForFile= true;
						this.fillEmpty(outSize, output1, output2);
						return;
					} else if(status >0) {
						// song is done or error (some file can not be loaded with no hope of recovery)												
						this.isPaused= true;
						this.fillEmpty(outSize, output1, output2);	

						var isExit= Module.ccall('emu_is_exit', 'number');
						if ( isExit) {
							// note: this code will also be hit if additional load is triggered from the playback,
							// i.e. we want to exclude that case
							if (this.onEnd && !this.isWaitingForFile) 
								this.onEnd();
						} else {
							if (this.onError) this.onError();	
						}
						return;
					}				
					if ((this.currentTimeout>0) && this.currentPlaytime/sampleRate > this.currentTimeout) {
						this.isPaused= true;
						if (this.onEnd) this.onEnd();
						return;
					}
					
					// get location of sample buffer
					this.sourceBuffer= Module.ccall('emu_get_audio_buffer', 'number');
					this.sourceBufferLen= Module.ccall('emu_get_audio_buffer_length', 'number') >>2;
				
					this.numberOfSamplesToRender = this.sourceBufferLen;
					this.sourceBufferIdx=0;			
				}
				
				var srcBufI16= this.sourceBuffer>>1;	// 2 x 16 bit samples
				if (this.numberOfSamplesRendered + this.numberOfSamplesToRender > outSize) {
					var availableSpace = outSize-this.numberOfSamplesRendered;					
					this.renderSamples(srcBufI16, availableSpace, output1, output2);
					
					this.numberOfSamplesToRender -= availableSpace;
					this.numberOfSamplesRendered = outSize;
				} else {
					this.renderSamples(srcBufI16, this.numberOfSamplesToRender, output1, output2);

					this.numberOfSamplesRendered += this.numberOfSamplesToRender;
					this.numberOfSamplesToRender = 0;
				} 
			}  
		}	
	},
	renderSamples: function(buf16, len, output1, output2) {
		var i;
		for (i= 0; i<len; i++) {
			var r16= Module.HEAP16[buf16 + (this.sourceBufferIdx++)];
			var l16= Module.HEAP16[buf16 + (this.sourceBufferIdx++)];

			output1[i+this.numberOfSamplesRendered]= r16/0x7fff;
			output2[i+this.numberOfSamplesRendered]= l16/0x7fff;
		}	
		this.currentPlaytime+= len;
	},
	fillEmpty: function(outSize, output1, output2) {
		var availableSpace = outSize-this.numberOfSamplesRendered;
		for (i= 0; i<availableSpace; i++) {
			output1[i+this.numberOfSamplesRendered]= 0;
			output2[i+this.numberOfSamplesRendered]= 0;
		}				
		this.numberOfSamplesToRender = 0;
		this.numberOfSamplesRendered = outSize;			
	}
};