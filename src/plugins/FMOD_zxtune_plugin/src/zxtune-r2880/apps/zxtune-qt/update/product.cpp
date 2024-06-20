/**
* 
* @file
*
* @brief Product entity implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "product.h"
#include "apps/version/api.h"
#include "apps/zxtune-qt/ui/utils.h"
//boost includes
#include <boost/range/end.hpp>
//qt includes
#include <QtCore/QFileInfo>

namespace
{
  class CurrentRelease : public Product::Release
  {
  public:
    virtual Product::Release::PlatformTag Platform() const
    {
      const String txt = GetBuildPlatform();
      if (txt == "windows")
      {
        return Product::Release::WINDOWS;
      }
      else if (txt == "mingw")
      {
        return Product::Release::MINGW;
      }
      else if (txt == "linux")
      {
        return Product::Release::LINUX;
      }
      else if (txt == "dingux")
      {
        return Product::Release::DINGUX;
      }
      else
      {
        return Product::Release::UNKNOWN_PLATFORM;
      }
    }

    virtual Product::Release::ArchitectureTag Architecture() const
    {
      const String txt = GetBuildArchitecture();
      if (txt == "x86")
      {
        return Product::Release::X86;
      }
      else if (txt == "x86_64")
      {
        return Product::Release::X86_64;
      }
      else if (txt == "arm")
      {
        return Product::Release::ARM;
      }
      else if (txt == "armhf")
      {
        return Product::Release::ARMHF;
      }
      else if (txt == "mipsel")
      {
        return Product::Release::MIPSEL;
      }
      else
      {
        return Product::Release::UNKNOWN_ARCHITECTURE;
      }
    }

    virtual QString Version() const
    {
      return ToQString(GetProgramVersion());
    }

    virtual QDate Date() const
    {
      return QDate::fromString(ToQString(GetBuildDate()), Qt::SystemLocaleShortDate);
    }
  };

  using namespace Product;

  struct PackagingTraits
  {
    const char* ReleaseFile;
    Update::PackagingTag Packaging;
  };

  const PackagingTraits PACKAGING_TYPES[] =
  {
    {"arch-release",   Update::TARXZ},
    {"debian_version", Update::DEB},
    {"debian_release", Update::DEB},
    {"redhat-release", Update::RPM},
    {"redhat_version", Update::RPM},
    {"fedora-release", Update::RPM},
    {"centos-release", Update::RPM},
    {"SuSE-release",   Update::RPM},
  };

  Update::PackagingTag GetLinuxPackaging()
  {
    static const QLatin1String RELEASE_DIR("/etc/");
    for (const PackagingTraits* it = PACKAGING_TYPES, *lim = boost::end(PACKAGING_TYPES); it != lim; ++it)
    {
      if (QFileInfo(RELEASE_DIR + it->ReleaseFile).exists())
      {
        return it->Packaging;
      }
    }
    return Update::TARGZ;
  }

  struct ReleaseTypeTraits
  {
    Update::TypeTag Type;
    Release::PlatformTag Platform;
    Release::ArchitectureTag Architecture;
    Update::PackagingTag Packaging;
  };

  const ReleaseTypeTraits RELEASE_TYPES[] =
  {
    {Update::WINDOWS_X86,      Release::WINDOWS, Release::X86,    Update::ZIP},
    {Update::WINDOWS_X86_64,   Release::WINDOWS, Release::X86_64, Update::ZIP},
    {Update::MINGW_X86,        Release::MINGW,   Release::X86,    Update::ZIP},
    {Update::MINGW_X86_64,     Release::MINGW,   Release::X86_64, Update::ZIP},
    {Update::LINUX_X86,        Release::LINUX,   Release::X86,    Update::TARGZ},
    {Update::LINUX_X86_64,     Release::LINUX,   Release::X86_64, Update::TARGZ},
    {Update::LINUX_ARM,        Release::LINUX,   Release::ARM,    Update::TARGZ},
    {Update::LINUX_ARMHF,      Release::LINUX,   Release::ARMHF,  Update::TARGZ},
    {Update::DINGUX_MIPSEL,    Release::DINGUX,  Release::MIPSEL, Update::TARGZ},
    {Update::ARCHLINUX_X86,    Release::LINUX,   Release::X86,    Update::TARXZ},
    {Update::ARCHLINUX_X86_64, Release::LINUX,   Release::X86_64, Update::TARXZ},
    {Update::UBUNTU_X86,       Release::LINUX,   Release::X86,    Update::DEB},
    {Update::UBUNTU_X86_64,    Release::LINUX,   Release::X86_64, Update::DEB},
    {Update::REDHAT_X86,       Release::LINUX,   Release::X86,    Update::RPM},
    {Update::REDHAT_X86_64,    Release::LINUX,   Release::X86_64, Update::RPM},
  };
}

namespace Product
{
  const Release& ThisRelease()
  {
    static const CurrentRelease CURRENT;
    return CURRENT;
  }

  Update::TypeTag GetUpdateType(Release::PlatformTag platform, Release::ArchitectureTag architecture, Update::PackagingTag packaging)
  {
    for (const ReleaseTypeTraits* it = RELEASE_TYPES, *lim = boost::end(RELEASE_TYPES); it != lim; ++it)
    {
      if (it->Platform == platform &&
          it->Architecture == architecture &&
          it->Packaging == packaging)
      {
        return it->Type;
      }
    }
    return Update::UNKNOWN_TYPE;
  }

  std::vector<Update::TypeTag> SupportedUpdateTypes()
  {
    std::vector<Update::TypeTag> result;
    const Release::PlatformTag platform = ThisRelease().Platform();
    const Release::ArchitectureTag architecture = ThisRelease().Architecture();

    //do not use cross-architecture update types
    switch (platform)
    {
    case Release::MINGW:
      result.push_back(GetUpdateType(Release::MINGW, architecture, Update::ZIP));
    case Release::WINDOWS:
      result.push_back(GetUpdateType(Release::WINDOWS, architecture, Update::ZIP));
      break;
    case Release::LINUX:
      {
        const Update::PackagingTag packaging = GetLinuxPackaging();
        result.push_back(GetUpdateType(Release::LINUX, architecture, packaging));
        if (packaging != Update::TARGZ)
        {
          result.push_back(GetUpdateType(Release::LINUX, architecture, Update::TARGZ));
        }
      };
      break;
    case Release::DINGUX:
      result.push_back(GetUpdateType(platform, architecture, Update::TARGZ));
      break;
    default:
      break;
    };
    return result;
  }
}
