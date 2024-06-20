/**
* 
* @file
*
* @brief Plugins settings pane interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//qt includes
#include <QtGui/QWidget>

namespace UI
{
  class PluginsSettingsWidget : public QWidget
  {
    Q_OBJECT
  protected:
    explicit PluginsSettingsWidget(QWidget& parent);
  public:
    static PluginsSettingsWidget* Create(QWidget& parent);
  };
}
