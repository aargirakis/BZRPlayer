#include "apps/zxtune-qt/update/rss.h"
#include "apps/zxtune-qt/update/downloads.h"
#include "apps/zxtune-qt/update/product.h"
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>
#include <stdexcept>
#include <iostream>

namespace
{
  QByteArray OpenFile(const std::string& name)
  {
    QFile file(QString::fromStdString(name));
    if (!file.open(QIODevice::ReadOnly))
    {
      throw std::runtime_error("Failed to open " + name);
    }
    return file.readAll();
  }

  void SaveFile(const std::string& name, const QByteArray& data)
  {
    QFile file(QString::fromStdString(name));
    if (!file.open(QIODevice::WriteOnly))
    {
      throw std::runtime_error("Failed to open " + name);
    }
    file.write(data);
  }
}

namespace
{
  class PrintRssVisitor : public RSS::Visitor
  {
  public:
    explicit PrintRssVisitor(QByteArray& dump)
      : Stream(&dump)
    {
    }

    virtual void OnEntry(const RSS::Entry& entry)
    {
      Stream << QString(
        "Entry %1\n"
        " Updated: %2\n"
        " DirectLink: %3\n"
        " AlternateLink: %4\n"
        " HtmlContent: %5\n"
        "\n")
      .arg(entry.Title)
      .arg(entry.Updated.toString(Qt::ISODate))
      .arg(entry.DirectLink.toString())
      .arg(entry.AlternateLink.toString())
      .arg(entry.HtmlContent);
    }
  private:
    QTextStream Stream;
  };

  class PrintDownloadsVisitor : public Downloads::Visitor
  {
  public:
    explicit PrintDownloadsVisitor(QByteArray& dump)
      : Stream(&dump)
    {
    }

    virtual void OnDownload(Product::Update::Ptr update)
    {
      Stream << QString(
        "Download %1\n"
        " Version: %2 %3\n"
        " Platform: %4 %5\n"
        " Description: %6\n"
        "\n")
      .arg(update->Package().toString())
      .arg(update->Version()).arg(update->Date().toString(Qt::ISODate))
      .arg(update->Platform()).arg(update->Architecture())
      .arg(update->Description().toString());
    }
  private:
    QTextStream Stream;
  };

  void TestFeed(const std::string& name)
  {
    std::cout << "Test for " << name << std::endl;
    const QByteArray feedContent = OpenFile(name + ".feed");
    {
      std::cout << " rss parse" << std::endl;
      const QByteArray parsedFeedRef = OpenFile(name + ".parsed");
      QByteArray parsedFeed;
      {
        PrintRssVisitor visitor(parsedFeed);
        if (!RSS::Parse(feedContent, visitor))
        {
          throw std::runtime_error("Failed to parse feed");
        }
      }
      if (parsedFeed != parsedFeedRef)
      {
        SaveFile(name + ".invalidparsed", parsedFeed);
        throw std::runtime_error("Invalid rss parse result for " + name);
      }
    }
    {
      std::cout << " downloads parse" << std::endl;
      const QByteArray parsedDownloadRef = OpenFile(name + ".download");
      QByteArray parsedDownload;
      {
        PrintDownloadsVisitor visitor(parsedDownload);
        std::unique_ptr<RSS::Visitor> rss = Downloads::CreateFeedVisitor("zxtune", visitor);
        if (!RSS::Parse(feedContent, *rss))
        {
          throw std::runtime_error("Failed to parse feed");
        }
      }
      if (parsedDownload != parsedDownloadRef)
      {
        SaveFile(name + ".invaliddownload", parsedDownload);
        throw std::runtime_error("Invalid download result for " + name);
      }
    }
  }
}

int main(int argc, char* argv[])
{
  try
  {
    QCoreApplication app(argc, argv);
    TestFeed("single_entry");
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return -1;
  }
}
