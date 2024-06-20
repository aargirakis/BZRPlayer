/**
 *
 * @file
 *
 * @brief Composite callback helper
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.playback;

import java.util.LinkedList;
import java.util.List;

public final class CompositeCallback implements Callback {
  
  private final List<Callback> delegates;
  private PlaybackControl.State lastState;
  private Item lastItem;
  private boolean lastIOStatus;
  
  public CompositeCallback() {
    this.delegates = new LinkedList<>();
    this.lastState = PlaybackControl.State.STOPPED;
  }
  
  @Override
  public void onStateChanged(PlaybackControl.State state) {
    synchronized (delegates) {
      lastState = state;
      for (Callback cb : delegates) {
        cb.onStateChanged(state);
      }
    }
  }

  @Override
  public void onItemChanged(Item item) {
    synchronized (delegates) {
      lastItem = item;
      for (Callback cb : delegates) {
        cb.onItemChanged(item);
      }
    }
  }

  @Override
  public void onIOStatusChanged(boolean isActive) {
    synchronized (delegates) {
      lastIOStatus = isActive;
      for (Callback cb : delegates) {
        cb.onIOStatusChanged(isActive);
      }
    }
  }
  
  @Override
  public void onError(String e) {
    synchronized (delegates) {
      for (Callback cb : delegates) {
        cb.onError(e);
      }
    }
  }
  
  public int add(Callback callback) {
    synchronized (delegates) {
      delegates.add(callback);
      if (lastItem != null) {
        callback.onItemChanged(lastItem);
      }
      callback.onStateChanged(lastState);
      callback.onIOStatusChanged(lastIOStatus);
      return delegates.size();
    }
  }

  public int remove(Callback callback) {
    synchronized (delegates) {
      delegates.remove(callback);
      return delegates.size();
    }
  }
}
