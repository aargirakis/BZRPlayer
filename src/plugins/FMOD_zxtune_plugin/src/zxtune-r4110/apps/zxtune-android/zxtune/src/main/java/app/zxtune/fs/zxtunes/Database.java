/**
 * @file
 * @brief DAL helper
 * @author vitamin.caig@gmail.com
 */

package app.zxtune.fs.zxtunes;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.support.annotation.Nullable;

import java.io.IOException;
import java.nio.ByteBuffer;

import app.zxtune.Log;
import app.zxtune.TimeStamp;
import app.zxtune.fs.VfsCache;
import app.zxtune.fs.dbhelpers.DBProvider;
import app.zxtune.fs.dbhelpers.Grouping;
import app.zxtune.fs.dbhelpers.Objects;
import app.zxtune.fs.dbhelpers.Timestamps;
import app.zxtune.fs.dbhelpers.Transaction;
import app.zxtune.fs.dbhelpers.Utils;

/**
 * Version 1
 *
 * CREATE TABLE authors (_id INTEGER PRIMARY KEY, nickname TEXT NOT NULL, name TEXT)
 * CREATE TABLE tracks (_id INTEGER PRIMARY KEY, filename TEXT NOT NULL, title TEXT, duration
 * INTEGER, date INTEGER)
 * CREATE TABLE author_tracks (hash INTEGER UNIQUE, author INTEGER, track INTEGER)
 *
 * use hash as 100000 * author + track to support multiple insertings of same pair
 *
 * Version 2
 *
 * Use author_tracks standard grouping
 * Use timestamps
 */

final class Database {

  private static final String TAG = Database.class.getName();

  private static final String NAME = "www.zxtunes.com";
  private static final int VERSION = 2;

  static final class Tables {

    static final class Authors extends Objects {

      static enum Fields {
        _id, nickname, name
      }

      static final String NAME = "authors";

      static final String CREATE_QUERY = "CREATE TABLE " + NAME + " (" + Fields._id
              + " INTEGER PRIMARY KEY, " + Fields.nickname + " TEXT NOT NULL, " + Fields.name
              + " TEXT);";

      Authors(DBProvider helper) throws IOException {
        super(helper, NAME, Fields.values().length);
      }

      final void add(Author obj) {
        add(obj.id, obj.nickname, obj.name);
      }

      private static Author createAuthor(Cursor cursor) {
        final int id = cursor.getInt(Tables.Authors.Fields._id.ordinal());
        final String name = cursor.getString(Tables.Authors.Fields.name.ordinal());
        final String nickname = cursor.getString(Tables.Authors.Fields.nickname.ordinal());
        return new Author(id, nickname, name);
      }
    }

    static final class Tracks extends Objects {

      static enum Fields {
        _id, filename, title, duration, date
      }

      static final String NAME = "tracks";

      static final String CREATE_QUERY = "CREATE TABLE " + NAME + " (" + Fields._id
              + " INTEGER PRIMARY KEY, " + Fields.filename + " TEXT NOT NULL, " + Fields.title
              + " TEXT, " + Fields.duration + " INTEGER, " + Fields.date + " INTEGER);";

      Tracks(DBProvider helper) throws IOException {
        super(helper, NAME, Fields.values().length);
      }

      final void add(Track obj) {
        add(obj.id, obj.filename, obj.title, obj.duration, obj.date);
      }

      private static Track createTrack(Cursor cursor) {
        return createTrack(cursor, 0);
      }

      private static Track createTrack(Cursor cursor, int fieldOffset) {
        final int id = cursor.getInt(fieldOffset + Tables.Tracks.Fields._id.ordinal());
        final String filename = cursor.getString(fieldOffset + Tables.Tracks.Fields.filename.ordinal());
        final String title = cursor.getString(fieldOffset + Tables.Tracks.Fields.title.ordinal());
        final int duration = cursor.getInt(fieldOffset + Tables.Tracks.Fields.duration.ordinal());
        final int date = cursor.getInt(fieldOffset + Tables.Tracks.Fields.date.ordinal());
        return new Track(id, filename, title, duration, date);
      }

      static String getSelection(String subquery) {
        return Fields._id + " IN (" + subquery + ")";
      }
    }

    static final class AuthorsTracks extends Grouping {

      static final String NAME = "authors_tracks";
      static final String CREATE_QUERY = Grouping.createQuery(NAME);

      AuthorsTracks(DBProvider helper) throws IOException {
        super(helper, NAME, 32);
      }

      final void add(Author author, Track track) {
        add(author.id, track.id);
      }

      final String getTracksIdsSelection(Author author) {
        return getIdsSelection(author.id);
      }
    }
  }

  private final DBProvider helper;
  private final Tables.Authors authors;
  private final Tables.AuthorsTracks authorsTracks;
  private final Tables.Tracks tracks;
  private final Timestamps timestamps;
  private final String findQuery;
  private final VfsCache cacheDir;

  Database(Context context, VfsCache cache) throws IOException {
    this.helper = new DBProvider(Helper.create(context));
    this.authors = new Tables.Authors(helper);
    this.authorsTracks = new Tables.AuthorsTracks(helper);
    this.tracks = new Tables.Tracks(helper);
    this.timestamps = new Timestamps(helper);
    this.findQuery = "SELECT * " +
            "FROM authors LEFT OUTER JOIN tracks ON " +
            "tracks." + Tables.Tracks.getSelection(authorsTracks.getIdsSelection("authors._id")) +
            " WHERE tracks.filename || tracks.title LIKE '%' || ? || '%'";
    this.cacheDir = cache.createNested("www.zxtunes.com");
  }

  final Transaction startTransaction() throws IOException {
    return new Transaction(helper.getWritableDatabase());
  }

  final Timestamps.Lifetime getAuthorsLifetime(TimeStamp ttl) {
    return timestamps.getLifetime(Tables.Authors.NAME, ttl);
  }

  final Timestamps.Lifetime getAuthorTracksLifetime(Author author, TimeStamp ttl) {
    return timestamps.getLifetime(Tables.Authors.NAME + author.id, ttl);
  }

  final boolean queryAuthors(Catalog.AuthorsVisitor visitor) {
    final SQLiteDatabase db = helper.getReadableDatabase();
    final Cursor cursor = db.query(Tables.Authors.NAME, null, null, null, null, null, null);
    try {
      final int count = cursor.getCount();
      if (count != 0) {
        visitor.setCountHint(count);
        while (cursor.moveToNext()) {
          visitor.accept(Tables.Authors.createAuthor(cursor));
        }
        return true;
      }
    } finally {
      cursor.close();
    }
    return false;
  }

  final void addAuthor(Author obj) {
    authors.add(obj);
  }

  final boolean queryAuthorTracks(Author author, Catalog.TracksVisitor visitor) {
    final String selection = Tables.Tracks.getSelection(authorsTracks.getTracksIdsSelection(author));
    return queryTracks(selection, visitor);
  }

  private boolean queryTracks(String selection, Catalog.TracksVisitor visitor) {
    final SQLiteDatabase db = helper.getReadableDatabase();
    final Cursor cursor = db.query(Tables.Tracks.NAME, null, selection, null, null, null, null);
    try {
      final int count = cursor.getCount();
      if (count != 0) {
        visitor.setCountHint(count);
        while (cursor.moveToNext()) {
          visitor.accept(Tables.Tracks.createTrack(cursor));
        }
        return true;
      }
    } finally {
      cursor.close();
    }
    return false;
  }

  final synchronized void findTracks(String query, Catalog.FoundTracksVisitor visitor) {
    final SQLiteDatabase db = helper.getReadableDatabase();
    final Cursor cursor = db.rawQuery(findQuery, new String[]{query});
    try {
      final int count = cursor.getCount();
      if (count != 0) {
        visitor.setCountHint(count);
        while (cursor.moveToNext()) {
          final Author author = Tables.Authors.createAuthor(cursor);
          final Track track = Tables.Tracks.createTrack(cursor, Tables.Authors.Fields.values().length);
          visitor.accept(author, track);
        }
      }
    } finally {
      cursor.close();
    }
  }

  final void addTrack(Track obj) {
    tracks.add(obj);
  }

  final void addAuthorTrack(Author author, Track track) {
    authorsTracks.add(author, track);
  }

  @Nullable
  final ByteBuffer getTrackContent(int id) {
    final String filename = Integer.toString(id);
    return cacheDir.getCachedFileContent(filename);
  }

  final void addTrackContent(int id, ByteBuffer content) {
    final String filename = Integer.toString(id);
    cacheDir.putCachedFileContent(filename, content);
  }

  private static class Helper extends SQLiteOpenHelper {

    static Helper create(Context context) {
      return new Helper(context);
    }

    private Helper(Context context) {
      super(context, NAME, null, VERSION);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
      Log.d(TAG, "Creating database");
      db.execSQL(Tables.Authors.CREATE_QUERY);
      db.execSQL(Tables.Tracks.CREATE_QUERY);
      db.execSQL(Tables.AuthorsTracks.CREATE_QUERY);
      db.execSQL(Timestamps.CREATE_QUERY);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
      Log.d(TAG, "Upgrading database %d -> %d", oldVersion, newVersion);
      Utils.cleanupDb(db);
      onCreate(db);
    }
  }
}
