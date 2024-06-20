/**
 *
 * @file
 *
 * @brief About dialog implementation
 *
 * @author vitamin.caig@gmail.com
 *
 **/

// local includes
#include "aboutdialog.h"
#include "aboutdialog.ui.h"
#include "ui/utils.h"
#include "urls.h"
// library includes
#include <platform/version/api.h>
// std includes
#include <utility>
// qt includes
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

namespace
{
  const char* const FEEDBACK_FORMAT =
      QT_TRANSLATE_NOOP("AboutDialog", "<a href=\"mailto:%1?subject=Feedback for %2\">Send feedback</a>");

  class AboutDialog
    : public QDialog
    , private Ui::AboutDialog
  {
  public:
    explicit AboutDialog(QWidget& parent)
      : QDialog(&parent)
    {
      // do not set parent
      setupUi(this);
      const QString appVersion(ToQString(Platform::Version::GetProgramVersionString()));
      buildLabel->setText(appVersion);
      const QString feedbackFormat(QApplication::translate("AboutDialog", FEEDBACK_FORMAT));
      feedbackLabel->setText(feedbackFormat.arg(ToQString(Urls::Email())).arg(appVersion));
    }
  };
}  // namespace

namespace UI
{
  void ShowProgramInformation(QWidget& parent)
  {
    AboutDialog dialog(parent);
    dialog.exec();
  }
}  // namespace UI
