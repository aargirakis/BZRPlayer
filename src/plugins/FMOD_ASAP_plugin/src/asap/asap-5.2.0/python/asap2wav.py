# asap2wav.py - converter of ASAP-supported formats to WAV files
#
# Copyright (C) 2020 Piotr Fusik
#
# This file is part of ASAP (Another Slight Atari Player),
# see http://asap.sourceforge.net
#
# ASAP is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation; either version 2 of the License,
# or (at your option) any later version.
#
# ASAP is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ASAP; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

from asap import ASAP, ASAPInfo, ASAPSampleFormat
from pathlib import Path
import sys

output_filename = None
song = None
duration = None
format = ASAPSampleFormat.S16_L_E
output_header = True
mute_mask = 0

def print_help():
	print(
		"Usage: python asap2wav.py [OPTIONS] INPUTFILE...\n"
		"Each INPUTFILE must be in a supported format:\n"
		"SAP, CMC, CM3, CMR, CMS, DMC, DLT, MPT, MPD, RMT, TMC, TM8, TM2 or FC.\n"
		"Options:\n"
		"-o FILE     --output=FILE      Set output file name\n"
		"-s SONG     --song=SONG        Select subsong number (zero-based)\n"
		"-t TIME     --time=TIME        Set output length (MM:SS format)\n"
		"-b          --byte-samples     Output 8-bit samples\n"
		"-w          --word-samples     Output 16-bit samples (default)\n"
		"            --raw              Output raw audio (no WAV header)\n"
		"-m CHANNELS --mute=CHANNELS    Mute POKEY channels (1-8)\n"
		"-h          --help             Display this information\n"
		"-v          --version          Display version information")

def set_mute_mask(s):
	mute_mask = 0
	for c in s:
		if "1" <= c <= "8":
			mute_mask |= 1 << (int(c) - 1)

def process_file(input_filename):
	with open(input_filename, "rb") as f: module = f.read()
	asap = ASAP()
	asap.load(input_filename, module, len(module))
	info = asap.get_info()
	global output_filename, song, duration, format, output_header, mute_mask
	if song is None:
		song = info.get_default_song()
	if duration is None:
		duration = info.get_duration(song)
		if duration < 0:
			duration = 180_000
	asap.play_song(song, duration)
	asap.mute_pokey_channels(mute_mask)
	if output_filename is None:
		output_filename = Path(input_filename).with_suffix(".wav" if output_header else ".raw")
	buffer = bytearray(8192)
	with open(output_filename, "wb") as f:
		if output_header:
			buffer_len = asap.get_wav_header(buffer, format, False)
			f.write(buffer[:buffer_len])
		buffer_len = 8192
		while buffer_len == 8192:
			buffer_len = asap.generate(buffer, 8192, format)
			f.write(buffer[:buffer_len])
	output_filename = None
	song = None
	duration = None

args = sys.argv[1:]
no_input_files = True
while (args):
	arg = args.pop(0)
	if arg[0] != "-":
		process_file(arg)
		no_input_files = False
	elif arg == "-o":
		output_filename = args.pop(0)
	elif arg.startswith("--output="):
		output_filename = arg[9:]
	elif arg == "-s":
		song = int(args.pop(0))
	elif arg.startswith("--song="):
		song = int(arg[7:])
	elif arg == "-t":
		duration = ASAPInfo.parse_duration(args.pop(0))
	elif arg.startswith("--time="):
		duration = ASAPInfo.parse_duration(arg[7:])
	elif arg == "-b" or arg == "--byte-samples":
		format = ASAPSampleFormat.U8
	elif arg == "-w" or arg == "--word-samples":
		format = ASAPSampleFormat.S16_L_E
	elif arg == "--raw":
		output_header = False
	elif arg == "-m":
		set_mute_mask(args.pop(0))
	elif arg == "--mute=":
		set_mute_mask(arg[7:])
	elif arg == "-h" or arg == "--help":
		print_help()
		no_input_files = False
	elif arg == "-v" or arg == "--version":
		print("ASAP2WAV (Python)", ASAPInfo.VERSION)
		no_input_files = False
	else:
		raise Exception("unknown option: " + arg)
if no_input_files:
	print_help()
	sys.exit(1)
