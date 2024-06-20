/*
 * AATRFileInputStream.java - ASAP for Android
 *
 * Copyright (C) 2015-2018  Piotr Fusik
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

package net.sf.asap;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.IOException;

public class AATRFileInputStream extends InputStream
{
	private final JavaAATR atr;
	private final AATRFileStream fileStream = new AATRFileStream();
	private byte[] byteBuffer = null;

	public AATRFileInputStream(AATRDirectory directory)
	{
		atr = null;
		fileStream.open(directory);
	}

	public AATRFileInputStream(JavaAATR atr, String filename) throws IOException
	{
		AATRDirectory directory = new AATRDirectory();
		directory.openRoot(atr);
		if (!directory.findEntryRecursively(filename) || directory.isEntryDirectory())
			throw new FileNotFoundException(filename);
		fileStream.open(directory);
		this.atr = atr;
	}

	@Override
	public int read() throws IOException
	{
		if (byteBuffer == null)
			byteBuffer = new byte[1];
		return fileStream.read(byteBuffer, 0, 1) > 0 ? byteBuffer[0] : -1;
	}

	@Override
	public int read(byte[] b, int off, int len) throws IOException
	{
		if (b == null)
			throw new NullPointerException();
		if (off < 0 || len < 0 || len > b.length - off)
			throw new IndexOutOfBoundsException();
		int result = fileStream.read(b, off, len);
		if (result == 0 && len != 0)
			return -1;
		return result;
	}

	@Override
	public void close() throws IOException
	{
		if (atr != null)
			atr.close();
	}
}
