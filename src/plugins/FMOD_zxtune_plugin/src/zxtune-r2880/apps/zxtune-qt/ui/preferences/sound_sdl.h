/**
* 
* @file
*
* @brief SDL settings pane interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "../conversion/backend_settings.h"

namespace UI
{
  class SdlSettingsWidget : public BackendSettingsWidget
  {
    Q_OBJECT
  protected:
    explicit SdlSettingsWidget(QWidget& parent);
  public:
    static BackendSettingsWidget* Create(QWidget& parent);
  };
}
