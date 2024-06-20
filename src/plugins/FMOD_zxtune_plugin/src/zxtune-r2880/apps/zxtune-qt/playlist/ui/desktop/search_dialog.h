/**
* 
* @file
*
* @brief Playlist search dialog interface
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "playlist/ui/search.h"
#include "playlist/supp/model.h"
#include "playlist/supp/operations_search.h"
//qt includes
#include <QtGui/QDialog>

namespace Playlist
{
  namespace UI
  {
    class SearchDialog : public QDialog
    {
      Q_OBJECT
    protected:
      explicit SearchDialog(QWidget& parent);
    public:
      typedef boost::shared_ptr<SearchDialog> Ptr;
      static Ptr Create(QWidget& parent);

      virtual bool Execute(Playlist::Item::Search::Data& res) = 0;
    };

    Playlist::Item::SelectionOperation::Ptr ExecuteSearchDialog(QWidget& parent, Playlist::Model::IndexSetPtr scope);
  }
}
