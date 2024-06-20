/**
* 
* @file
*
* @brief Implementation of .xspf exporting
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "export.h"
#include "tags/xspf.h"
#include "ui/utils.h"
//common includes
#include <error_tools.h>
//library includes
#include <zxtune.h>
#include <core/module_attrs.h>
#include <debug/log.h>
#include <sound/sound_parameters.h>
#include <parameters/convert.h>
#include <core/plugins/containers/zdata_supp.h>
//std includes
#include <set>
//boost includes
#include <boost/scoped_ptr.hpp>
//qt includes
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QXmlStreamWriter>
//text includes
#include "text/text.h"

#define FILE_TAG A016CAAF

namespace
{
  const Debug::Stream Dbg("Playlist::IO::XSPF");

  const QLatin1String ENDL("\n");

  const unsigned XSPF_VERSION = 1;

  typedef bool (*AttributesFilter)(const Parameters::NameType&);

  QString DataToQString(const QByteArray& data)
  {
    return QString::fromAscii(data.data(), data.size());
  }

  QString ConvertString(const String& str)
  {
    const QByteArray data = QUrl::toPercentEncoding(ToQString(str));
    return DataToQString(data);
  }

  class ElementHelper
  {
  public:
    ElementHelper(QXmlStreamWriter& xml, const Char* tagName)
      : Xml(xml)
    {
      Xml.writeStartElement(QLatin1String(tagName));
    }

    ~ElementHelper()
    {
      Xml.writeEndElement();
    }

    ElementHelper& Attribute(const Char* name, const QString& value)
    {
      Xml.writeAttribute(QLatin1String(name), value);
      return *this;
    }

    ElementHelper& Text(const QString& str)
    {
      Xml.writeCharacters(str);
      return *this;
    }

    ElementHelper& Text(const Char* name, const QString& str)
    {
      Xml.writeTextElement(QLatin1String(name), str);
      return *this;
    }

    ElementHelper& CData(const QString& str)
    {
      Xml.writeCDATA(str);
      return *this;
    }

    ElementHelper Subtag(const Char* tagName)
    {
      return ElementHelper(Xml, tagName);
    }
  private:
    QXmlStreamWriter& Xml;
  };

  class ExtendedPropertiesSaver : public Parameters::Visitor
  {
    class StringPropertySaver
    {
    public:
      explicit StringPropertySaver(QXmlStreamWriter& xml)
        : Extension(xml, XSPF::EXTENSION_TAG)
      {
        Extension.Attribute(XSPF::APPLICATION_ATTR, QLatin1String(Text::PLAYLIST_APPLICATION_ID));
      }

      void SaveProperty(const Parameters::NameType& name, const String& strVal)
      {
        Extension.Subtag(XSPF::EXTENDED_PROPERTY_TAG)
          .Attribute(XSPF::EXTENDED_PROPERTY_NAME_ATTR, ToQString(name.FullPath()))
          .Text(ConvertString(strVal));
      }
    private:
      ElementHelper Extension;
    };
  public:
    ExtendedPropertiesSaver(QXmlStreamWriter& xml, AttributesFilter filter = 0)
      : XML(xml)
      , Filter(filter)
    {
    }

    virtual void SetValue(const Parameters::NameType& name, Parameters::IntType val)
    {
      if (Filter && !Filter(name))
      {
        return;
      }
      Dbg("  saving extended attribute %1%=%2%", name.FullPath(), val);
      SaveProperty(name, val);
    }

    virtual void SetValue(const Parameters::NameType& name, const Parameters::StringType& val)
    {
      if (Filter && !Filter(name))
      {
        return;
      }
      Dbg("  saving extended attribute %1%='%2%'", name.FullPath(), val);
      SaveProperty(name, val);
    }

    virtual void SetValue(const Parameters::NameType& name, const Parameters::DataType& val)
    {
      if (Filter && !Filter(name))
      {
        return;
      }
      Dbg("  saving extended attribute %1%=data(%2%)", name.FullPath(), val.size());
      SaveProperty(name, val);
    }
  private:
    template<class T>
    void SaveProperty(const Parameters::NameType& name, const T& value)
    {
      if (!Saver)
      {
        Saver.reset(new StringPropertySaver(XML));
      }
      const String strVal = Parameters::ConvertToString(value);
      Saver->SaveProperty(name, strVal);
    }
  private:
    QXmlStreamWriter& XML;
    const AttributesFilter Filter;
    boost::scoped_ptr<StringPropertySaver> Saver;
  };

  class ItemPropertiesSaver : private Parameters::Visitor
  {
  public:
    explicit ItemPropertiesSaver(QXmlStreamWriter& xml)
      : XML(xml)
      , Element(xml, XSPF::ITEM_TAG)
    {
    }

    void SaveModuleLocation(const String& location)
    {
      Dbg("  saving absolute item location %1%", location);
      SaveModuleLocation(ToQString(location));
    }

    void SaveModuleLocation(const String& location, const QDir& root)
    {
      Dbg("  saving relative item location %1%", location);
      const QString path = ToQString(location);
      const QString rel = root.relativeFilePath(path);
      if (path == root.absoluteFilePath(rel))
      {
        SaveModuleLocation(rel);
      }
      else
      {
        SaveModuleLocation(path);
      }
    }

    void SaveModuleProperties(const Module::Information& info, const Parameters::Accessor& props)
    {
      //save common properties
      Dbg(" Save basic properties");
      props.Process(*this);
      SaveDuration(info, props);
      Dbg(" Save extended properties");
      SaveExtendedProperties(props);
    }

    void SaveStubModuleProperties(const Parameters::Accessor& props)
    {
      Dbg(" Save basic stub properties");
      props.Process(*this);
      Dbg(" Save stub extended properties");
      SaveExtendedProperties(props);
    }

    void SaveAdjustedParameters(const Parameters::Accessor& params)
    {
      Dbg(" Save adjusted parameters");
      ExtendedPropertiesSaver saver(XML, &KeepOnlyParameters);
      params.Process(saver);
    }

    void SaveData(const Binary::Data& content)
    {
      Dbg(" Save content");
      Element.Text(ENDL);
      Element.CData(QString::fromAscii(static_cast<const char*>(content.Start()), content.Size()));
      Element.Text(ENDL);
    }
  private:
    void SaveModuleLocation(const QString& location)
    {
      Element.Text(XSPF::ITEM_LOCATION_TAG, DataToQString(QUrl(location).toEncoded()));
    }

    virtual void SetValue(const Parameters::NameType& /*name*/, Parameters::IntType /*val*/)
    {
    }

    virtual void SetValue(const Parameters::NameType& name, const Parameters::StringType& val)
    {
      const String value = Parameters::ConvertToString(val);
      const QString valStr = ConvertString(value);
      if (name == Module::ATTR_TITLE)
      {
        Dbg("  saving item attribute %1%='%2%'", name.FullPath(), val);
        Element.Text(XSPF::ITEM_TITLE_TAG, valStr);
      }
      else if (name == Module::ATTR_AUTHOR)
      {
        Dbg("  saving item attribute %1%='%2%'", name.FullPath(), val);
        Element.Text(XSPF::ITEM_CREATOR_TAG, valStr);
      }
      else if (name == Module::ATTR_COMMENT)
      {
        Dbg("  saving item attribute %1%='%2%'", name.FullPath(), val);
        Element.Text(XSPF::ITEM_ANNOTATION_TAG, valStr);
      }
    }

    virtual void SetValue(const Parameters::NameType& /*name*/, const Parameters::DataType& /*val*/)
    {
    }
  private:
    void SaveDuration(const Module::Information& info, const Parameters::Accessor& props)
    {
      Parameters::IntType frameDuration = Parameters::ZXTune::Sound::FRAMEDURATION_DEFAULT;
      props.FindValue(Parameters::ZXTune::Sound::FRAMEDURATION, frameDuration);
      const uint64_t msecDuration = info.FramesCount() * frameDuration / 1000;
      Dbg("  saving item attribute Duration=%1%", msecDuration);
      Element.Text(XSPF::ITEM_DURATION_TAG, QString::number(msecDuration));
    }

    static bool KeepExtendedProperties(const Parameters::NameType& name)
    {
      return
        //skip path-related properties
        name != Module::ATTR_FULLPATH &&
        name != Module::ATTR_PATH &&
        name != Module::ATTR_FILENAME &&
        name != Module::ATTR_EXTENSION &&
        name != Module::ATTR_SUBPATH &&
        //skip existing properties
        name != Module::ATTR_AUTHOR &&
        name != Module::ATTR_TITLE &&
        name != Module::ATTR_COMMENT &&
        //skip redundand properties
        name != Module::ATTR_CONTENT &&
        //skip all the parameters
        !IsParameter(name)
      ;
    }

    static bool IsParameter(const Parameters::NameType& name)
    {
      return name.IsPath();
    }

    static bool KeepOnlyParameters(const Parameters::NameType& name)
    {
      return name.IsSubpathOf(Parameters::ZXTune::PREFIX) &&
            !name.IsSubpathOf(Playlist::ATTRIBUTES_PREFIX);
    }

    void SaveExtendedProperties(const Parameters::Accessor& props)
    {
      ExtendedPropertiesSaver saver(XML, &KeepExtendedProperties);
      props.Process(saver);
    }
  private:
    QXmlStreamWriter& XML;
    ElementHelper Element;
  };

  class ItemWriter
  {
  public:
    virtual ~ItemWriter() {}

    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const = 0;
  };

  class ItemFullLocationWriter : public ItemWriter
  {
  public:
    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const
    {
      saver.SaveModuleLocation(item.GetFullPath());
    }
  };

  class ItemRelativeLocationWriter : public ItemWriter
  {
  public:
    explicit ItemRelativeLocationWriter(const QString& dirName)
      : Root(dirName)
    {
    }

    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const
    {
      saver.SaveModuleLocation(item.GetFullPath(), Root);
    }
  private:
    const QDir Root;
  };

  class ItemContentLocationWriter : public ItemWriter
  {
  public:
    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const
    {
      if (const Module::Holder::Ptr holder = item.GetModule())
      {
        const Binary::Data::Ptr rawContent = Module::GetRawData(*holder);
        const ZXTune::DataLocation::Ptr container = ZXTune::BuildZdataContainer(*rawContent);
        const String id = container->GetPath()->AsString();
        saver.SaveModuleLocation(XSPF::EMBEDDED_PREFIX + id);
        if (Ids.insert(id).second)
        {
          saver.SaveData(*container->GetData());
        }
        else
        {
          Dbg("Use already stored data for id=%1%", id);
        }
      }
      else
      {
        static const ItemFullLocationWriter fallback;
        fallback.Save(item, saver);
      }
    }
  private:
    mutable std::set<String> Ids;
  };

  class ItemShortPropertiesWriter : public ItemWriter
  {
  public:
    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const
    {
      const Parameters::Accessor::Ptr adjustedParams = item.GetAdjustedParameters();
      saver.SaveStubModuleProperties(*adjustedParams);
      saver.SaveAdjustedParameters(*adjustedParams);
    }
  };

  class ItemFullPropertiesWriter : public ItemWriter
  {
  public:
    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const
    {
      if (const Module::Holder::Ptr holder = item.GetModule())
      {
        const Module::Information::Ptr info = holder->GetModuleInformation();
        const Parameters::Accessor::Ptr props = holder->GetModuleProperties();
        saver.SaveModuleProperties(*info, *props);
        const Parameters::Accessor::Ptr adjustedParams = item.GetAdjustedParameters();
        saver.SaveAdjustedParameters(*adjustedParams);
      }
      else
      {
        static const ItemShortPropertiesWriter fallback;
        fallback.Save(item, saver);
      }
    }
  };

  class ItemCompositeWriter : public ItemWriter
  {
  public:
    ItemCompositeWriter(std::auto_ptr<ItemWriter> loc, std::auto_ptr<ItemWriter> props)
      : Location(loc)
      , Properties(props)
    {
    }

    virtual void Save(const Playlist::Item::Data& item, ItemPropertiesSaver& saver) const
    {
      Location->Save(item, saver);
      Properties->Save(item, saver);
    }
  private:
    const std::auto_ptr<ItemWriter> Location;
    const std::auto_ptr<ItemWriter> Properties;
  };

  std::auto_ptr<const ItemWriter> CreateWriter(const QString& filename, Playlist::IO::ExportFlags flags)
  {
    std::auto_ptr<ItemWriter> location;
    std::auto_ptr<ItemWriter> props;
    if (0 != (flags & Playlist::IO::SAVE_CONTENT))
    {
      location.reset(new ItemContentLocationWriter());
      props.reset(new ItemShortPropertiesWriter());
    }
    else
    {
      if (0 != (flags & Playlist::IO::SAVE_ATTRIBUTES))
      {
        props.reset(new ItemFullPropertiesWriter());
      }
      else
      {
        props.reset(new ItemShortPropertiesWriter());
      }
      if (0 != (flags & Playlist::IO::RELATIVE_PATHS))
      {
        location.reset(new ItemRelativeLocationWriter(QFileInfo(filename).absolutePath()));
      }
      else
      {
        location.reset(new ItemFullLocationWriter());
      }
    }
    return std::auto_ptr<const ItemWriter>(new ItemCompositeWriter(location, props));
  }

  class XSPFWriter
  {
  public:
    XSPFWriter(QIODevice& device, const ItemWriter& writer)
      : XML(&device)
      , Writer(writer)
    {
      XML.setAutoFormatting(true);
      XML.setAutoFormattingIndent(2);
      XML.writeStartDocument();
      XML.writeStartElement(QLatin1String(XSPF::ROOT_TAG));
      XML.writeAttribute(QLatin1String(XSPF::VERSION_ATTR), QLatin1String(XSPF::VERSION_VALUE));
      XML.writeAttribute(QLatin1String(XSPF::XMLNS_ATTR), QLatin1String(XSPF::XMLNS_VALUE));
    }

    void WriteProperties(const Parameters::Accessor& props, uint_t items)
    {
      ExtendedPropertiesSaver saver(XML);
      props.Process(saver);
      saver.SetValue(Playlist::ATTRIBUTE_VERSION, XSPF_VERSION);
      saver.SetValue(Playlist::ATTRIBUTE_ITEMS, items);
    }

    void WriteItems(const Playlist::IO::Container& container, Log::ProgressCallback& cb)
    {
      const uint64_t PERCENTS = 100;
      ElementHelper tracklist(XML, XSPF::TRACKLIST_TAG);
      const uint_t totalItems = container.GetItemsCount();
      uint_t doneItems = 0;
      for (Playlist::Item::Collection::Ptr items = container.GetItems(); items->IsValid(); items->Next())
      {
        const Playlist::Item::Data::Ptr item = items->Get();
        WriteItem(item);
        cb.OnProgress((PERCENTS * ++doneItems / totalItems));
      }
    }

    ~XSPFWriter()
    {
      XML.writeEndDocument();
    }
  private:
    void WriteItem(Playlist::Item::Data::Ptr item)
    {
      Dbg("Save playitem");
      ItemPropertiesSaver saver(XML);
      Writer.Save(*item, saver);
    }
  private:
    QXmlStreamWriter XML;
    const ItemWriter& Writer;
  };
}

namespace Playlist
{
  namespace IO
  {
    void SaveXSPF(Container::Ptr container, const QString& filename, Log::ProgressCallback& cb, ExportFlags flags)
    {
      QFile device(filename);
      if (!device.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
      {
        throw Error(THIS_LINE, FromQString(QFile::tr("Cannot create %1 for output").arg(filename)));
      }
      const std::auto_ptr<const ItemWriter> itemWriter = CreateWriter(filename, flags);
      XSPFWriter writer(device, *itemWriter);
      const Parameters::Accessor::Ptr playlistProperties = container->GetProperties();
      writer.WriteProperties(*playlistProperties, container->GetItemsCount());
      writer.WriteItems(*container, cb);
    }
  }
}

