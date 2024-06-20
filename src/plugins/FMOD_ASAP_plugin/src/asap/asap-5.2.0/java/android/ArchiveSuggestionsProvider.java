/*
 * ArchiveSuggestionsProvider.java - ASAP for Android
 *
 * Copyright (C) 2020  Piotr Fusik
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

import android.app.SearchManager;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.provider.BaseColumns;
import java.io.IOException;
import java.io.LineNumberReader;
import java.util.Map;
import java.util.TreeMap;

public class ArchiveSuggestionsProvider extends ContentProvider
{
	private static final String[] COLUMN_NAMES = {
		BaseColumns._ID,
		SearchManager.SUGGEST_COLUMN_TEXT_1,
		SearchManager.SUGGEST_COLUMN_TEXT_2,
		SearchManager.SUGGEST_COLUMN_INTENT_ACTION,
		SearchManager.SUGGEST_COLUMN_INTENT_DATA,
		SearchManager.SUGGEST_COLUMN_QUERY };

	@Override
	public boolean onCreate()
	{
		return true;
	}

	@Override
	public Cursor query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
	{
		String query = uri.getLastPathSegment();
		MatrixCursor cursor = new MatrixCursor(COLUMN_NAMES);
		TreeMap<String, Integer> authorToCount = new TreeMap<String, Integer>();
		Context context = getContext();
		Object[] columnValues = new Object[6];
		try (LineNumberReader r = Util.openIndex(context)) {
			for (int id = 1; ; id++) {
				String name = r.readLine();
				if (name == null)
					break;
				String title = r.readLine();
				String authors = r.readLine();
				/*String date =*/ r.readLine();
				/*String songs =*/ r.readLine();
				if (Util.matches(title, query)) {
					columnValues[0] = id;
					columnValues[1] = title;
					columnValues[2] = authors;
					columnValues[3] = Intent.ACTION_VIEW;
					columnValues[4] = "asma:" + Uri.encode(name);
					columnValues[5] = null;
					cursor.addRow(columnValues);
				}
				for (String author : authors.split(" & ")) {
					if (Util.matches(author, query)) {
						if (author.endsWith(" <?>"))
							author = author.substring(0, author.length() - 4);
						Integer count = authorToCount.get(author);
						authorToCount.put(author, count == null ? 1 : count + 1);
					}
				}
			}
		}
		catch (IOException ex) {
			// shouldn't happen
		}

		int id = -1;
		for (Map.Entry<String, Integer> entry : authorToCount.entrySet()) {
			columnValues[0] = id--;
			columnValues[1] = entry.getKey();
			columnValues[2] = context.getString(R.string.author_suggestion_songs, entry.getValue());
			columnValues[3] = Intent.ACTION_SEARCH;
			columnValues[4] = null;
			columnValues[5] = entry.getKey();
			cursor.addRow(columnValues);
		}

		return cursor;
	}

	@Override
	public int delete(Uri uri, String selection, String[] selectionArgs)
	{
		return 0;
	}

	@Override
	public String getType(Uri uri)
	{
		return null;
	}

	@Override
	public Uri insert(Uri uri, ContentValues values)
	{
		return null;
	}

	@Override
	public int update(Uri uri, ContentValues values, String selection, String[] selectionArgs)
	{
		return 0;
	}
}
