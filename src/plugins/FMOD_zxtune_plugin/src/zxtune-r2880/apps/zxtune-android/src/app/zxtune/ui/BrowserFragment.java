/**
 *
 * @file
 *
 * @brief Vfs browser fragment component
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.ui;

import java.io.IOException;

import android.app.Activity;
import android.net.Uri;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.util.SparseBooleanArray;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListAdapter;
import app.zxtune.PlaybackServiceConnection;
import app.zxtune.R;
import app.zxtune.fs.Vfs;
import app.zxtune.fs.VfsDir;
import app.zxtune.fs.VfsFile;
import app.zxtune.fs.VfsObject;
import app.zxtune.fs.VfsRoot;
import app.zxtune.playback.PlaybackService;

public class BrowserFragment extends Fragment implements PlaybackServiceConnection.Callback {

  private static final String TAG = BrowserFragment.class.getName();
  private PlaybackService service;
  private VfsRoot root;
  private BrowserState state;
  private View sources;
  private BreadCrumbsView position;
  private BrowserView listing;

  public static BrowserFragment createInstance() {
    return new BrowserFragment();
  }

  @Override
  public void onAttach(Activity activity) {
    super.onAttach(activity);
    
    root = Vfs.createRoot(activity.getApplicationContext());
    state = new BrowserState(PreferenceManager.getDefaultSharedPreferences(activity));
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
    return container != null ? inflater.inflate(R.layout.browser, container, false) : null;
  }

  @Override
  public synchronized void onViewCreated(View view, Bundle savedInstanceState) {
    super.onViewCreated(view, savedInstanceState);
    
    sources = view.findViewById(R.id.browser_sources);
    final View roots = view.findViewById(R.id.browser_roots);
    roots.setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View v) {
        setCurrentDir(root);
      }
    });
    position = (BreadCrumbsView) view.findViewById(R.id.browser_breadcrumb);
    position.setDirSelectionListener(new BreadCrumbsView.DirSelectionListener() {
      @Override
      public void onDirSelection(VfsDir dir) {
        setCurrentDir(dir);
      }
    });
    listing = (BrowserView) view.findViewById(R.id.browser_content);
    listing.setOnItemClickListener(new OnItemClickListener());
    listing.setEmptyView(view.findViewById(R.id.browser_stub));
    listing.setMultiChoiceModeListener(new MultiChoiceModeListener());

    final Uri currentPath = state.getCurrentPath(); 
    if (savedInstanceState == null) {
      Log.d(TAG, "Load persistent state");
      loadBrowser(currentPath);
    } else {
      loadNavigation(currentPath);
      loadListing();
    }
  }

  @Override
  public synchronized void onDestroyView() {
    super.onDestroyView();

    Log.d(TAG, "Saving persistent state");
    storeCurrentViewPosition();
    service = null;
  }
  
  public final void moveUp() {
    try {
      final VfsDir curDir = (VfsDir) root.resolve(state.getCurrentPath());
      if (curDir != root) {
        final VfsDir parent = curDir.getParent();
        setCurrentDir(parent != null ? parent : root);
      }
    } catch (IOException e) {
      listing.showError(e);
    }
  }

  @Override
  public void onServiceConnected(PlaybackService service) {
    this.service = service;
  }
  
  private String getActionModeTitle() {
    final int count = listing.getCheckedItemsCount();
    return getResources().getQuantityString(R.plurals.items, count, count);
  }

  class OnItemClickListener implements AdapterView.OnItemClickListener {

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
      final VfsObject obj = (VfsObject) parent.getItemAtPosition(position);
      if (obj instanceof VfsFile) {
        onClick((VfsFile) obj);
      } else if (obj instanceof VfsDir) {
        onClick((VfsDir) obj);
      }
    }

    private void onClick(VfsFile file) {
      final Uri[] toPlay = {file.getUri()};
      service.setNowPlaying(toPlay);
    }

    private void onClick(VfsDir dir) {
      setCurrentDir(dir);
    }
  }

  private class MultiChoiceModeListener implements ListViewCompat.MultiChoiceModeListener {

    @Override
    public boolean onCreateActionMode(ListViewCompat.ActionMode mode, Menu menu) {
      setEnabledRecursive(sources, false);
      final MenuInflater inflater = mode.getMenuInflater();
      inflater.inflate(R.menu.selection, menu);
      inflater.inflate(R.menu.browser, menu);
      mode.setTitle(getActionModeTitle());
      return true;
    }

    @Override
    public boolean onPrepareActionMode(ListViewCompat.ActionMode mode, Menu menu) {
      return false;
    }

    @Override
    public boolean onActionItemClicked(ListViewCompat.ActionMode mode, MenuItem item) {
      if (listing.processActionItemClick(item.getItemId())) {
        return true;
      } else {
        switch (item.getItemId()) {
          case R.id.action_play:
            service.setNowPlaying(getSelectedItemsUris());
            break;
          case R.id.action_add:
            service.getPlaylistControl().add(getSelectedItemsUris());
            break;
          default:
            return false;
        }
        mode.finish();
        return true;
      }
    }

    @Override
    public void onDestroyActionMode(ListViewCompat.ActionMode mode) {
      setEnabledRecursive(sources, true);
    }

    @Override
    public void onItemCheckedStateChanged(ListViewCompat.ActionMode mode, int position, long id,
        boolean checked) {
      mode.setTitle(getActionModeTitle());
    }
    
    private Uri[] getSelectedItemsUris() {
      final Uri[] result = new Uri[listing.getCheckedItemsCount()];
      final SparseBooleanArray selected = listing.getCheckedItemPositions();
      final ListAdapter adapter = listing.getAdapter();
      int pos = 0;
      for (int i = 0, lim = selected.size(); i != lim; ++i) {
        if (selected.valueAt(i)) {
          final VfsObject obj = (VfsObject) adapter.getItem(selected.keyAt(i));
          result[pos++] = obj.getUri();
        }
      }
      return result;
    }
  }
  
  private static void setEnabledRecursive(View view, boolean enabled) {
    if (view instanceof ViewGroup) {
      final ViewGroup group = (ViewGroup) view;
      for (int idx = 0, lim = group.getChildCount(); idx != lim; ++idx) {
        setEnabledRecursive(group.getChildAt(idx), enabled);
      }
    } else {
      view.setEnabled(enabled);
    }
  }

  private void setCurrentDir(VfsDir dir) {
    storeCurrentViewPosition();
    setNewState(dir.getUri());
    loadBrowser(dir);
  }

  private void storeCurrentViewPosition() {
    state.setCurrentViewPosition(listing.getFirstVisiblePosition());
  }

  private void setNewState(Uri uri) {
    Log.d(TAG, "Set current path to " + uri);
    state.setCurrentPath(uri);
  }
  
  private void loadBrowser(Uri path) {
    try {
      final VfsDir dir = (VfsDir) root.resolve(path);
      loadBrowser(dir);
    } catch (IOException e) {
      listing.showError(e);
    }
  }
  
  private void loadBrowser(VfsDir dir) {
    loadNavigation(dir);
    loadListing(dir);
  }
  
  private void loadNavigation(Uri path) {
    try {
      final VfsDir dir = (VfsDir) root.resolve(path);
      loadNavigation(dir);
    } catch (IOException e) {
      listing.showError(e);
    }
  }

  private void loadListing() {
    listing.load(getLoaderManager());
  }
  
  private void loadNavigation(VfsDir dir) {
    if (dir == root) {
      position.setDir(null);
    } else {
      position.setDir(dir);
    }
  }

  private void loadListing(VfsDir dir) {
    listing.load(getLoaderManager(), dir, state.getCurrentViewPosition());
  }
}
