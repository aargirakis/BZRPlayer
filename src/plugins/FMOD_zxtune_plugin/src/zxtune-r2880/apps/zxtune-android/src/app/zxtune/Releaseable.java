/**
 *
 * @file
 *
 * @brief Releaseable interface declaration
 *
 * @author vitamin.caig@gmail.com
 *
 */

package app.zxtune;

/**
 * Base interface for managed resources supporting release call
 * Analogue of Closeable but do not throw any specific exceptions
 */
public interface Releaseable {
  
  void release();
}
