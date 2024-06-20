/**
 * 
 * @file
 * 
 * @brief
 * 
 * @author vitamin.caig@gmail.com
 * 
 */
package app.zxtune.fs;

import java.io.IOException;
import java.nio.ByteBuffer;

import android.content.Context;
import android.net.Uri;
import app.zxtune.R;
import app.zxtune.playlist.XspfStorage;

final class VfsRootPlaylists implements VfsRoot {

  private final static String SCHEME = "playlists";

  private final Context context;

  public VfsRootPlaylists(Context context) {
    this.context = context;
  }

  @Override
  public Uri getUri() {
    return new Uri.Builder().scheme(SCHEME).build();
  }

  @Override
  public String getName() {
    return context.getString(R.string.vfs_playlists_root_name);
  }

  @Override
  public String getDescription() {
    return context.getString(R.string.vfs_playlists_root_description);
  }

  @Override
  public VfsDir getParent() {
    return null;
  }

  @Override
  public void enumerate(Visitor visitor) throws IOException {
    final XspfStorage storage = new XspfStorage(context);
    for (String name : storage.enumeratePlaylists()) {
      visitor.onFile(new PlaylistFile(storage.getPlaylistUri(name), name));
    }
  }

  @Override
  public void find(String mask, Visitor visitor) {}

  @Override
  public VfsObject resolve(Uri uri) throws IOException {
    return SCHEME.equals(uri.getScheme()) && uri.getPathSegments().isEmpty() ? this : null;
  }

  private static class PlaylistFile implements VfsFile {

    private final Uri uri;
    private final String name;

    PlaylistFile(Uri uri, String name) {
      this.uri = uri;
      this.name = name;
    }

    @Override
    public Uri getUri() {
      return uri;
    }

    @Override
    public String getName() {
      return name;
    }

    @Override
    public String getDescription() {
      return "".intern();
    }

    @Override
    public String getSize() {
      return "".intern();
    }

    @Override
    public ByteBuffer getContent() throws IOException {
      return null;
    }
  }
}
