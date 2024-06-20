/**
* 
* @file
*
* @brief Scanner view interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "playlist/supp/scanner.h"
//qt includes
#include <QtGui/QWidget>

namespace Playlist
{
  namespace UI
  {
    class ScannerView : public QWidget
    {
      Q_OBJECT
    protected:
      explicit ScannerView(QWidget& parent);
    public:
      static ScannerView* Create(QWidget& parent, Playlist::Scanner::Ptr scanner);

    private slots:
      virtual void ScanStart(Playlist::ScanStatus::Ptr) = 0;
      virtual void ScanStop() = 0;
      virtual void ShowProgress(unsigned) = 0;
      virtual void ShowProgressMessage(const QString&) = 0;
    };
  }
}
