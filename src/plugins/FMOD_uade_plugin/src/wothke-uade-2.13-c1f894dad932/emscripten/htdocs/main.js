
let songs = [
	"music/stone/modland.spm",

	"YMST/Jochen Hippel/zout-game.ymst",
	"YMST/- unknown/backlash.ymst",
	"Synth Dream/Laurens Tummers/sdr.monsterbusiness 1",
	"TFMX ST/Chris Huelsbeck/mdat.jim power level 1",
	"TFMX ST/Chris Huelsbeck/mdat.quick&silva",

	"MCMD/Zymox/ep-bbsintro91.mcmd",
	"MusicMaker V8 Old/- unknown/moveback.sdata",
	"MusicMaker V8 Old/- unknown/best of guitars.sdata",

	"Zoundmonitor/The Balance/gravediving.sng",

	"music/JO/jo.hiscore",
	"music/stone/modland.spm",
	"music/Quartet ST/absentia.4q",
	"music/Quartet ST/Confuzion (Hubbard cover).4v",


/*
	"music/Skizzo_Demo_2-02.GPMO",
	"music/Skizzo_Demo-03.Skizzo",
	"music/mod.Damaged.pp",
	"music/1h later.p22a",
	"music/blue vibrations.hcd",
	"music/Crystal Hammer.mod",
	"music/DC&Complex-FinlandiaA-1.WantonPacker",
	"music/di.partyland",
	"music/ECHOMIX.PMD3",
	"music/Heartbreak.nw1",
	"music/L.C.S.wnp",
	"music/NetzoneInvit.PerfSong",
	"music/OutlawDead.TMK",
	"music/Sanity-Arte-elekfunk.Noiserunner",
	"music/STA-Athmosphere.StoneArtsPlayer",
	"music/TECH-DigitalIntoxica-2.PM10b",
	"music/titletheme.fuchs",
	"music/Tok In Space.nw1",
	"music/Ultimate Seduction 3.ppk",
	"music/Vivid Final.ben",
	"music/Welcome To The Future.bnr",
	"music/Why Not.eureka",
	"music/the little things.prt",
	"music/best of guitars.sdata;0;145",
	*/
	"music/dont cry.ftm;0;145",
	"music/Geo.mus",
	"music/CUST.Archon",
	"music/MXP.Discovery walk1",
	"music/DUX.Carver End",
	"music/secret silver blades.mxtx",
	"music/BloodMoney.mm4",
	"music/TMK.NoOutlawAnymore",
	"music/MOSH.DistyTwister",
	"music/BYE.b17title",
	"music/jo.title",
	"music/ASH.FantasticDizzy",
	"music/JS.mandie",
	"music/KIM.Rotor_title",
	"music/NPP.wing.c.arctheme",
	"music/TITS.house style",
	"music/amad.ghostbusters.ay",
	"music/mega mix 1.strc",
	"music/cust.Beginner;2;30",
	"music/1 c ya @ the killing ground.spm",
	"music/Darkness-Style.perfsng2",
	"music/Confuzion (Hubbard cover).4v",
	];


class UADEOscilloscopesWidget extends OscilloscopesWidget{
	constructor(divId, tracer, backend)
	{
		super(divId, tracer, backend);		
	}

	_generateHTML(display, n)
	{
		let labels = ["<b>MOS 8364 channels</b>", "", "", ""];

		let t =
		"<div class=\"oscRow\">" +
			"<div class=\"osc1\">";

		for (let i = 0; i < n; i++) {
			t += "<div class=\"panHead\">" + labels[i] + "</div>"+
			  "<div class=\"voiceContainer\"><canvas class=\"voiceCanvas\" id=\"" + this._genCanvId(i) + "\" ></canvas></div>";
		}
		t += "</div>";

		let div = document.getElementById(this._divId);
		div.innerHTML = t;
	}
}

class UADEPlaylistPlayerWidget extends PlaylistPlayerWidget {
	constructor(containerId, songs, onNewTrackCallback, enableSeek, enableSpeedTweak, doParseUrl, doOnDropFile, currentTrack, defaultOptions)
	{
		super(containerId, songs, onNewTrackCallback, enableSeek, enableSpeedTweak, doParseUrl, doOnDropFile, currentTrack, defaultOptions);
	}
	
	_makeLabel(path)
	{
		// e.g.	'/LOCAL/music/Echofied_6581.sid;0;120' or just the filename (for drag&drop)
		let name;

		let i = path.lastIndexOf('/');
		if (i == -1)
		{
			name = path;
		}
		else
		{
			name = path.slice(i+1);
		}
		name = name.split(";")[0];
		return name;
	}
	
	_addSong(filename)
	{
		if ((filename.indexOf(".set") == -1)&& (filename.indexOf("SMP.") == -1))
		{
			super._addSong(filename);
		}
	}
	
};

class Main {
	constructor()
	{
		this._backend;
		this._playerWidget;
		this._scopesWidget;
	}

	_doOnTrackEnd()
	{
		this._playerWidget.playNextSong();
	}

	_setupSliderHoverState()
	{
		// hack: add "highlight while dragging" feature for all sliders on the page
		// (this should rather be setup locally upon creation of the sliders..)

		let slider = $(".slider a.ui-slider-handle");
		slider.hover(function() {
				$(this).prev().toggleClass('hilite');
			});
		slider.mousedown(function() {
				$(this).prev().addClass('dragging');
				$("*").mouseup(function() {
					$(slider).prev().removeClass('dragging');
				});
			});
	}

	_draw()
	{
		gl.uniform2f(this._program.uCanvasSize, c.width, c.height);

		let time= performance.now() / 1000;	// secs
		time = time % 900;					// reset every 15 minutes

		gl.uniform1f(this._program.iTime, time);
		gl.uniform1i(this._program.tex, 0);

		gl.drawArrays(gl.TRIANGLE_STRIP, 0, this._vertexPosBuffer.numItems);

		requestAnimationFrame(this._draw.bind(this))
	}

	run()
	{
		if (useWEBGL())
		{
			this._vertexPosBuffer = screenQuad();

			this._program = setupProgram(window.shaders.vertex, window.shaders.fragment);
			gl.useProgram(this._program);

			this._program.vertexPosAttrib = gl.getAttribLocation(this._program, 'aVertexPosition');
			this._program.uCanvasSize = gl.getUniformLocation(this._program, 'uCanvasSize');
			this._program.iTime = gl.getUniformLocation(this._program, 'iTime');
			this._program.tex = gl.getUniformLocation(this._program, "tex");

			gl.enableVertexAttribArray(this._program.vertexPosAttrib);
			gl.vertexAttribPointer(this._program.vertexPosAttrib, this._vertexPosBuffer.itemSize, gl.FLOAT, false, 0, 0);


			let p1 = loadTexture("warp2.webp", gl.CLAMP_TO_EDGE);
			Promise.all([p1]).then((result) => {
				activateTexture2d(0, result[0]);
				this._draw();
			})
			.catch(err => {
				console.log(err);
			});
		}


		this._tracer = new ChannelStreamer();

		let basePathUADE = 'uade';	// UADE backend uses this when looking for files
		let preload = [	// shorten the trial&error phase by preloading essential files
					basePathUADE + "/uaerc",
					basePathUADE + "/eagleplayer.conf",
					basePathUADE + "/system/score",
				];

		// note: with WASM this may not be immediately ready
		this._backend = new UADEBackendAdapter(basePathUADE, true, 0, function() {
//			console.log(">>> new Meta: " + JSON.stringify(ScriptNodePlayer.getInstance().getSongInfo()))
		});
		this._backend.setProcessorBufSize(2048);

		ScriptNodePlayer.initialize(this._backend, this._doOnTrackEnd.bind(this), preload, false, this._tracer)
		.then((msg) => {

			// constructor requires funtional backend!
			this._scopesWidget= new UADEOscilloscopesWidget("scopesContainer", this._tracer, this._backend);

			let defaultSongIdx = -1;
			this._playerWidget = new UADEPlaylistPlayerWidget("playerContainer", songs, this._scopesWidget.onSongChanged.bind(this._scopesWidget), false, true,
						function(someSong) {
							let options = {};

							let arr = someSong.split(";");
							options.track = arr.length > 1 ? parseInt(arr[1]) : 0;
							options.timeout = arr.length > 2 ? parseInt(arr[2]) : -1;


							someSong = arr[0];
							let isLocal = someSong.startsWith("/tmp/") || someSong.startsWith("music/");
							someSong = isLocal ? someSong : window.location.protocol + "//ftp.modland.com/pub/modules/" + someSong;
									
							return [someSong, options];
						},
						0, defaultSongIdx,
						{}
					);

			this._setupSliderHoverState();

			this._playerWidget.playNextSong();	// player was ready before it could trigger the playback
		});
	}
}
