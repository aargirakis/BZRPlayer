package app.zxtune.sound;

import androidx.annotation.Nullable;

import java.util.concurrent.Exchanger;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

import app.zxtune.Log;

class AsyncSamplesTarget {

  private static final String TAG = AsyncSamplesTarget.class.getName();

  private final SamplesTarget target;
  private final Thread thread;
  private final Exchanger<short[]> exchanger;
  @Nullable
  private short[] inputBuffer;
  @Nullable
  private short[] outputBuffer;

  AsyncSamplesTarget(SamplesTarget target) {
    this.target = target;
    this.exchanger = new Exchanger<>();
    this.thread = new Thread("ConsumeSoundThread") {
      @Override
      public void run() {
        consumeCycle();
      }
    };
    final int bufferSize = target.getPreferableBufferSize();
    this.inputBuffer = new short[bufferSize];
    this.outputBuffer = new short[bufferSize];
    thread.start();
  }

  final int getSampleRate() {
    return target.getSampleRate();
  }

  final void release() {
    while (true) {
      thread.interrupt();
      try {
        thread.join();
        break;
      } catch (InterruptedException e) {
        Log.w(TAG, e, "Failed to release");
      }
    }
    target.release();
  }

  final void start() throws Exception {
    target.start();
  }

  final void stop() throws Exception {
    target.stop();
  }

  final short[] getBuffer() throws Exception {
    final short[] result = inputBuffer;
    if (result != null) {
      return result;
    } else {
      throw new Exception("Output is in error state");
    }
  }

  final boolean commitBuffer() throws Exception {
    try {
      inputBuffer = exchanger.exchange(getBuffer(), 1, TimeUnit.SECONDS);
      return true;
    } catch (TimeoutException e) {
      return false;
    }
  }

  private void consumeCycle() {
    try {
      while (true) {
        outputBuffer = exchanger.exchange(outputBuffer);//interruption point
        target.writeSamples(outputBuffer);
      }
    } catch (InterruptedException e) {
    } catch (Exception e) {
      Log.w(TAG, e, "Error in consume cycle");
    } finally {
      inputBuffer = null;
      outputBuffer = null;
    }
  }
}
