/*
 uade_adapter.js: Adapts UADE backend to generic WebAudio/ScriptProcessor player.

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

function _splitPath(url)
{
	let arr = url.split("/");

	let path = "";
	for (let i = 0; i < arr.length-1; i++) {
		if (i > 0)
		{
			path += "/";
		}
		path += arr[i];
	}
	let filename = arr[arr.length-1];
	return [path, filename];
}


// treat WantedTeam.bin as local resource
class WantedTeamPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		let patched = fullFilename.indexOf("WantedTeam.bin") > -1;
		if (patched)
		{
			this._wantedOrig = fullFilename;											// e.g. /songs/JO/WantedTeam.bin
			fullFilename =  this._fileMapper.basePathUADE + "/players/WantedTeam.bin"; // always use local file since the file often seems to be missing
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let patched = pathFilenameArray[1].indexOf("WantedTeam.bin") > -1;
		if (patched)
		{
			if (typeof this._wantedOrig != 'undefined')
			{
				pathFilenameArray = _splitPath(this._wantedOrig);	// make sure it "saves" under the original name that the backend expects
			}
		}
		return [patched, pathFilenameArray];
	}
}


// load amiga libs always from the "system" folder
class AmigaLibsPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
		this._libMap = {};
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		let patched = fullFilename.endsWith(".library");
		if (patched)
		{
			let p = _splitPath(fullFilename);

			this._libMap[p[1]] = fullFilename;
			fullFilename = this._fileMapper.basePathUADE + "/system/" + p[1];
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let patched = pathFilenameArray[1].endsWith(".library");
		if (patched)
		{
			if (typeof this._libMap != 'undefined')
			{
				// fixme: seems the path info isn't always properly spilt when it gets here
				let tmp = pathFilenameArray[0] + "/" + pathFilenameArray[1];
				let tp = _splitPath(tmp);

				let op = this._libMap[tp[1]];
				if (typeof op != 'undefined')
				{
					pathFilenameArray = _splitPath(op);
				}
			}
		}
		return [patched, pathFilenameArray];
	}
}


// From the WantedTeam's Quartet_ST player perspective there are 2 used
// prefixes: "QTS." and "SMP.". It makes no sense to use this Amiga specific naming
// for what are originally Atari ST files (Even modland has meanwhile replaced the
// Amiga-naming versions with properly named files.). webUADE therefore uses the
// original names with extensions ".4v" and ".set" and maps the files such that
// the eagleplayer sees the dumb Amiga names while the physical files are
// named properly. It also handles the ".4q" archives (which contain both the
// .4v and the .set data) by extracting those files on the fly (see
// converted_quartet_4q() in eagleplayer.c). (webUADE doesn't care whether or not
// the Amiga-naming might still work when used directly as input files. If you
// still have respective files, do rename them!)
// note: the default name for sample sets is: SMP.set (on Atari and on Amiga!)

class QartettST_Patcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		let patched = (fullFilename.indexOf("/SMP.") > -1 );
		if (patched)
		{
			if ((fullFilename.indexOf("/SMP.set") < 0))
			{
				fullFilename = fullFilename.replace("/SMP.", "/") + ".set";
			}
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let input = pathFilenameArray[1];

		let patched = input.endsWith(".set");
		if (patched)
		{
			if (!input.endsWith("smp.set") && !input.endsWith("SMP.set"))
			{
				pathFilenameArray[1] = "SMP." + input.replace(".set", "");
			}
		}
		return [patched, pathFilenameArray];
	}
}


// On Modland the extension .sng is used for both RichardJoseph and Zoundmonitor while eagleplayer expects .zm
// for Zoundmonitor - also the Zoundmonitor "Samples" are stored  in a separate top level folder in Modland
// and the lib files all use incorrect lowercase names

class ZoundPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
		this._nameMap = {};
	}

	mapToVirtualFilename(filename)
	{
		let patched = false;
		if (filename.endsWith(".sng"))
		{
			filename = filename.replace(".sng", ".zm");
			patched = true;
		}
		return [patched, filename];
	}

	backend2web(fullFilename)
	{
		let patched = false;
		if (fullFilename.indexOf("/Samples/") > -1) 	// libs on modland are all lowercase
		{
			let n = fullFilename.substr(0, fullFilename.lastIndexOf("/")) + fullFilename.substr(fullFilename.lastIndexOf("/")).toLowerCase();

			// modland idiots changed the layout and now put the sample files
			// into a separate "top" level folder.."
			n = n.replace("/Samples/", "/../Samples/");

			this._nameMap[n] = fullFilename;
			fullFilename = n;
			patched = true;
		}
		return [patched, fullFilename];
	}

	web2backend(pathFilenameArray)
	{
		let n = pathFilenameArray[0] + "/" + pathFilenameArray[1];
		let orig = this._nameMap[n];
		let patched = typeof orig != 'undefined';
		if (patched)
		{
			// reverse mapping of "Samples" path
			pathFilenameArray = _splitPath(orig);
		}
		return [patched, pathFilenameArray];
	}
}


class AbstractPatcher {
	constructor(fileMapper)
	{
		this._fileMapper = fileMapper;
		this._nameMap = {};

		// set these in subclasses
		this._mapToVirtualCfg;
		this._backend2webCfg;
	}

	mapToVirtualFilename(filename)
	{
		return this._patch(this._mapToVirtualCfg, filename, false)
	}

	backend2web(fullFilename)
	{
		return this._patch(this._backend2webCfg, fullFilename, true)
	}

	web2backend(pathFilenameArray)
	{
		let n = pathFilenameArray[0] + "/" + pathFilenameArray[1];
		let orig = this._nameMap[n];
		let patched = typeof orig != 'undefined';

		if (patched)
		{
			pathFilenameArray = _splitPath(orig);
		}

		return [false, pathFilenameArray];
	}

	_patch(cfg, filename, setMap)
	{
		let patched = false;

		cfg.every((c) => {
			let match = c[0];
			let replacement = c[1];

			if (filename.indexOf(match) > -1)
			{
				let n = filename.replace(match, replacement);
				if (setMap)
				{
					this._nameMap[n] = filename;
				}
				filename = n;
				patched = true;
			}
			return !patched;
		});

		return [patched, filename];
	}
}

// hack: there only seem to be these 2 songs; MusicMaker songs need a prefix that allows
// UADE to match the correct player
class OldMM8_Patcher extends AbstractPatcher {
	constructor(fileMapper)
	{
		super(fileMapper);

		this._mapToVirtualCfg = [	["/best of guitars.", "/mm8.best of guitars."],
									["/moveback.", "/mm4.moveback."]	];
		this._backend2webCfg = [	["mm8.", ""],
									["mm4.", ""]	];
	}
}

// .mcmd detection conflicts with certain Hippel songs.. fake postfix to force correct detection
class MCMD_Patcher extends AbstractPatcher {
	constructor(fileMapper) {
		super(fileMapper);

		this._mapToVirtualCfg = [	[".mcmd", ".mcmd_org"]	];
		this._backend2webCfg = [	[".mcmd_org", ".mcmd"]	];
	}
}

// mdst. prefix needed for proper ST player selection (modland's naming of the
// "smpl." file also is total garbage.

// note: the player actually uses /players/ENV/EaglePlayer/EP-TFMX_Pro.cfg
class TFMX_ST_Patcher extends AbstractPatcher {
	constructor(fileMapper) {
		super(fileMapper);

		this._mapToVirtualCfg = [	["/mdat.", "/mdst."]	];
		this._backend2webCfg = [	["/mdst.", "/mdat."],
									["smpl.mdst.", "smpl."]	];
	}
}

// patch various names used on Modland: either the modland names are
// garbage or UADE's naming conventions are..
class ModlandPatcher {
	constructor(fileMapper, filename)
	{
		this._fileMapper = fileMapper;

		this._originalFile = decodeURI(filename);
		this._modlandMap = {};
	}

	mapToVirtualFilename(filename) { return [false, filename]; }

	backend2web(fullFilename)
	{
		fullFilename = decodeURI(fullFilename);	// replace escape sequences...

		// modland uses wrong (lower) case for practically all sample file names.. damn jerks
		let output = fullFilename.replace(".adsc.AS", ".adsc.as");	// AudioSculpture

		output = output.replace("/SMP.", "/smp.");	// Dirk Bialluch, Dynamic Synthesizer, Jason Page, Magnetic Fields Packer,
													// Quartet ST(not used here since handled previously), Synth Dream, Thomas Hermann
		output = output.replace(".SSD", ".ssd");	// Paul Robotham
		output = output.replace(".INS", ".ins");	// Richard Joseph
		output = output.replace("/mcS.", "/mcs.");	// Mark Cooksey Old


		let o = this._originalFile.substr(this._originalFile.lastIndexOf("/") + 1);

		let ot = output.substr(output.lastIndexOf("/")+1);

		if (this._originalFile.endsWith(".spm") && (output.indexOf("/SPS.") > 0)) 	// Stonetracker
		{
			output = output.replace("SPS.", "").replace(".spm", ".sps");
		}
		else if (this._originalFile.endsWith(".soc") && output.endsWith(".so")) 	// Hippel ST COSO
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (this._originalFile.endsWith(".pap") && output.endsWith(".pa"))  // Pierre Adane Packer
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (this._originalFile.endsWith(".osp") && output.endsWith(".os"))  // Synth Pack
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (o.startsWith("sdr.") && ot.startsWith("smp."))  // Synth Dream  (always use the "set".. other songs don't seem to play properly anyway - even in uade123)
		{
			output = output.substr(0, output.lastIndexOf("/")) + "/smp.set";
		}
		else if (this._originalFile.endsWith(".ymst") && output.endsWith("replay")) // YMST
		{
			let idx = output.lastIndexOf("/");
			let fn = output.substr(idx).toLowerCase().replace(" ", "_");	// see inconsistent zout-game.ymst
			output = output.substr(0, idx) + fn;
		}

		// map:  actual file name -> "wrong" name used on emu side
		// e.g.  "smp.set"  ->      "dyter07 ongame01.os"
		if (fullFilename != output)	// remember the filename mapping (path is the same)
			this._modlandMap[output.replace(/^.*[\\\/]/, '')]= fullFilename.replace(/^.*[\\\/]/, '');	// needed to create FS expected by "amiga"

		return [true, output];
	}

	web2backend(pathFilenameArray)
	{
		pathFilenameArray = this._fileMapper.normalizePathFilename(pathFilenameArray);

		let patched = typeof this._modlandMap[pathFilenameArray[1]] != 'undefined';
		if (patched)
		{
			pathFilenameArray[1] = this._modlandMap[pathFilenameArray[1]];	// reverse map
		}
		return [patched, pathFilenameArray];
	}
}


/**
* This mapper has three main purposes: 1) handle the various shared resource files
* that need to be located independently of the location of the music files.
* 2) handle the various "sample lib" files that are named totally inconsistently
* on modland - so that the player can still play respective songs directly from
* that collection. 3) make sure the song files use the "virtual" file extensions that
* UADE's eagleplayer detection expects.
*/
class UadeFileMapper extends SimpleFileMapper {
	constructor(module, basePathUADE, modlandMode)
	{
		super(module);

		this.basePathUADE = basePathUADE;

		this._modlandMode = (typeof modlandMode != 'undefined') ? modlandMode : 0;
		this._patchers = [];
	}

	init(filename)
	{
		// the folder names matched below to identify song formats are Modland specific,
		// but by using those names UADE can be given an extra hint whenever necessary..

		this._patchers = [];

		this._patchers.push(new WantedTeamPatcher(this));
		this._patchers.push(new AmigaLibsPatcher(this));

		if (filename.endsWith(".4v")) this._patchers.push(new QartettST_Patcher(this));
		if (filename.indexOf("/Zoundmonitor/") > -1) this._patchers.push(new ZoundPatcher(this));

		if ((filename.indexOf("/best of guitars.") > -1) || (filename.indexOf("/moveback.") > -1)) this._patchers.push(new OldMM8_Patcher(this));
		if (filename.indexOf("/MCMD/") > -1) this._patchers.push(new MCMD_Patcher(this));
		if (filename.indexOf("/TFMX ST/") > -1) this._patchers.push(new TFMX_ST_Patcher(this));

		if (this._modlandMode) this._patchers.push(new ModlandPatcher(this, filename));
	}

	mapToVirtualFilename(filename)
	{
		filename = super.mapToVirtualFilename(filename);
		filename = decodeURI(filename);	// e.g. see TFMX songs Huelsbeck with spaces XXX.. move into baseclass?

		let done = false;
		this._patchers.every((p) => {
			[done, filename] = p.mapToVirtualFilename(filename);
			return !done;
		});
		return filename;
	}

	registerFileData(pathFilenameArray, data)
	{
		// NOTE: UADE uses the "C fopen" based impl to access all files, i.e. the files
		// MUST BE properly provided within the FS (but the local cache within the player
		// is also used when pre-play song conversion is used)

		let done = false;

		this._patchers.every((p) => {
			[done, pathFilenameArray] = p.web2backend(pathFilenameArray);
			return !done;
		});

		return this._registerEmscriptenFileData(this.normalizePathFilename(pathFilenameArray), data);
	}

	mapBackendFilename(name)
	{
		// the input "name" is what UADE later will use for fopen (i.e. the above registerFileData()
		// must ensure that the FS contains the file under that name

		let done = false;

		this._patchers.every((p) => {
			[done, name] = p.backend2web(name);
			return !done;
		});

		return name;
	}
};

class UADEBackendAdapter extends EmsHEAP16BackendAdapter {
	constructor(basePath, modlandMode, disableModConverter, onUpdateSongInfo)
	{
		super(backend_UADE.Module, 2, new UadeFileMapper(backend_UADE.Module, basePath, modlandMode),
						new HEAP16ScopeProvider(backend_UADE.Module, 0x8000));

		if (typeof onUpdateSongInfo == 'undefined') onUpdateSongInfo = function() {};
		this._onUpdateSongInfo = onUpdateSongInfo;

		this.disableModConverter = (typeof disableModConverter != 'undefined') ? disableModConverter : 0;

		this._basePathUADE = basePath;
		this._sendUpdateNotifications = true;

		this.ensureReadyNotification();
	}

	disableModConverter(disable)
	{
		// disabling "automatic mod file comversion" reverts to UADE's original mod file handling
		this.disableModConverter = (typeof disable != 'undefined') ? disable : 0;
	}

	loadMusicData(sampleRate, path, filename, data, options)
	{
		this._sendUpdateNotifications = false;

		this.Module.ccall('emu_prepare', 'number', ['string'], [this._basePathUADE]);	// init virtual FS

		let ret = this.Module.ccall('emu_load_file', 'number',
							['number', 'string', 'string','number'],
							[sampleRate, path, filename, this.disableModConverter]);

		if (ret == 0)
		{
			// UADE's maximum sample rate is SOUNDTICKS_NTSC 3579545 which
			// should never be a relevant limitation here..
			let inputSampleRate = sampleRate;
			this.resetSampleRate(sampleRate, inputSampleRate);
		}
		return ret;
	}

	setPanning(value)
	{
		if (!this.isAdapterReady()) return;

		let pan = Math.min(Math.max(-1.0, value), 1.0);
		this.Module.ccall('emu_set_panning', 'number', ['number'], [pan]);
	}

	evalTrackOptions(options)
	{
		super.evalTrackOptions(options);

		if ((typeof options.pan != 'undefined'))
		{
			this.setPanning(options.pan);
		}

		let track = (typeof options.track === 'undefined') ? -1 : options.track;
		let ret = this.Module.ccall('emu_set_subsong', 'number', ['number'], [track]);

		this._sendUpdateNotifications = true; // any updates that occur later require a notification

		return ret;
	}

	_setDefaultSongAttributes(target)
	{

		let keys = ["info1", "info2", "info3", "mins", "maxs", "curs", "infoText", "format", "player"];
		keys.forEach((key) => {
			target[key] = "";
		});
	}

	getSongInfoMeta()
	{
		return {
			info1: String,
			info2: String,
			info3: String,
			mins: String,
			maxs: String,
			curs: String,
			infoText: String,
			format: String,
			player: String
		};
	}

	// by default called after load of song - normally backends then have the
	// meta info (but not UADE - where some meta data may be asynchronously filled in later)

	updateSongInfo(filename)
	{
		if (Object.keys(this._songInfo).length == 0)
			this._setDefaultSongAttributes(this._songInfo);
	}


	// disable unsupported default impls
	getMaxPlaybackPosition()
	{
		return 0;
	}
	getPlaybackPosition()
	{
		return 0;
	}
	seekPlaybackPosition(ms)
	{
	}

// --------------------------- async file loading stuff -------------------------

	handleBackendSongAttributes(backendAttr)	// called via _songUpdateCallback from callback.js
	{
		// UADE may trigger this from "song load" before updateSongInfo is called (the respective
		// "player" and "format" info is triggered before the remaining song info)

		if (Object.keys(this._songInfo).length == 0)
			this._setDefaultSongAttributes(this._songInfo);

		Object.assign(this._songInfo, backendAttr);


		// unfortunately UADE also triggers some of these notifications late during "computeAudioSamples"
		// (testcase: Confuzion (Hubbard cover).4v; which sets "maxs, mins, curs" from there)

		if (this._sendUpdateNotifications) this._onUpdateSongInfo();	// notify UI of changes
	}
};