/*
 * asapweb.js - pure JavaScript ASAP for web browsers
 *
 * Copyright (C) 2009-2021  Piotr Fusik
 *
 * This file is part of ASAP (Another Slight Atari Player),
 * see http://asap.sourceforge.net
 *
 * ASAP is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * ASAP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ASAP; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

const asap = {
	stop : function()
	{
		if (this.processor) {
			this.processor.disconnect();
			delete this.processor;
		}
	},

	playContent : function(filename, content, song)
	{
		const asap = new ASAP();
		asap.load(filename, content, content.length);
		const info = asap.getInfo();
		if (song === undefined)
			song = info.getDefaultSong();
		asap.playSong(song, -1);

		this.stop();
		const length = 4096;
		const channels = asap.getInfo().getChannels();
		const buffer = new Uint8Array(new ArrayBuffer(length * channels));

		const AudioContext = window.AudioContext || window.webkitAudioContext;
		if (this.context)
			this.context.close();
		this.context = new AudioContext({ sampleRate : ASAP.SAMPLE_RATE });
		if (typeof(this.onUpdate) == "function")
			this.context.onstatechange = this.onUpdate;
		this.processor = this.context.createScriptProcessor(length, 0, channels);
		this.processor.onaudioprocess = e => {
			asap.generate(buffer, length * channels, ASAPSampleFormat.U8);
			for (let c = 0; c < channels; c++) {
				const output = e.outputBuffer.getChannelData(c);
				for (let i = 0; i < length; i++)
					output[i] = (buffer[i * channels + c] - 128) / 128;
			}
			if (typeof(this.onUpdate) == "function")
				this.onUpdate();
		};
		this.processor.connect(this.context.destination);
		this.asap = asap;
		if (typeof(this.onUpdate) == "function")
			this.onUpdate();
	},

	togglePause : function()
	{
		if (this.context) {
			switch (this.context.state) {
			case "running":
				this.context.suspend();
				return true;
			case "suspended":
				this.context.resume();
				return false;
			default:
				break;
			}
		}
		return null;
	},

	isPaused : function()
	{
		if (this.context) {
			switch (this.context.state) {
			case "running":
				return false;
			case "suspended":
				return true;
			default:
				break;
			}
		}
		return null;
	},

	playUrl : function(url, song)
	{
		const request = new XMLHttpRequest();
		request.open("GET", url, true);
		request.responseType = "arraybuffer";
		request.onload = e => {
			if (request.status == 200 || request.status == 0)
				this.playContent(url, new Uint8Array(request.response), song);
		};
		request.send();
	},

	playFile : function(file)
	{
		const reader = new FileReader();
		reader.onload = e => this.playContent(file.name, new Uint8Array(e.target.result));
		reader.readAsArrayBuffer(file);
	}
};
