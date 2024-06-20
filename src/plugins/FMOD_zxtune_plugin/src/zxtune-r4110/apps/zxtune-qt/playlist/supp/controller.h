/**
* 
* @file
*
* @brief Playlist controller interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "data_provider.h"
#include "playlist/io/container.h"
#include "playlist/supp/model.h"
#include "playlist/supp/scanner.h"
//common includes
#include <iterator.h>
//qt includes
#include <QtCore/QObject>

namespace Playlist
{
  namespace Item
  {
    //dynamic part
    enum State
    {
      STOPPED,
      PAUSED,
      PLAYING,
      ERROR,
    };

    enum Playorder
    {
      LOOPED = 1,
      RANDOMIZED = 2
    };

    class StateCallback
    {
    public:
      virtual ~StateCallback() = default;

      virtual State GetState(const QModelIndex& index) const = 0;
    };

    class Iterator : public QObject
    {
      Q_OBJECT
    protected:
      explicit Iterator(QObject& parent);
    public:
      typedef Iterator* Ptr;

      //access
      virtual unsigned GetIndex() const = 0;
      virtual State GetState() const = 0;
      //change
      virtual void SetState(State state) = 0;
      //navigate
      virtual bool Next(unsigned playorderMode) = 0;
      virtual bool Prev(unsigned playorderMode) = 0;

      //select item without activation 
      virtual void Select(unsigned idx) = 0;
    public slots:
      //navigate
      virtual void Reset() = 0;
      virtual void Reset(unsigned idx) = 0;
    private slots:
      virtual void UpdateIndices(Playlist::Model::OldToNewIndexMap::Ptr remapping) = 0;
    signals:
      void Activated(Playlist::Item::Data::Ptr);
      void ItemActivated(Playlist::Item::Data::Ptr);
      void ItemActivated(unsigned idx);
      void Deactivated();
    };
  }

  class TextNotification
  {
  public:
    typedef std::shared_ptr<const TextNotification> Ptr;
    virtual ~TextNotification() = default;

    virtual QString Category() const = 0;
    virtual QString Text() const = 0;
    virtual QString Details() const = 0;
  };

  class Controller : public QObject
  {
    Q_OBJECT
  public:
    typedef std::shared_ptr<Controller> Ptr;
    typedef ObjectIterator<Ptr> Iterator;

    static Ptr Create(const QString& name, Item::DataProvider::Ptr provider);

    virtual QString GetName() const = 0;
    virtual void SetName(const QString& name) = 0;
    virtual Scanner::Ptr GetScanner() const = 0;
    virtual Model::Ptr GetModel() const = 0;
    virtual Item::Iterator::Ptr GetIterator() const = 0;
    virtual void Shutdown() = 0;
  public slots:
    virtual void ShowNotification(Playlist::TextNotification::Ptr notification) = 0;
  signals:
    void Renamed(const QString& name);
  };
}
