/*
 * BaseSelector.java - ASAP for Android
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

import android.app.ListActivity;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.net.Uri;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

abstract class BaseSelector extends ListActivity
{
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setVolumeControlStream(AudioManager.STREAM_MUSIC);
	}

	protected Uri uri;

	protected static class FileInfoAdapter extends ArrayAdapter<FileInfo>
	{
		private LayoutInflater layoutInflater;

		protected FileInfoAdapter(Context context, int rowViewResourceId, FileInfo[] infos)
		{
			super(context, rowViewResourceId, infos);
			layoutInflater = LayoutInflater.from(context);
		}

		private static class ViewHolder
		{
			private TextView title;
			private TextView author;
			private TextView date;
			private TextView songs;
		}

		public View getView(int position, View convertView, ViewGroup parent)
		{
			ViewHolder holder;
			if (convertView == null) {
				convertView = layoutInflater.inflate(R.layout.fileinfo_list_item, null);
				holder = new ViewHolder();
				holder.title = (TextView) convertView.findViewById(R.id.title);
				holder.author = (TextView) convertView.findViewById(R.id.author);
				holder.date = (TextView) convertView.findViewById(R.id.date);
				holder.songs = (TextView) convertView.findViewById(R.id.songs);
				convertView.setTag(holder);
			}
			else
				holder = (ViewHolder) convertView.getTag();

			FileInfo info = getItem(position);
			holder.title.setText(info.title);
			holder.author.setText(info.author);
			holder.date.setText(info.date);
			holder.songs.setText(info.songs > 1 ? getContext().getString(R.string.songs_format, info.songs) : null);

			return convertView;
		}
	}

	@Override
	protected void onListItemClick(ListView l, View v, int position, long id)
	{
		Intent intent;
		FileInfo info = (FileInfo) l.getItemAtPosition(position);
		String name = info.filename;
		if (name == null) {
			// shuffle all
			intent = new Intent(Intent.ACTION_VIEW, uri, this, Player.class);
		}
		else {
			Class klass = ASAPInfo.isOurFile(name) ? Player.class : FileSelector.class;
			intent = new Intent(Intent.ACTION_VIEW, Util.buildUri(uri, name), this, klass);
			if (Util.endsWithIgnoreCase(uri.toString(), ".m3u"))
				intent.putExtra(PlayerService.EXTRA_PLAYLIST, uri.toString());
		}
		startActivity(intent);
	}
}
