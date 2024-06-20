/*
 * Indexer.java - prepares text-file index of SAP files
 *
 * Copyright (C) 2014  Piotr Fusik
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

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.IOException;
import java.util.Arrays;

import net.sf.asap.ASAPInfo;

class Indexer
{
	private static String rootPath;

	/**
	 * Reads bytes from the stream into the byte array
	 * until end of stream or array is full.
	 * @param is source stream
	 * @param b output array
	 * @return number of bytes read
	 */
	private static int readAndClose(InputStream is, byte[] b) throws IOException
	{
		int got = 0;
		int len = b.length;
		try {
			while (got < len) {
				int i = is.read(b, got, len - got);
				if (i <= 0)
					break;
				got += i;
			}
		}
		finally {
			is.close();
		}
		return got;
	}

	private static void process(File dir) throws Exception
	{
		File[] children = dir.listFiles();
		Arrays.sort(children);
		for (File file : children) {
			if (file.isDirectory())
				process(file);
			else {
				String path = file.getPath();
				if (ASAPInfo.isOurFile(path)) {
					if (!path.startsWith(rootPath))
						throw new IllegalArgumentException(path);
					path = path.substring(rootPath.length()).replace(File.separatorChar, '/');
					byte[] module = new byte[ASAPInfo.MAX_MODULE_LENGTH];
					int moduleLen = readAndClose(new FileInputStream(file), module);
					ASAPInfo info = new ASAPInfo();
					info.load(path, module, moduleLen);
					System.out.println(path);
					System.out.println(info.getTitleOrFilename());
					System.out.println(info.getAuthor());
					System.out.println(info.getDate());
					System.out.println(info.getSongs());
				}
			}
		}
	}
	
	public static void main(String[] args) throws Exception
	{
		if (args.length != 1)
			throw new IllegalArgumentException("Usage: java Indexer PATH_TO_ASMA >asma.txt");
		File rootDir = new File(args[0]);
		rootPath = rootDir.getPath() + File.separatorChar;
		process(rootDir);
	}
}
