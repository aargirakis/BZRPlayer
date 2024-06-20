/*
 * ArchiveSelector.java - ASAP for Android
 *
 * Copyright (C) 2015-2020  Piotr Fusik
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
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.SearchView;

public class ArchiveSelector extends BaseSelector
{
	@Override
	public void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		Intent intent = getIntent();
		if (Intent.ACTION_VIEW.equals(intent.getAction())) {
			finish();
			startActivity(new Intent(Intent.ACTION_VIEW, intent.getData(), this, Player.class));
		}
		else {
			uri = Util.asmaRoot;
			String query = Intent.ACTION_SEARCH.equals(intent.getAction()) ? intent.getStringExtra(SearchManager.QUERY) : null;
			setListAdapter(new FileInfoAdapter(this, R.layout.fileinfo_list_item, FileInfo.listIndex(this, query)));
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.archive_selector, menu);

		SearchManager searchManager = (SearchManager) getSystemService(SEARCH_SERVICE);
		SearchView searchView = (SearchView) menu.findItem(R.id.menu_search).getActionView();
		searchView.setSearchableInfo(searchManager.getSearchableInfo(getComponentName()));
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId()) {
		case R.id.menu_browse:
			startActivity(new Intent(Intent.ACTION_VIEW, null, this, FileSelector.class));
			return true;
		default:
			return false;
		}
	}
}
