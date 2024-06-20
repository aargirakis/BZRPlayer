/*
 * FileInfo.java - ASAP for Android
 *
 * Copyright (C) 2010-2019  Piotr Fusik
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

import android.content.Context;
import java.io.IOException;
import java.io.LineNumberReader;
import java.text.Collator;
import java.util.ArrayList;
import java.util.Comparator;

class FileInfo implements Comparable<FileInfo>
{
	String filename;
	String title;
	String author;
	String date;
	int songs;

	FileInfo(String filename)
	{
		this.title = this.filename = filename;
	}

	@Override
	public String toString()
	{
		return title;
	}

	@Override
	public boolean equals(Object obj)
	{
		if (!(obj instanceof FileInfo))
			return false;
		FileInfo that = (FileInfo) obj;
		if (this.filename == null)
			return that.filename == null;
		return this.filename.equals(that.filename);
	}

	@Override
	public int hashCode()
	{
		return filename == null ? 0 : filename.hashCode();
	}

	private static Comparator<? super String> comparator;

	private static Comparator<? super String> getComparator()
	{
		if (comparator == null) {
			synchronized (FileInfo.class) {
				if (comparator == null)
					comparator = Collator.getInstance();
			}
		}
		return comparator;
	}

	public int compareTo(FileInfo that)
	{
		if (this.filename == null)
			return -1;
		if (that.filename == null)
			return 1;
		boolean dir1 = this.filename.endsWith("/");
		boolean dir2 = that.filename.endsWith("/");
		if (dir1 != dir2)
			return dir1 ? -1 : 1;
		Comparator<? super String> comparator = getComparator();
		int titleCmp = comparator.compare(this.title, that.title);
		if (titleCmp != 0)
			return titleCmp;
		return comparator.compare(this.filename, that.filename);
	}

	static FileInfo getShuffleAll(Context context)
	{
		FileInfo info = new FileInfo(null);
		info.title = context.getString(R.string.shuffle_all);
		return info;
	}

	static FileInfo[] listIndex(Context context, String query)
	{
		ArrayList<FileInfo> coll = new ArrayList<FileInfo>();
		if (query == null)
			coll.add(getShuffleAll(context));
		try (LineNumberReader r = Util.openIndex(context)) {
			for (;;) {
				String name = r.readLine();
				if (name == null)
					break;
				String title = r.readLine();
				String author = r.readLine();
				String date = r.readLine();
				String songs = r.readLine();
				if (query == null || Util.matches(title, query) || Util.matches(author, query)) {
					FileInfo info = new FileInfo(name);
					info.title = title;
					info.author = author;
					info.date = date;
					info.songs = Integer.parseInt(songs);
					coll.add(info);
				}
			}
		}
		catch (IOException ex) {
			// shouldn't happen
		}
		return coll.toArray(new FileInfo[coll.size()]);
	}
}
