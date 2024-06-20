/*
 * main.swift - converter of ASAP-supported formats to WAV files
 *
 * Copyright (C) 2021  Piotr Fusik
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

import Foundation

var outputFilenameOption : String?
var songOption : Int?
var durationOption : Int?
var format = ASAPSampleFormat.s16LE
var outputHeader = true
var muteMask = 0

func printHelp()
{
	print("""
Usage: asap2wav [OPTIONS] INPUTFILE...
Each INPUTFILE must be in a supported format:
SAP, CMC, CM3, CMR, CMS, DMC, DLT, MPT, MPD, RMT, TMC, TM8, TM2 or FC.
Options:
-o FILE     --output=FILE      Set output file name
-s SONG     --song=SONG        Select subsong number (zero-based)
-t TIME     --time=TIME        Set output length (MM:SS format)
-b          --byte-samples     Output 8-bit samples
-w          --word-samples     Output 16-bit samples (default)
            --raw              Output raw audio (no WAV header)
-m CHANNELS --mute=CHANNELS    Mute POKEY channels (1-8)
-h          --help             Display this information
-v          --version          Display version information
""")
}

func getOptionValue(_ i : inout Int, _ option : String) -> String?
{
	let arg = CommandLine.arguments[i]
	if arg == "-" + option.prefix(1) {
		i += 1
		return CommandLine.arguments[i]
	}
	let longOption = "--" + option + "="
	if arg.hasPrefix(longOption) {
		return String(arg[longOption.endIndex...])
	}
	return nil
}

func processFile(_ inputFilename : String) throws
{
	if let data = FileManager.default.contents(atPath: inputFilename) {
		let asap = ASAP()
		try asap.load(inputFilename, ArrayRef(Array(data)), data.count)
		let info = asap.getInfo()!
		let song = songOption ?? info.getDefaultSong()
		var duration = durationOption ?? info.getDuration(song)
		if duration < 0 {
			duration = 180_000
		}
		try asap.playSong(song, duration)
		asap.mutePokeyChannels(muteMask)
		let outputFilename = outputFilenameOption ?? inputFilename.deletingPathExtension.appendingPathExtension(outputHeader ? "wav" : "raw")!
		let buffer = ArrayRef<UInt8>(repeating: 0, count: 8192)
		if let f = OutputStream(toFileAtPath: outputFilename, append: false) {
			f.open()
			var ok = true
			if outputHeader {
				let headerLen = asap.getWavHeader(buffer, format, false)
				ok = f.write(buffer.array, maxLength : headerLen) == headerLen
			}
			while ok {
				let bufferLen = asap.generate(buffer, 8192, format)
				if bufferLen > 0 {
					ok = f.write(buffer.array, maxLength : bufferLen) == bufferLen
				}
				if bufferLen != 8192 {
					break
				}
			}
			f.close()
			if !ok {
				print(outputFilename, ": error writing file", separator: "")
			}
		}
		else {
			print(outputFilename, ": cannot write file", separator: "")
		}
	}
	else {
		print(inputFilename, ": cannot open file", separator: "")
	}
	outputFilenameOption = nil
	songOption = nil
	durationOption = nil
}

var noInputFiles = true
var i = 1
while i < CommandLine.argc {
	let arg = CommandLine.arguments[i]
	if !arg.hasPrefix("-") {
		try processFile(arg)
		noInputFiles = false
	}
	else if let value = getOptionValue(&i, "output") {
		outputFilenameOption = value
	}
	else if let value = getOptionValue(&i, "song") {
		songOption = Int(value)
	}
	else if let value = getOptionValue(&i, "time") {
		durationOption = try ASAPInfo.parseDuration(value)
	}
	else if arg == "-b" || arg == "--byte-samples" {
		format = ASAPSampleFormat.u8
	}
	else if arg == "-w" || arg == "--word-samples" {
		format = ASAPSampleFormat.s16LE
	}
	else if arg == "--raw" {
		outputHeader = false
	}
	else if let value = getOptionValue(&i, "mute") {
		for c in value {
			if let i = c.wholeNumberValue {
				if i >= 1 && i <= 8 {
					muteMask |= 1 << (i - 1)
				}
			}
		}
	}
	else if arg == "-h" || arg == "--help" {
		printHelp()
		noInputFiles = false
	}
	else if arg == "-v" || arg == "--version" {
		print("ASAP2WAV (Swift)", ASAPInfo.version)
		noInputFiles = false
	}
	else {
		fatalError("unknown option: " + arg)
	}
	i += 1
}
if noInputFiles {
	printHelp()
	exit(1)
}
