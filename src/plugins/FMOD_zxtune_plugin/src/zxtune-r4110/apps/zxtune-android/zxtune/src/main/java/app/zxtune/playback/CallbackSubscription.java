/**
 *
 * @file
 *
 * @brief Callback subscription handler
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.playback;

import app.zxtune.Releaseable;

public class CallbackSubscription implements Releaseable {
  
  private PlaybackService service;
  private Callback callback;

  public CallbackSubscription(PlaybackService service, Callback callback) {
    this.service = service;
    this.callback = callback;
    service.subscribe(callback);
  }

  @Override
  public void release() {
    service.unsubscribe(callback);
    callback = null;
    service = null;
  }
}
