/**
 *
 * @file
 *
 * @brief Parcelable implementation of app.zxtune.playback.Item
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune.rpc;

import java.util.concurrent.TimeUnit;

import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import app.zxtune.TimeStamp;
import app.zxtune.playback.Item;

final class ParcelablePlaybackItem implements Item, Parcelable {

  public static final Parcelable.Creator<ParcelablePlaybackItem> CREATOR =
      new Parcelable.Creator<ParcelablePlaybackItem>() {
        @Override
        public ParcelablePlaybackItem createFromParcel(Parcel in) {
          return new ParcelablePlaybackItem(in);
        }

        @Override
        public ParcelablePlaybackItem[] newArray(int size) {
          return new ParcelablePlaybackItem[size];
        }
      };
      
  public static ParcelablePlaybackItem create(Item src) {
    return src != null ? new ParcelablePlaybackItem(src) : null;
  }

  private final Uri id;
  private final Uri dataId;
  private final String title;
  private final String author;
  private final String program;
  private final String comment;
  private final TimeStamp duration;

  private ParcelablePlaybackItem(Item src) {
    this.id = src.getId();
    this.dataId = src.getDataId();
    this.title = src.getTitle();
    this.author = src.getAuthor();
    this.program = src.getProgram();
    this.comment = src.getComment();
    this.duration = src.getDuration();
  }

  private ParcelablePlaybackItem(Parcel in) {
    id = in.readParcelable(Uri.class.getClassLoader());
    dataId = in.readParcelable(Uri.class.getClassLoader());
    title = in.readString();
    author = in.readString();
    program = in.readString();
    comment = in.readString();
    duration = TimeStamp.createFrom(in.readLong(), TimeUnit.MILLISECONDS);
  }

  @Override
  public Uri getId() {
    return id;
  }

  @Override
  public Uri getDataId() {
    return dataId;
  }

  @Override
  public String getTitle() {
    return title;
  }

  @Override
  public String getAuthor() {
    return author;
  }
  
  @Override
  public String getProgram() {
    return program;
  }
  
  @Override
  public String getComment() {
    return comment;
  }

  @Override
  public TimeStamp getDuration() {
    return duration;
  }

  @Override
  public int describeContents() {
    return 0;
  }

  @Override
  public void writeToParcel(Parcel dest, int flags) {
    dest.writeParcelable(id, 0);
    dest.writeParcelable(dataId, 0);
    dest.writeString(title);
    dest.writeString(author);
    dest.writeString(program);
    dest.writeString(comment);
    dest.writeLong(duration.convertTo(TimeUnit.MILLISECONDS));
  }
}
