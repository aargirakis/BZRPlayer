/**
 *
 * @file
 *
 * @brief Seek control view logic
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.ui;

import java.util.concurrent.TimeUnit;

import android.os.Handler;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;
import app.zxtune.R;
import app.zxtune.TimeStamp;
import app.zxtune.playback.SeekControl;
import app.zxtune.playback.SeekControlStub;

class SeekControlView {
  
  private final TextView currentTime;
  private final TextView totalTime;
  private final SeekBar currentPosition;
  private final Handler timer;
  private final UpdateViewTask updateTask;
  private SeekControl control;

  SeekControlView(View layout) {
    this.currentTime = (TextView) layout.findViewById(R.id.position_time);
    this.totalTime = (TextView) layout.findViewById(R.id.position_duration);
    this.currentPosition = (SeekBar) layout.findViewById(R.id.position_seek);
    this.timer = new Handler();
    this.updateTask = new UpdateViewTask();
    this.control = SeekControlStub.instance(); 
    this.currentPosition.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
      @Override
      public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        if (fromUser) {
          final TimeStamp pos = TimeStamp.createFrom(progress, TimeUnit.SECONDS);
          control.setPosition(pos);
        }
      }
      
      @Override
      public void onStartTrackingTouch(SeekBar seekBar) {}
      
      @Override
      public void onStopTrackingTouch(SeekBar seekBar) {}
    });
  }

  final void setControl(SeekControl control) {
    this.control = control;
  }
  
  final void setEnabled(boolean enabled) {
    currentPosition.setEnabled(enabled);
    updateTask.stop();
    if (enabled) {
      final TimeStamp duration = control.getDuration();
      totalTime.setText(duration.toString());
      currentPosition.setProgress(0);
      currentPosition.setMax((int) duration.convertTo(TimeUnit.SECONDS));
      updateTask.run();
    }
  }
  
  private class UpdateViewTask implements Runnable {

    @Override
    public void run() {
      final TimeStamp pos = control.getPosition();
      currentPosition.setProgress((int) pos.convertTo(TimeUnit.SECONDS));
      currentTime.setText(pos.toString());
      
      timer.postDelayed(this, 1000);
    }
    
    final void stop() {
      timer.removeCallbacks(this);
    }
  }
}
