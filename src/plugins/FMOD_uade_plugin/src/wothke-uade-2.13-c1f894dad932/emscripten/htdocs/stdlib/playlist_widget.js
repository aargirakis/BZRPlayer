// version 1.03a music player widget and utilites

// dependencies: channelstreamer.js, scriptnode_player.js

/**
 * A collapsed "one-liner" selection list that expands during selection.
 * from https://codepen.io/plines/pen/LVLgVw
 *
 * The original impl from codepen is total garbage (no meaningful event handlers,
 * not accessors to change the selection programmatically, fragile redundancy with
 * regard to the shadow structures mirroring the original "select" list) and only
 * useful if *exactly one* selection list exists on the whole page!
 *
 * I did NOT fix these flaws but just quick&dirty patched the code so it works
 * for my very limited usecase. (The functionality works and I did not want to
 * spend the time to implement something from scratch.)
 *
 * This impl is still hard-wired to one "select" list which is identified
 * using a CSS style class: "old-select" and a "shadow" <div> identified by the
 * class "new-select".
 */
class PlaylistWidget {
	constructor(onClickCallback)
	{
		this.onClickCallback = onClickCallback;
	}

	// added functions hacked into the original impl

	addEntry(label)
	{
		// added entry (end of list) is immediately selected
		$('.old-select option[selected]').removeAttr('selected');
        $('.old-select').append('<option value="unused' + label + '" selected>' + label + '</option>');

		this.reinit();
	}

	setSelection(idx)
	{
		// issue: this seems to work only up to 49 entries.. when list is larger, the
		// 1st entry seems to stick
		this.openSelect();	// existing state handling expect changes to happen while menu is opened

		// remove old selection
		$('.old-select option[selected]').removeAttr('selected');

		let label;
		let options = $('.old-select option');	// HTMLOptionElement array
		options.each(function(i, val){
			// 'val' here is an unusable empty object.. but $(this) seems to be what we need..
			if (i == idx)
			{
				label = val.innerHTML;

				$(this).attr('selected','');
				return false; // break
			}
		});

		// Selection New Select
        $('.selection p span').html($('.old-select option[selected]').html());
	//	$('.selection').click();

		// close menu (which is open after setting new selection)

		let countOption = $('.old-select option').size();
		let reverseIndex = countOption;
		$('.new-select .new-option').each(function() {
			$(this).css('z-index',reverseIndex);
			reverseIndex = reverseIndex-1;
		});

		this.closeSelect();
	}
	
	_escapeHTML(txt) 
	{
		let div = document.createElement('div');
		div.innerText = txt;
		return div.innerHTML;
	}

	addClickHandler(onClickCallback)
	{
		let clickHandler = function(d) {
				d.stopPropagation();

				if ($('.selection').hasClass('open') !== true) // ignore clicks on collapsed line
				{
					let selected = this._escapeHTML(d.target.innerText);	// & -> &amp; etc
					
					if (typeof selected == 'undefined' || !selected.length) return; // when open/close arrow is clicked

					let idx;

					// find selected in original options list (based on label)
					let options = $('.old-select option');	// HTMLOptionElement array
					let countOption = options.size();
					options.each(function(i, val){
						val = val.innerHTML;						
						if (val == selected)
						{
							idx = i;
							return false; // break
						}
					});
					onClickCallback(idx, selected);
				}
			}.bind(this);

		$(".selection").click(clickHandler);
	}

	// -------------------------- originally existing code -------------
    openSelect()
	{
        let heightSelect = $('.new-select').height();
        let j = 1;
        $('.new-select .new-option').each(function(){
            $(this).addClass('reveal');
            $(this).css({
				/* all expanded items */
              //  'box-shadow':'1px 1px 1px rgba(1,0,0,0.4)',
                'border':'1px solid #999',

                'left':'0',
                'right':'0',
                'top': j * (heightSelect + 1) + 'px'
            });
            j++;
        });
    }

    closeSelect()
	{
        let i = 0;
        $('.new-select .new-option').each(function() {
            $(this).removeClass('reveal');
			let countOption = $('.old-select option').size();

			// hack: just hide behind the header item
            $(this).css('top',0);
			$(this).css('box-shadow','none');

			/*
			// below garbage seems to have been intended to create a
			// visible "stack of cards" (max 3) when collapsed..
			// below is the styling for the expandable entries (not the separate list header)

			if(i<countOption-3){
                $(this).css('top',0);
                $(this).css('box-shadow','none');
            } else if(i===countOption-3){
                $(this).css('top','3px');
            } else if(i===countOption-2){
                $(this).css({
                    'top':'7px',
                    'left':'2px',
                    'right':'2px',
                });
            } else if(i===countOption-1){
                $(this).css({
                    'top':'11px',
                    'left':'4px',
                    'right':'4px'
                });
            }*/

            i++;
        });
    }

    reinit()
	{
		// hack: reset initial HTML (as workaround for garbage impl)
		$('.selection p span').html('');
		$('.new-select').html("<div class=\"selection\"><p><span></span><i></i></p><span></span></div>");	// expected start state

		// Initialisation
		let selections= $('.old-select option[selected]').size();
		if(selections === 1)
		{
			$('.selection p span').html($('.old-select option[selected]').html());
		}
		else
		{
			$('.selection p span').html($('.old-select option:first-child').html());
		}

		$('.old-select option').each(function() {
			let newValue = $(this).val();
			let newHTML = $(this).html();
			$('.new-select').append('<div class="new-option" data-value="' + newValue + '"><p>' + newHTML + '</p></div>');
		});

		let countOption = $('.old-select option').size();
		let reverseIndex = countOption;
		$('.new-select .new-option').each(function() {
			$(this).css('z-index',reverseIndex);
			reverseIndex = reverseIndex-1;
		});

		this.closeSelect();


		// Ouverture / Fermeture
		let that = this;
		$('.selection').click(function(e) {
			e.stopPropagation();

			$(this).toggleClass('open');
			if($(this).hasClass('open') === true)
			{
				that.openSelect();
			}
			else
			{
				that.closeSelect();
			}
		});


		// Selection
		$('.new-option').click(function(e) {
			e.stopPropagation();

			let newValue = $(this).data('value');

			// Selection New Select
			let oldS = $('.selection p span').html();
			let newS = $(this).find('p').html();

			$('.selection p span').html(newS);

			if (oldS != newS)
			{
				$('.selection').click();
			}
			else
			{
				that.closeSelect();
			}
			// Selection Old Select
			$('.old-select option[selected]').removeAttr('selected');
			$('.old-select option[value="'+newValue+'"]').attr('selected','');

			// Visuellement l'option dans le old-select ne change pas
			// mais la value selectionnée est bien pris en compte lors
			// de l'envoi du formulaire. Test à l'appui.
		});
		this.addClickHandler(this.onClickCallback);
	}
}


/**
* Minimal controls for ScriptNodePlayer - this updated version uses the Promise based player APIs.
*
* <p>Features an initial playlist, drag&drop of additional music files, and controls for "play", "pause",
* "next song", "previous song", "seek" (optional), "volume".
*
* <p>This crude UI is not meant to be reusable but to show how the ScriptNodePlayer is used.
*/
class PlaylistPlayerWidget {
	constructor(containerId, songs, onNewTrackCallback, enableSeek, enableSpeedTweak, doParseUrl, doOnDropFile, currentTrack, defaultOptions)
	{
		this._containerId = containerId;
		this._onNewTrackCallback = onNewTrackCallback;

		this._playlistWidget = new PlaylistWidget(this.onPlaylistClick.bind(this));

		this._doOnDropFile = doOnDropFile;
		this._options = (typeof defaultOptions != 'undefined') ? defaultOptions : {};
		this._current = (typeof currentTrack != 'undefined') ? currentTrack : -1;

		this.setPlaylist(songs);

		this._enableSeek = enableSeek;
		this._enableSpeedTweak = enableSpeedTweak;
		this._doParseUrl = doParseUrl;

		this._initDomElements();
	}

	setPlaylist(songs)
	{
		if(Object.prototype.toString.call( songs ) === '[object Array]')
		{
			this._someSongs = songs;
		}
		else
		{
			console.log("warning: no valid song list supplied.. starting empty");
			this._someSongs = [];
		}
	}

	// facade for player functionality so that PlaylistPlayerWidget user does not also need to know the player
	pause() 				{ ScriptNodePlayer.getInstance().pause(); }
	resume() 				{ ScriptNodePlayer.getInstance().resume(); }
	setVolume(value) 		{ ScriptNodePlayer.getInstance().setVolume(value); }
	getSongInfo()			{ return ScriptNodePlayer.getInstance().getSongInfo(); }

	onPlaylistClick(idx, name)
	{
		this._playlistWidget.setSelection(idx);

		this._current = idx - 1;	// hack: playNextSong will count up
		this.playNextSong();
	}

	_addSong(filename)
	{
		this._someSongs.push(filename);

		// API to reconstruct from the list would be nicer
		// but I don't want to waste more time with that garbage PlaylistWidget impl
		this._playlistWidget.addEntry(this._makeLabel(filename));	// updates selection

		this._current = this._someSongs.length - 1;
		this._current -= 1;	// hack: playNextSong will count up
	}

	seekPos(relPos)
	{
		let p = ScriptNodePlayer.getInstance();
		if (p)
		{
			p.seekPlaybackPosition(Math.round(p.getMaxPlaybackPosition() * relPos));
		}
	}

	// some playlist handling
	removeFromPlaylist(songname)
	{
		/* avoid additional complexity for now -
		if (this._someSongs[this._current] == songname) {
			this._someSongs.splice(this._current, 1);
			if (this._current + 1 == this._someSongs.length) this._current= 0;
		}
		*/
	}

	playNextSong()
	{
		let p = ScriptNodePlayer.getInstance();
		if (p && this._someSongs.length)
		{
			this._current = (++this._current >= this._someSongs.length) ? 0 : this._current;

			this._playlistWidget.setSelection(this._current);

			let someSong = this._someSongs[this._current];
			this.playSong(someSong);
		}
	}

	playPreviousSong()
	{
		let p = ScriptNodePlayer.getInstance();
		if (p && this._someSongs.length)
		{
			this._current = (--this._current < 0) ? this._current + this._someSongs.length : this._current;

			this._playlistWidget.setSelection(this._current);

			let someSong = this._someSongs[this._current];
			this.playSong(someSong);
		}
	}

	_playSongWithBackand(options, onSuccess)
	{
		// backend adapter to be used has been explicitly specified
		let o = options.backendAdapter;

		ScriptNodePlayer.initialize(o.adapter, o.doOnTrackEnd, o.basePath, o.preload, o.enableSpectrum)
		.then((msg) => {
			onSuccess();
		});
	}

	playSong(someSong)
	{
		// handle Google's bullshit "autoplay policy"

		let audioCtx = ScriptNodePlayer.getWebAudioContext();
		if (audioCtx.state == "suspended")
		{
			let modal = document.getElementById('autoplayConfirm');
			modal.style.display = "block";		// force user to click

			window.globalDeferredPlay = function() {	// setup function to be used "onClick"
				audioCtx.resume();
				this._playSong(someSong);
			}.bind(this);
		}
		else
		{
			this._playSong(someSong);
		}
	}

	_loadSongFromURL(name, options)
	{
		let onFail = function(){ this.removeFromPlaylist(name); }.bind(this);

		ScriptNodePlayer.loadMusicFromURL(name, options, onFail).then((status) => {

			// this corresponds to onTrackReadyToPlay callback in old API (reminder: autoplays)

			// song's meta data can now be accessed
			let d = document.getElementById("songInfo");
			d.innerHTML = this._getInfoLine(this.getSongInfo());

			this._onNewTrackCallback();
		});
	}

	_playSong(someSong)
	{
		let arr = this._doParseUrl(someSong);
		let url = arr[0];
		let options = arr[1];

		if (typeof options.backendAdapter != 'undefined')
		{
			let onPlayerReady = function() { this._loadSongFromURL(url, options); }.bind(this);
			this._playSongWithBackand(options, onPlayerReady);
		}
		else
		{
			this._loadSongFromURL(url, options);
		}
	}

/*	animate()
	{
		// animate playback position slider
		let slider = document.getElementById("seekPos");
		if(slider && !slider.blockUpdates)
		{
			let p = ScriptNodePlayer.getInstance();
			slider.value = Math.round(255 * p.getPlaybackPosition() / p.getMaxPlaybackPosition());
		}
	}*/

	// ---------------------    drag&drop feature -----------------------------------
	_arrayify(f)
	{
		// convert original FileList garbage into a regular array (once again those
		// clueless morons where unable to design APIs that fit together..)

		let files = [];
		for (let i = 0; i < f.length; i++) {
			files.push(f[i]);
		}
		return files;
	}

	_drop(ev)
	{
		ev.preventDefault();

		let data = ev.dataTransfer.getData("Text");
		let files = this._arrayify(ev.dataTransfer.files);

		if (ScriptNodePlayer.getInstance())
		{
			files.forEach((file) => {
				file.xname = "/tmp/" + file.name;	// hack needed since file.name has no setter
			});

			ScriptNodePlayer.loadFileData(files).then((result) => {

				result.forEach((filename) => {
					this._addSong(filename);
				});
				this.playNextSong();
			});
		}
	}

	_initExtensions() {}	// to be overridden in subclass

	_allowDrop(ev)
	{
		ev.preventDefault();
		ev.dataTransfer.dropEffect = 'move'; 	// needed for FF
	}

	_initUserEngagement()
	{
		// handle Goggle's latest "autoplay policy" bullshit (patch the HTML/Script from here within this
		// lib so that the various existing html files need not be touched)

		let d = document.createElement("DIV");
		d.setAttribute("id", "autoplayConfirm");
		d.setAttribute("class", "modal-autoplay");

		let dc = document.createElement("DIV");
		dc.setAttribute("class", "modal-autoplay-content");

		let p = document.createElement("P");
		let t = document.createTextNode(
		"This page will auto play music. (You can white list this  \
		site to avoid this annoying dialog in the future.)\
		Click outside of this box to continue.");
		p.appendChild(t);

		dc.appendChild(p);
		d.appendChild(dc);

		document.body.insertBefore(d, document.body.firstChild);


		let s = document.createElement('script');
		s.text = 'var modal = document.getElementById("autoplayConfirm");\
			window.onclick = function(event) {\
				if (event.target == modal) {\
					modal.style.display = "none";\
					if (typeof window.globalDeferredPlay !== "undefined") { window.globalDeferredPlay();\
					delete window.globalDeferredPlay; }\
				}\
			}';
		document.body.appendChild(s);
	}

	_initTooltip() {
		let tooltipDiv = document.getElementById("tooltip");
		tooltipDiv.setAttribute("alt", "This is a hobby project, but it costs not only time to regularly maintain this site but also money to pay for the internet service provider (etc). If you want to keep this site up and running.. or if you just like my work and you'd like to see more of it in the future, buy me a cup of coffee. Thank you!");

		let f = document.createElement("form");
		f.setAttribute('method',"post");
		f.setAttribute('action',"https://www.paypal.com/cgi-bin/webscr");
		f.setAttribute('target',"_blank");

		let i1 = document.createElement("input");
		i1.type = "hidden";
		i1.value = "_s-xclick";
		i1.name = "cmd";
		f.appendChild(i1);

		let i2 = document.createElement("input");
		i2.type = "hidden";
		i2.value = "E7ACAHA7W5FYC";
		i2.name = "hosted_button_id";
		f.appendChild(i2);

		let i3 = document.createElement("input");
		i3.type = "image";
		i3.src = "stdlib/btn_donate_LG.gif";
		i3.className = "donate_btn";
		i3.border = "0";
		i3.name = "submit";
		i3.alt = "PayPal - The safer, easier way to pay online!";
		f.appendChild(i3);

		let i4 = document.createElement("img");
		i4.alt = "";
		i4.border = "0";
		i4.src = "stdlib/pixel.gif";
		i4.width = "1";
		i4.height = "1";
		f.appendChild(i4);

		tooltipDiv.appendChild(f);
	}

	_initDrop() {
		// the 'window' level handlers are needed to show a useful mouse cursor in Firefox
		window.addEventListener("dragover",function(e){
		  e = e || event;
		  e.preventDefault();
		  e.dataTransfer.dropEffect = 'none';
		},true);

		window.addEventListener("drop",function(e){
		  e = e || event;
		  e.preventDefault();
		},true);

		let dropDiv = document;	// no point to restrict drop to sub-elements..
		dropDiv.ondrop  = this._drop.bind(this);
		dropDiv.ondragover  = this._allowDrop.bind(this);
	}

	_appendControlElement(elmt)
	{
		let controls = document.getElementById(this._containerId);
		controls.appendChild(elmt);
		controls.appendChild(document.createTextNode(" "));  	// spacer
	}

	_addSlider(name, label, min, max, pos, onStop)
	{
		let d = document.createElement("div");
		d.id = name;
		d.name = name;
		d.className = "slider " + name;

		// just to make Lighthouse happy: Google's dumbshit tool "requires" labels for respective
		// input elements (even if empty) ..
		let l = document.createElement("label");

		l.innerHTML = label;
		l.className = "sliderIconLabel";
		l.appendChild(d);

		this._appendControlElement(l);

		$("#" + name).slider({
				orientation: "horizontal",
				range: "min",
				min: min,
				max: max,
				value: pos,
				animate: 200,
				slide: onStop
				});
		$( "#" + name ).on( "slidestop", onStop );
	}

	_addPlayerButton(id, label, onClick)
	{
		let btn = document.createElement("BUTTON");

		btn.id = id;
		btn.className = "playerBtn";
		btn.innerHTML = label;
		btn.onclick = onClick;

		this._appendControlElement(btn);
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
		name = name.substr(0, name.indexOf('.'));

		if (name.length > 40)	// make sure it fits on a single line
		{
			let a =  name.split("-");
			name = a[a.length-1].trim();
		}
		
		while (name.length > 40)	// make sure it fits on a single line
		{
			let lastIndex = name.lastIndexOf(" ");
			name = name.substring(0, lastIndex);
		}

		return name;
	}

	_addPlaylist()
	{
		let d = document.createElement("DIV");
		d.className = "playlistSelect";
		let sel= " <select class=\"old-select\" >";

		for (let i = 0; i < this._someSongs.length; i++) {
			sel +=  "<option value=\"opt" + i + "\">" + this._makeLabel(this._someSongs[i]) + "</option>";
		}
		sel += "</select><div class=\"new-select\" ></div>";	// silly construct currently used by PlaylistWidget impl

		d.innerHTML = sel;
		this._appendControlElement(d);
	}

	// overwrite in subclass if additional info should be shown
	_getInfoLine(info)
	{
		return ""; 	// default: nothing to show
	}

	_addInfoLine()
	{
		let d = document.createElement("DIV");
		d.className = "songInfo";
		d.innerHTML = "<p id=\"songInfo\">&nbsp;</p>";	// note: centering <p> works.. with <span> it doesn't
		this._appendControlElement(d);
	}

	_initDomElements()
	{
		// symbolic names do longer work in stripped down font..
		this._addPlayerButton("play", "<span class=\"material-icons\">&#57399;</span>",  function(e){ e.stopPropagation(); this.resume(); }.bind(this) );
		this._addPlayerButton("pause", "<span class=\"material-icons\">&#57396;</span>",  function(e){ e.stopPropagation(); this.pause(); }.bind(this) );
		this._addPlayerButton("previous", "<span class=\"material-icons\">&#57413;</span>",  function(e){ e.stopPropagation(); this.playPreviousSong(); }.bind(this));
		this._addPlayerButton("next", "<span class=\"material-icons\">&#57412;</span>",  function(e){ e.stopPropagation(); this.playNextSong(); }.bind(this) );

		let off = "<span class=\"material-icons\">&#57423;</span>";
		let low = "<span class=\"material-icons\">&#57422;</span>";
		let high = "<span class=\"material-icons\">&#57424;</span>";

		$(".playerRow").click(function() {
			this._playlistWidget.closeSelect();	// in case it was open
		}.bind(this));


		this._addSlider("gain", high, 0, 255, 255, function( event, ui ) {
				let gain = document.getElementById("gain").parentElement.firstChild;

				if (ui.value ==  0)
				{
					gain.innerHTML = off;
				}
				else if (ui.value < 127)
				{
					gain.innerHTML = low;
				}
				else
				{
					gain.innerHTML = high;
				}

				this.setVolume(parseInt(ui.value) / 255);
			}.bind(this));

		if (this._enableSeek)
		{
			let evtHandler = function( e, ui ) { e.stopPropagation();  this.seekPos(ui.value / 255); }.bind(this);
			this._addSlider("seekPos", "seek", 0, 255, 0, evtHandler);
		}

		if (this._enableSpeedTweak)
		{
			let evtHandler = function( e, ui ) {
									e.stopPropagation();

									let p = ScriptNodePlayer.getInstance();

									let tweak = 0.2; 			// allow 20% speed correction
									let f= ((100 - ui.value) / 50) - 1;	// -1..1

									let s = ScriptNodePlayer.getWebAudioSampleRate();
									s = Math.round(s * (1 + (tweak * f)));
									p.resetSampleRate(s);
								}.bind(this);

			this._addSlider("speed", "<span class=\"material-icons\">&#59876;</span>", 0,100,50, evtHandler);
		}

		this._addPlaylist();
		this._playlistWidget.reinit();

		this._addInfoLine();

		this._initUserEngagement();
		this._initDrop();
		this._initTooltip();

		this._initExtensions();
	}
}

/**
* Utility used to draw a number of scope streams on HTML canvas.
*/
class OscilloscopesWidget {
	constructor(divId, tracer, backend, width, height)
	{
		this._divId = divId;
		this._channelStreamer = tracer;
		this._backend = backend;
		this._useSyncMode = true;
		this._zoomLevel = 5;

		this._width = (typeof width == 'undefined') ? 300 : width;
		this._height = (typeof height == 'undefined') ? 60 : height;
		
		this._numScopes = 0;
	}

	_genCanvId(i)
	{
		let id = "voice" + i + "Canvas";
		return id;
	}

	_syncUI()
	{
		let n = this._backend.getNumberTraceStreams();
		if (n != this._numScopes)
		{
			this._scopeDisplays = [];

			this._generateHTML(this, n);

			for (let i= 0; i<n; i++) {
				let d =  new VoiceDisplay(this._genCanvId(i), this._channelStreamer,
									function() { return this._channelStreamer.getData(i);}.bind(this), true);
				d.setSize(this._width, this._height);

				this._scopeDisplays.push(d);
			}
			this._numScopes = n;
		}
	}

	_generateHTML(display, n)
	{
		let cols = 4;
		let t =	"<div class=\"oscRow\">";

		for(let x = 0, i = 0; x < cols; x++) {

			t +=		"<div class=\"osc" +(x+1)+"\">";

			for (let y = 0; y < n/cols; y++) {
				t += "<div class=\"panHead\">channel "+i+"</div>"+
				  "<div class=\"voiceContainer\"><canvas class=\"voiceCanvas\" id=\""+this._genCanvId(i)+"\" ></canvas></div>";
				i++;
			}
			t += 		"</div>";
		}

		t +=	"</div>";

		let div = document.getElementById(this._divId);
		div.innerHTML = t;
	}

	onSongChanged()
	{
		if (typeof this._lastId != 'undefined')
		{
			window.cancelAnimationFrame(this._lastId);	// prevent duplicate chains
		}

		this._redrawRepeated();
	}

	_redrawRepeated()
	{
		this._redraw();

		this._lastId = window.requestAnimationFrame(this._redrawRepeated.bind(this));
	}

	_redraw()
	{
		this._syncUI();

		for (let i= 0; i<this._numScopes; i++) {
			this._scopeDisplays[i].redrawGraph();
		}
	}

	toggleMode()
	{
		this._useSyncMode = !this._useSyncMode;

		for (const display of this._scopeDisplays) {
			display.setSyncMode(this._useSyncMode);
		}
	}

	setZoom(zoom)
	{
		this._zoomLevel = zoom;

		this._channelStreamer.setZoom(this._zoomLevel);
	}
}


$(function() {	// call once the DOM has also been loaded

	$('html').addClass($.fn.details.support ? 'details' : 'no-details');
	$('details').details();

	// initially expand details
	if (!(window.openDetails === 'undefined') && window.openDetails)
	{
		if ($.fn.details.support)
		{
			$('details').attr('open', '');	// Chrome
		}
		else
		{
			let $details = $('details');
			let $detailsSummary = $('summary', $details).first();
			let $detailsNotSummary = $details.children(':not(summary)');

			$details.addClass('open').prop('open', true).triggerHandler('open.details');
			$detailsNotSummary.show();
		}
	}
});
