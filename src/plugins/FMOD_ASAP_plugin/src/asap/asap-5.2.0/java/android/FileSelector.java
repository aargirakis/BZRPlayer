/*
 * FileSelector.java - ASAP for Android
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

import android.Manifest;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;
import android.view.MenuItem;
import android.view.inputmethod.InputMethodManager;
import android.widget.ArrayAdapter;
import android.widget.ListAdapter;
import android.widget.Toast;
import java.io.InputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collection;
import java.util.TreeSet;

public class FileSelector extends BaseSelector
{
	private boolean isSearch;
	private boolean isDetails;

	private class FileInfoList extends FileContainer
	{
		private Collection<FileInfo> coll;
		private int songFiles;

		@Override
		protected void onSongFile(String name, InputStream is)
		{
			FileInfo fi = new FileInfo(name);
			if (is != null) {
				ASAPInfo info = new ASAPInfo();
				try {
					byte[] module = new byte[ASAPInfo.MAX_MODULE_LENGTH];
					int moduleLen = Util.readAndClose(is, module);
					info.load(name, module, moduleLen);
				}
				catch (Exception ex) {
					// ignore files we cannot read or understand
					return;
				}
				fi.title = info.getTitleOrFilename();
				fi.author = info.getAuthor();
				fi.date = info.getDate();
				fi.songs = info.getSongs();
			}
			coll.add(fi);
			songFiles++;
		}

		@Override
		protected void onContainer(String name)
		{
			coll.add(new FileInfo(name));
		}

		FileInfo[] list() throws IOException
		{
			boolean isM3u = Util.endsWithIgnoreCase(uri.toString(), ".m3u");
			coll = isM3u ? new ArrayList<FileInfo>() : new TreeSet<FileInfo>();
			songFiles = 0;
			list(FileSelector.this, uri, isDetails, false);

			// "(shuffle all)" if any song files or non-empty ZIP directory
			if (songFiles > 1 || (!coll.isEmpty() && Util.endsWithIgnoreCase(uri.getPath(), ".zip"))) {
				FileInfo shuffleAll = FileInfo.getShuffleAll(FileSelector.this);
				if (isM3u)
					((ArrayList<FileInfo>) coll).add(0, shuffleAll); // insert at the beginning
				else
					coll.add(shuffleAll);
			}

			return coll.toArray(new FileInfo[coll.size()]);
		}
	}

	private void reload()
	{
		uri = getIntent().getData();
		if (uri == null)
			uri = Uri.fromFile(Environment.getExternalStorageDirectory());

		String title = uri.getPath();
		String fragment = uri.getFragment();
		if (fragment != null)
			title += "#" + fragment;
		setTitle(getString(R.string.selector_title, title));

		FileInfo[] infos;
		try {
			infos = new FileInfoList().list();
		}
		catch (IOException ex) {
			infos = new FileInfo[0];
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M
			 && checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_DENIED)
				requestPermissions(new String[] { Manifest.permission.READ_EXTERNAL_STORAGE }, 1);
			else
				Toast.makeText(this, R.string.access_denied, Toast.LENGTH_SHORT).show();
		}

		ListAdapter adapter = isDetails
			? new FileInfoAdapter(this, R.layout.fileinfo_list_item, infos)
			: new ArrayAdapter<FileInfo>(this, R.layout.filename_list_item, infos);
		setListAdapter(adapter);
	}

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		getActionBar().setDisplayHomeAsUpEnabled(true);
		getListView().setTextFilterEnabled(true);
		isDetails = getPreferences(MODE_PRIVATE).getBoolean("fileDetails", false);
		reload();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults)
	{
		super.onRequestPermissionsResult(requestCode, permissions, grantResults);
		if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED)
			reload();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.file_selector, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId()) {
		case android.R.id.home:
			finish();
			return true;
		case R.id.menu_search:
			InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);
			if (isSearch) {
				imm.hideSoftInputFromWindow(getListView().getWindowToken(), 0);
				getListView().clearTextFilter();
				isSearch = false;
			}
			else {
				imm.showSoftInput(getListView(), 0);
				isSearch = true;
			}
			return true;
		case R.id.menu_toggle_details:
			isDetails = !isDetails;
			getPreferences(MODE_PRIVATE).edit().putBoolean("fileDetails", isDetails).commit();
			reload();
			return true;
		default:
			return false;
		}
	}
}
