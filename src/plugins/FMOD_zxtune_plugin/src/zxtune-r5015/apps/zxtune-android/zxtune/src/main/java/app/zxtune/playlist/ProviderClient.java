package app.zxtune.playlist;

import android.content.ContentResolver;
import android.content.Context;
import android.database.ContentObserver;
import android.database.Cursor;
import android.net.Uri;

import androidx.annotation.Nullable;

import java.util.HashMap;

import javax.annotation.CheckForNull;

import app.zxtune.analytics.Analytics;

public final class ProviderClient {

  //should be name-compatible with Database
  public enum SortBy {
    title,
    author,
    duration
  }

  public enum SortOrder {
    asc,
    desc
  }

  public interface ChangesObserver {
    void onChange();
  }

  private final ContentResolver resolver;
  @Nullable
  private ContentObserver contentObserver;

  public ProviderClient(Context ctx) {
    this.resolver = ctx.getContentResolver();
  }

  public static Uri createUri(long id) {
    return PlaylistQuery.uriFor(id);
  }

  @Nullable
  public static Long findId(Uri uri) {
    return PlaylistQuery.isPlaylistUri(uri)
        ? PlaylistQuery.idOf(uri)
        : null;
  }

  public final void addItem(Item item) {
    resolver.insert(PlaylistQuery.ALL, item.toContentValues());
  }

  public final void notifyChanges() {
    resolver.notifyChange(PlaylistQuery.ALL, null);
  }

  public final void registerObserver(final ChangesObserver observer) {
    if (contentObserver != null) {
      throw new IllegalStateException();
    }
    contentObserver = new ContentObserver(null) {
      @Override
      public boolean deliverSelfNotifications() {
        return false;
      }

      @Override
      public void onChange(boolean selfChange) {
        observer.onChange();
      }
    };
    resolver.registerContentObserver(PlaylistQuery.ALL, true, contentObserver);
  }

  public final void unregisterObserver() {
    if (contentObserver != null) {
      resolver.unregisterContentObserver(contentObserver);
      contentObserver = null;
    }
  }

  @Nullable
  public final Cursor query(@Nullable long[] ids) {
    return resolver.query(PlaylistQuery.ALL, null,
        PlaylistQuery.selectionFor(ids), null, null);
  }

  public final void delete(long[] ids) {
    deleteItems(PlaylistQuery.selectionFor(ids));
    Analytics.sendPlaylistEvent(Analytics.PLAYLIST_ACTION_DELETE, ids.length);
  }

  public final void deleteAll() {
    deleteItems(null);
    Analytics.sendPlaylistEvent(Analytics.PLAYLIST_ACTION_DELETE, 0);
  }

  private void deleteItems(@Nullable String selection) {
    resolver.delete(PlaylistQuery.ALL, selection, null);
    notifyChanges();
  }

  public final void move(long id, int delta) {
    Provider.move(resolver, id, delta);
    notifyChanges();
    Analytics.sendPlaylistEvent(Analytics.PLAYLIST_ACTION_MOVE, 1);
  }

  public final void sort(SortBy by, SortOrder order) {
    Provider.sort(resolver, by.name(), order.name());
    notifyChanges();
    Analytics.sendPlaylistEvent(Analytics.PLAYLIST_ACTION_SORT,
        100 * by.ordinal() + order.ordinal());
  }

  @Nullable
  public final Statistics statistics(@Nullable long[] ids) {
    final Cursor cursor = resolver.query(PlaylistQuery.STATISTICS, null,
        PlaylistQuery.selectionFor(ids), null, null);
    if (cursor == null) {
      return null;
    }
    try {
      if (cursor.moveToFirst()) {
        return new Statistics(cursor);
      }
    } finally {
      cursor.close();
    }
    return null;
  }

  // id => path
  public final HashMap<String, String> getSavedPlaylists(@CheckForNull String id) {
    final HashMap<String, String> result = new HashMap<>();
    final Cursor cursor = resolver.query(PlaylistQuery.SAVED, null, id, null, null);
    if (cursor != null) {
      try {
        while (cursor.moveToNext()) {
          result.put(cursor.getString(0), cursor.getString(1));
        }
      } finally {
        cursor.close();
      }
    }
    return result;
  }

  public final void savePlaylist(String id, @CheckForNull long[] ids) throws Exception {
      Provider.save(resolver, id, ids);
      Analytics.sendPlaylistEvent(Analytics.PLAYLIST_ACTION_SAVE, ids != null ? ids.length : 0);
  }
}
