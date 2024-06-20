/**
 *
 * @file
 *
 * @brief Xspf file parser
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.playlist;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.ArrayList;

import org.xml.sax.SAXException;

import android.sax.Element;
import android.sax.EndElementListener;
import android.sax.EndTextElementListener;
import android.sax.RootElement;
import android.util.Xml;

public final class XspfIterator {

  public static ReferencesIterator create(ByteBuffer buf) throws IOException {
    return new ReferencesArrayIterator(parse(buf));
  }

  private static ArrayList<ReferencesIterator.Entry> parse(ByteBuffer buf) throws IOException {
    try {
      final ArrayList<ReferencesIterator.Entry> result = new ArrayList<ReferencesIterator.Entry>();
      final RootElement root = createPlaylistParseRoot(result);
      Xml.parse(newInputStream(buf), Xml.Encoding.UTF_8, root.getContentHandler());
      return result;
    } catch (SAXException e) {
      throw new IOException(e);
    }
  }
  
  private static RootElement createPlaylistParseRoot(final ArrayList<ReferencesIterator.Entry> entries) {
    final EntriesBuilder builder = new EntriesBuilder();
    final RootElement result = new RootElement(Xspf.XMLNS, Xspf.Tags.PLAYLIST);
    //TODO: check extension
    //final Element extension = result.getChild(Xspf.Tags.EXTENSION);
    final Element tracks = result.getChild(Xspf.XMLNS, Xspf.Tags.TRACKLIST);
    final Element track = tracks.getChild(Xspf.XMLNS, Xspf.Tags.TRACK);
    track.setEndElementListener(new EndElementListener() {
      @Override
      public void end() {
        final ReferencesIterator.Entry res = builder.captureResult();
        if (res != null) {
          entries.add(res);
        }
      }
    });
    track.getChild(Xspf.XMLNS, Xspf.Tags.LOCATION).setEndTextElementListener(new EndTextElementListener() {
      @Override
      public void end(String body) {
        builder.setLocation(body);
      }
    });
    //TODO: parse rest properties
    return result;
  }

  private static class EntriesBuilder {
    
    private ReferencesIterator.Entry result;
    
    EntriesBuilder() {
      this.result = new ReferencesIterator.Entry();
    }
    
    final void setLocation(String location) {
      result.location = location;
    }
    
    final ReferencesIterator.Entry captureResult() {
      final ReferencesIterator.Entry res = result;
      result = new ReferencesIterator.Entry();
      return res;
    }
  }
  
  static InputStream newInputStream(final ByteBuffer buf) {
    return new InputStream() {
      
      @Override
      public int available() throws IOException {
        return buf.remaining();
      }
      
      @Override
      public void mark(int readLimit) {
        buf.mark();
      }
      
      @Override
      public void reset() {
        buf.reset();
      }
      
      @Override
      public boolean markSupported() {
        return true;
      }
      
      @Override
      public int read() throws IOException {
        return buf.hasRemaining()
          ? buf.get()
          : -1;
      }
      
      @Override
      public int read(byte[] bytes, int off, int len) {
        if (buf.hasRemaining()) {
          len = Math.min(len, buf.remaining());
          buf.get(bytes, off, len);
          return len;
        } else {
          return -1;
        }
      }
    };
  }
}
