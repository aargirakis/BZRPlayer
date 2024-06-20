/**
 *
 * @file
 *
 * @brief Asynchronous implementation of Player
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.sound;

import android.os.Process;
import android.util.Log;

/**
 * Asynchronous player state machine
 * Synchronized methods protect this from parallel modifications.
 * State's monitor used to synchronize state transitions
 */
final public class AsyncPlayer implements Player {

  private static final String TAG = AsyncPlayer.class.getName();

  private Player sync;
  private final PlayerEventsListener events;
  private final Object stateGuard;
  private Player state;
  private Thread playThread;

  private AsyncPlayer(SamplesSource source, SamplesTarget target, PlayerEventsListener events) {
    this.sync = new SyncPlayer(source, target, new EventsSynchronized());
    this.events = events;
    this.stateGuard = new Object();
    this.state = new StoppedPlayer();
  }

  public static Player create(SamplesSource source, PlayerEventsListener events) {
    final SamplesTarget target = SoundOutputSamplesTarget.create();
    return new AsyncPlayer(source, target, events);
  }

  @Override
  public void startPlayback() {
    Log.d(TAG, "Play");
    synchronized (stateGuard) {
      state.startPlayback();
    }
  }

  @Override
  public void stopPlayback() {
    Log.d(TAG, "Stop");
    synchronized (stateGuard) {
      state.stopPlayback();
    }
  }
  
  @Override
  public boolean isStarted() {
    synchronized (stateGuard) {
      return state.isStarted();
    }
  }
  
  @Override
  public synchronized void release() {
    sync.release();
    sync = null;
    state = null;
  }
  
  private void setState(Player newState) {
    synchronized (stateGuard) {
      state = newState;
      stateGuard.notify();
    }
  }
  
  private void waitForStateChange() {
    try {
      synchronized (stateGuard) {
        stateGuard.wait();
      }
    } catch (InterruptedException e) {
      Log.d(TAG, "Interrupted while stop: ");
      e.printStackTrace();
    }
  }

  private final class EventsSynchronized implements PlayerEventsListener {

    @Override
    public void onStart() {
      setState(new StartedPlayer());
      events.onStart();
    }

    @Override
    public void onFinish() {
      events.onFinish();
    }

    @Override
    public void onStop() {
      setState(new StoppedPlayer());
      events.onStop();
    }

    @Override
    public void onError(Error e) {
      events.onError(e);
    }
  }
  
  private final class StoppedPlayer extends StubPlayer {
    
    StoppedPlayer() {
      Log.d(TAG, "Stopped");
    }

    @Override
    public void startPlayback() {
      playThread = new PlayThread();
      playThread.start();
      waitForStateChange();
    }
  }

  private final class StartedPlayer extends StubPlayer {
    
    StartedPlayer() {
      Log.d(TAG, "Started");
    }
    
    @Override
    public void stopPlayback() {
      try {
        sync.stopPlayback();
        waitForStateChange();
        playThread.join();
        playThread = null;
      } catch (InterruptedException e) {
        Log.d(TAG, "Interrupted while stop: ");
        e.printStackTrace();
      }
    }

    @Override
    public boolean isStarted() {
      return true;
    }
  }

  private class PlayThread extends Thread {
    @Override
    public void run() {
      Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);
      sync.startPlayback();
    }
  }
}
