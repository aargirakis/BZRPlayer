/**
 * 
 * @file
 *
 * @brief
 *
 * @author vitamin.caig@gmail.com
 * 
 */

package app.zxtune.sound;

import java.io.FileNotFoundException;
import java.io.RandomAccessFile;

public final class WaveWriteSamplesTarget implements SamplesTarget {
  
  private static final int SAMPLERATE = 44100;
  private static final int CHANNELS = 1;
  private static final int BYTES_PER_SAMPLE = 2;
  
  private static final byte header[] = {
      //+0
      'R', 'I', 'F', 'F',
      //+4 - file size - 8
      0, 0, 0, 0,
      //+8
      'W', 'A', 'V', 'E',
      //+12 - chunkId
      'f', 'm', 't', ' ',
      //+16 - chunkSize=16
      16, 0, 0, 0,
      //+20 - compression
      1, 0,
      //+22 - channels
      CHANNELS, 0,
      //+24 - samplerate
      0, 0, 0, 0,
      //+28 - bytes per sec
      0, 0, 0, 0,
      //+32 - align
      BYTES_PER_SAMPLE, 0,
      //+34 - bits per sample
      8 * BYTES_PER_SAMPLE, 0,
      //+36
      'd', 'a', 't', 'a',
      //+40 - data size
      0, 0, 0, 0
  };
  private static final int OFFSET_WAVESIZE = 4;
  private static final int OFFSET_SAMPLERATE = 24;
  private static final int OFFSET_BPS = 28;
  private static final int OFFSET_DATA_SIZE = 40;

  private byte buffer[];
  private final RandomAccessFile file;
  private int doneBytes;

  public WaveWriteSamplesTarget(String filename) throws Exception {
    this.buffer = new byte[CHANNELS * BYTES_PER_SAMPLE * SAMPLERATE];
    try {
      this.file = new RandomAccessFile(filename, "rw");
    } catch (FileNotFoundException e) {
      throw new Exception(e);
    }
  }

  @Override
  public int getSampleRate() {
    return SAMPLERATE;
  }

  @Override
  public int getPreferableBufferSize() {
    return SAMPLERATE;//1s buffer
  }

  @Override
  public void start() throws Exception {
    file.seek(header.length);
    doneBytes = 0;
  }

  @Override
  public void pause() {}

  @Override
  public void writeSamples(short[] input) throws Exception {
    final int inSamples = input.length / SamplesSource.Channels.COUNT;
    final int outBytes = inSamples * SamplesSource.Sample.BYTES; 
    allocateBuffer(outBytes);
    convertBuffer(input, outBytes);
    file.write(buffer, 0, outBytes);
    doneBytes += outBytes;
  }

  @Override
  public void stop() throws Exception {
    setHeaderLE32(OFFSET_SAMPLERATE, SAMPLERATE);
    setHeaderLE32(OFFSET_BPS, SAMPLERATE * BYTES_PER_SAMPLE);
    setHeaderLE32(OFFSET_DATA_SIZE, doneBytes);
    setHeaderLE32(OFFSET_WAVESIZE, header.length - 8 + doneBytes);
    file.seek(0);
    file.write(header);
    file.seek(header.length + doneBytes);
  }

  @Override
  public void release() {
  }
  
  private void allocateBuffer(int size) {
    if (buffer.length < size) {
      buffer = new byte[size];
    }
  }

  private void convertBuffer(short[] input, int outBytes) {
    for (int in = 0, out = 0; out < outBytes; )
    {
      int sample = 0;
      for (int ch = 0; ch != SamplesSource.Channels.COUNT; ++ch) {
        sample += input[in++];
      }
      sample /= SamplesSource.Channels.COUNT;
      buffer[out++] = (byte) (sample & 0xff);
      buffer[out++] = (byte) ((sample >> 8) & 0xff);
    }
  }
  
  private void setHeaderLE32(int offset, int value) {
    for (int b = 0; b != 4; ++b) {
      header[offset++] = (byte) (value & 0xff);
      value >>= 8;
    }
  }
}
