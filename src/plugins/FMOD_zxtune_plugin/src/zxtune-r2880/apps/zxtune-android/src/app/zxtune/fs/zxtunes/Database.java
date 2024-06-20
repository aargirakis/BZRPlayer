/**
 *
 * @file
 *
 * @brief DAL helper
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.fs.zxtunes;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.database.sqlite.SQLiteQueryBuilder;
import android.util.Log;

/**
 * Version 1
 * 
 * CREATE TABLE authors (_id INTEGER PRIMARY KEY, nickname TEXT NOT NULL, name TEXT)
 * CREATE TABLE tracks (_id INTEGER PRIMARY KEY, filename TEXT NOT NULL, title TEXT, duration
 * INTEGER, date INTEGER)
 * CREATE TABLE owning (hash INTEGER UNIQUE, author INTEGER, track INTEGER)
 * 
 * use hash as 100000 * author + track to support multiple insertings of same pair
 */

final class Database {

  final static String TAG = Database.class.getName();

  final static String NAME = "www.zxtunes.com";
  final static int VERSION = 1;

  final static class Tables {

    final static String DROP_QUERY = "DROP TABLE ?;";

    final static class Authors {

      static enum Fields {
        _id, nickname, name
      }

      final static String NAME = "authors";

      final static String CREATE_QUERY = "CREATE TABLE " + NAME + " (" + Fields._id
          + " INTEGER PRIMARY KEY, " + Fields.nickname + " TEXT NOT NULL, " + Fields.name
          + " TEXT);";
    }

    final static class Tracks {

      static enum Fields {
        _id, filename, title, duration, date
      }

      final static String NAME = "tracks";

      final static String CREATE_QUERY = "CREATE TABLE " + NAME + " (" + Fields._id
          + " INTEGER PRIMARY KEY, " + Fields.filename + " TEXT NOT NULL, " + Fields.title
          + " TEXT, " + Fields.duration + " INTEGER, " + Fields.date + " INTEGER);";
    }

    final static class AuthorsTracks {

      static enum Fields {
        hash, author, track
      }

      final static String NAME = "authors_tracks";

      final static String CREATE_QUERY = "CREATE TABLE " + NAME + " (" + Fields.hash
          + " INTEGER UNIQUE, " + Fields.author + " INTEGER, " + Fields.track + " INTEGER);";
    }
  }

  private final Helper helper;

  Database(Context context) {
    this.helper = Helper.create(context);
  }

  class Transaction {

    private final SQLiteDatabase db;

    Transaction(SQLiteDatabase db) {
      this.db = db;
      db.beginTransaction();
    }

    final void succeed() {
      db.setTransactionSuccessful();
    }

    final void finish() {
      db.endTransaction();
    }
  }

  final Transaction startTransaction() {
    return new Transaction(helper.getWritableDatabase());
  }

  final void queryAuthors(Catalog.AuthorsVisitor visitor, Integer id) {
    Log.d(TAG, "queryAuthors(" + id + ")");
    final SQLiteDatabase db = helper.getReadableDatabase();
    final String selection = id != null ? Tables.Authors.Fields._id + " = " + id : null;
    final Cursor cursor = db.query(Tables.Authors.NAME, null, selection, null, null, null, null);
    try {
      while (cursor.moveToNext()) {
        visitor.accept(createAuthor(cursor));
      }
    } finally {
      cursor.close();
    }
  }

  final void addAuthor(Author obj) {
    final SQLiteDatabase db = helper.getWritableDatabase();
    db.insertWithOnConflict(Tables.Authors.NAME, null/* nullColumnHack */, createValues(obj),
        SQLiteDatabase.CONFLICT_REPLACE);
  }

  final void queryTracks(Catalog.TracksVisitor visitor, Integer id, Integer author) {
    final SQLiteDatabase db = helper.getReadableDatabase();
    final String selection =
        id != null ? createSingleTrackSelection(id) : createAuthorTracksSelection(author);
    final Cursor cursor = db.query(Tables.Tracks.NAME, null, selection, null, null, null, null);
    try {
      while (cursor.moveToNext()) {
        visitor.accept(createTrack(cursor));
      }
    } finally {
      cursor.close();
    }
  }

  private static String createSingleTrackSelection(int id) {
    return Tables.Tracks.Fields._id + " = " + id;
  }

  private static String createAuthorTracksSelection(int author) {
    final String idQuery =
        SQLiteQueryBuilder.buildQueryString(true, Tables.AuthorsTracks.NAME,
            new String[] {Tables.AuthorsTracks.Fields.track.name()}, Tables.AuthorsTracks.Fields.author + " = "
                + author, null, null, null, null);
    return Tables.Tracks.Fields._id + " IN (" + idQuery + ")";
  }

  final void addTrack(Track obj, Integer author) {
    final SQLiteDatabase db = helper.getWritableDatabase();
    db.insertWithOnConflict(Tables.Tracks.NAME, null/* nullColumnHack */, createValues(obj),
        SQLiteDatabase.CONFLICT_REPLACE);
    if (author != null) {
      db.insert(Tables.AuthorsTracks.NAME, null/* nullColumnHack */, createValues(author, obj.id));
    }
  }

  private static Author createAuthor(Cursor cursor) {
    final int id = cursor.getInt(Tables.Authors.Fields._id.ordinal());
    final String name = cursor.getString(Tables.Authors.Fields.name.ordinal());
    final String nickname = cursor.getString(Tables.Authors.Fields.nickname.ordinal());
    return new Author(id, nickname, name);
  }

  private static ContentValues createValues(Author obj) {
    final ContentValues res = new ContentValues();
    res.put(Tables.Authors.Fields._id.name(), obj.id);
    res.put(Tables.Authors.Fields.name.name(), obj.name);
    res.put(Tables.Authors.Fields.nickname.name(), obj.nickname);
    return res;
  }

  private static Track createTrack(Cursor cursor) {
    final int id = cursor.getInt(Tables.Tracks.Fields._id.ordinal());
    final String filename = cursor.getString(Tables.Tracks.Fields.filename.ordinal());
    final String title = cursor.getString(Tables.Tracks.Fields.title.ordinal());
    final int duration = cursor.getInt(Tables.Tracks.Fields.duration.ordinal());
    final int date = cursor.getInt(Tables.Tracks.Fields.date.ordinal());
    return new Track(id, filename, title, duration, date);
  }

  private static ContentValues createValues(Track obj) {
    final ContentValues res = new ContentValues();
    res.put(Tables.Tracks.Fields._id.name(), obj.id);
    res.put(Tables.Tracks.Fields.filename.name(), obj.filename);
    res.put(Tables.Tracks.Fields.title.name(), obj.title);
    res.put(Tables.Tracks.Fields.duration.name(), obj.duration);
    res.put(Tables.Tracks.Fields.date.name(), obj.date);
    return res;
  }

  private static ContentValues createValues(int author, int track) {
    final int hash = 100000 * author + track;
    final ContentValues res = new ContentValues();
    res.put(Tables.AuthorsTracks.Fields.hash.name(), hash);
    res.put(Tables.AuthorsTracks.Fields.author.name(), author);
    res.put(Tables.AuthorsTracks.Fields.track.name(), track);
    return res;
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
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
      Log.d(TAG, String.format("Upgrading database %d -> %d", oldVersion, newVersion));
      db.execSQL(Tables.DROP_QUERY, new Object[] {Tables.Authors.NAME});
      db.execSQL(Tables.DROP_QUERY, new Object[] {Tables.Tracks.NAME});
      db.execSQL(Tables.DROP_QUERY, new Object[] {Tables.AuthorsTracks.NAME});
      onCreate(db);
    }
  }
}
