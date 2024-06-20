/**
* 
* @file
*
* @brief Scanner interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "data_provider.h"
//qt includes
#include <QtCore/QThread>

namespace Playlist
{
  class ScanStatus
  {
  public:
    typedef boost::shared_ptr<const ScanStatus> Ptr;
    virtual ~ScanStatus() {}

    virtual unsigned DoneFiles() const = 0;
    virtual unsigned FoundFiles() const = 0;
    virtual QString CurrentFile() const = 0;
    virtual bool SearchFinished() const = 0;
  };

  class Scanner : public QObject
  {
    Q_OBJECT
  protected:
    explicit Scanner(QObject& parent);
  public:
    typedef Scanner* Ptr;

    static Ptr Create(QObject& parent, Item::DataProvider::Ptr provider);

    virtual void AddItems(const QStringList& items) = 0;
    virtual void PasteItems(const QStringList& items) = 0;
  public slots:
    virtual void Pause(bool pause) = 0;
    virtual void Stop() = 0;
  signals:
    //for UI
    void ScanStarted(Playlist::ScanStatus::Ptr status);
    void ScanProgressChanged(unsigned progress);
    void ScanMessageChanged(const QString& message);
    void ScanStopped();
    //for BL
    void ItemFound(Playlist::Item::Data::Ptr item);
    void ItemsFound(Playlist::Item::Collection::Ptr items);
    void ErrorOccurred(const Error& e);
  };
}
