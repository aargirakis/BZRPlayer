/**
 *
 * @file
 *
 * @brief Global parameters implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "global_options.h"
#include "jni_options.h"
#include "properties.h"

namespace Parameters
{
  Container& GlobalOptions()
  {
    static const Parameters::Container::Ptr instance = Parameters::Container::Create();
    return *instance;
  }
}  // namespace Parameters

JNIEXPORT jlong JNICALL Java_app_zxtune_core_jni_JniOptions_getProperty__Ljava_lang_String_2J(JNIEnv* env,
                                                                                              jobject /*self*/,
                                                                                              jstring propName,
                                                                                              jlong defVal)
{
  const auto& params = Parameters::GlobalOptions();
  const Jni::PropertiesReadHelper props(env, params);
  return props.Get(propName, defVal);
}

JNIEXPORT jstring JNICALL Java_app_zxtune_core_jni_JniOptions_getProperty__Ljava_lang_String_2Ljava_lang_String_2(
    JNIEnv* env, jobject /*self*/, jstring propName, jstring defVal)
{
  const auto& params = Parameters::GlobalOptions();
  const Jni::PropertiesReadHelper props(env, params);
  return props.Get(propName, defVal);
}

JNIEXPORT void JNICALL Java_app_zxtune_core_jni_JniOptions_setProperty__Ljava_lang_String_2J(JNIEnv* env,
                                                                                             jobject /*self*/,
                                                                                             jstring propName,
                                                                                             jlong value)
{
  auto& params = Parameters::GlobalOptions();
  Jni::PropertiesWriteHelper props(env, params);
  props.Set(propName, value);
}

JNIEXPORT void JNICALL Java_app_zxtune_core_jni_JniOptions_setProperty__Ljava_lang_String_2Ljava_lang_String_2(
    JNIEnv* env, jobject /*self*/, jstring propName, jstring value)
{
  auto& params = Parameters::GlobalOptions();
  Jni::PropertiesWriteHelper props(env, params);
  props.Set(propName, value);
}
