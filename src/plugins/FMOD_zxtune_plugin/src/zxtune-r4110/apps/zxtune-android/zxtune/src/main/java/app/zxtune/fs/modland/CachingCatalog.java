/**
 * @file
 * @brief Caching catalog implementation
 * @author vitamin.caig@gmail.com
 */

package app.zxtune.fs.modland;

import android.support.annotation.Nullable;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.concurrent.TimeUnit;

import app.zxtune.TimeStamp;
import app.zxtune.fs.dbhelpers.CommandExecutor;
import app.zxtune.fs.dbhelpers.FetchCommand;
import app.zxtune.fs.dbhelpers.QueryCommand;
import app.zxtune.fs.dbhelpers.Timestamps;
import app.zxtune.fs.dbhelpers.Transaction;

final class CachingCatalog extends Catalog {

  private final TimeStamp GROUPS_TTL = days(30);
  private final TimeStamp GROUP_TRACKS_TTL = days(14);

  private static TimeStamp days(int val) {
    return TimeStamp.createFrom(val, TimeUnit.DAYS);
  }

  private final Catalog remote;
  private final Database db;
  private final Grouping authors;
  private final Grouping collections;
  private final Grouping formats;
  private final CommandExecutor executor;

  public CachingCatalog(Catalog remote, Database db) {
    this.remote = remote;
    this.db = db;
    this.authors = new CachedGrouping(Database.Tables.Authors.NAME, remote.getAuthors());
    this.collections = new CachedGrouping(Database.Tables.Collections.NAME, remote.getCollections());
    this.formats = new CachedGrouping(Database.Tables.Formats.NAME, remote.getFormats());
    this.executor = new CommandExecutor("modland");
  }

  @Override
  public Grouping getAuthors() {
    return authors;
  }

  @Override
  public Grouping getCollections() {
    return collections;
  }

  @Override
  public Grouping getFormats() {
    return formats;
  }

  private class CachedGrouping implements Grouping {

    private final String category;
    private final Grouping remote;

    CachedGrouping(String category, Grouping remote) {
      this.category = category;
      this.remote = remote;
    }

    @Override
    public void query(final String filter, final GroupsVisitor visitor) throws IOException {
      executor.executeQuery(category, new QueryCommand() {
        @Override
        public Timestamps.Lifetime getLifetime() {
          return db.getGroupsLifetime(category, filter, GROUPS_TTL);
        }

        @Override
        public Transaction startTransaction() throws IOException {
          return db.startTransaction();
        }

        @Override
        public void updateCache() throws IOException {
          remote.query(filter, new GroupsVisitor() {
            @Override
            public void accept(Group obj) {
              db.addGroup(category, obj);
            }
          });
        }

        @Override
        public boolean queryFromCache() {
          return db.queryGroups(category, filter, visitor);
        }
      });
    }

    @Override
    @Nullable
    public Group query(final int id) throws IOException {
      final String categoryElement = category.substring(0, category.length() - 1);
      return executor.executeFetchCommand(categoryElement, new FetchCommand<Group>() {
        @Override
        public Group fetchFromCache() {
          return db.queryGroup(category, id);
        }

        @Override
        @Nullable
        public Group fetchFromRemote() throws IOException {
          final Group res = remote.query(id);
          if (res != null) {
            db.addGroup(category, res);
          }
          return res;
        }
      });
    }

    @Override
    public void queryTracks(final int id, final TracksVisitor visitor) throws IOException {
      executor.executeQuery("tracks", new QueryCommand() {
        @Override
        public Timestamps.Lifetime getLifetime() {
          return db.getGroupTracksLifetime(category, id, GROUP_TRACKS_TTL);
        }

        @Override
        public Transaction startTransaction() throws IOException {
          return db.startTransaction();
        }

        @Override
        public void updateCache() throws IOException {
          remote.queryTracks(id, new TracksVisitor() {
            @Override
            public boolean accept(Track obj) {
              db.addTrack(obj);
              db.addGroupTrack(category, id, obj);
              return true;
            }
          });
        }

        @Override
        public boolean queryFromCache() {
          return db.queryTracks(category, id, visitor);
        }
      });
    }

    @Override
    public Track findTrack(final int id, final String filename) throws IOException {
      return executor.executeFetchCommand("track", new FetchCommand<Track>() {
        @Override
        public Track fetchFromCache() {
          return db.findTrack(category, id, filename);
        }

        @Override
        @Nullable
        public Track fetchFromRemote() throws IOException {
          //fill cache
          queryTracks(id, new TracksVisitor() {

            @Override
            public boolean accept(Track obj) {
              return true;
            }
          });
          return db.findTrack(category, id, filename);
        }
      });
    }
  }

  @Override
  public ByteBuffer getTrackContent(final String id) throws IOException {
    return executor.executeFetchCommand("file", new FetchCommand<ByteBuffer>() {
      @Override
      public ByteBuffer fetchFromCache() {
        return db.getTrackContent(id);
      }

      @Override
      public ByteBuffer fetchFromRemote() throws IOException {
        final ByteBuffer res = remote.getTrackContent(id);
        db.addTrackContent(id, res);
        return res;
      }
    });
  }
}
