/**
* 
* @file
*
* @brief QT application implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "singlemode.h"
#include "ui/factory.h"
#include "ui/utils.h"
#include "supp/options.h"
//common includes
#include <contract.h>
//library includes
#include <platform/application.h>
#include <platform/version/api.h>
//qt includes
#include <QtGui/QApplication>
//std includes
#include <utility>
//text includes
#include "text/text.h"

namespace
{
  class QTApplication : public Platform::Application
  {
  public:
    QTApplication()
    {
    }

    int Run(Strings::Array argv) override
    {
      int fakeArgc = 1;
      char* fakeArgv[] = {""};
      QApplication qapp(fakeArgc, fakeArgv);
      qapp.setOrganizationName(QLatin1String(Text::PROJECT_NAME));
      qapp.setOrganizationDomain(QLatin1String(Text::PROGRAM_SITE));
      qapp.setApplicationVersion(ToQString(Platform::Version::GetProgramVersionString()));
      const Parameters::Container::Ptr params = GlobalOptions::Instance().Get();
      const SingleModeDispatcher::Ptr mode = SingleModeDispatcher::Create(params, std::move(argv));
      if (mode->StartMaster()) {
        const MainWindow::Ptr win = WidgetsFactory::Instance().CreateMainWindow(params);
        Require(win->connect(mode, SIGNAL(OnSlaveStarted(const QStringList&)), SLOT(SetCmdline(const QStringList&))));
        win->SetCmdline(mode->GetCmdline());
        return qapp.exec();
      } else {
        return 0;
      }
    }
  };
}

namespace Platform
{
  std::unique_ptr<Application> Application::Create()
  {
    return std::unique_ptr<Application>(new QTApplication());
  }
}
