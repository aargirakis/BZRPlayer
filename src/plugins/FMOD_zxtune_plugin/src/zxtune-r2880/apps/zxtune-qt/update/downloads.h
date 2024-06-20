/**
* 
* @file
*
* @brief Downloads visitor adapter factory
*
* @author vitamin.caig@gmail.com
*
**/

#pragma once

//local includes
#include "product.h"
#include "rss.h"
//std includes
#include <memory>

namespace Downloads
{
  class Visitor
  {
  public:
    virtual ~Visitor() {}

    virtual void OnDownload(Product::Update::Ptr update) = 0;
  };

  std::auto_ptr<RSS::Visitor> CreateFeedVisitor(const QString& project, Visitor& delegate);
}
