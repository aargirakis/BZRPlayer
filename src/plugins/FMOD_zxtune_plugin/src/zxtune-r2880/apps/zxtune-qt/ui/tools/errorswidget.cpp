/**
* 
* @file
*
* @brief Errors widget implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "errorswidget.h"
#include "errorswidget.ui.h"
#include "ui/utils.h"
//common includes
#include <contract.h>
//std includes
#include <list>
//qt includes
#include <QtGui/QLabel>
#include <QtGui/QPainter>

namespace
{
  class ErrorsContainer
  {
  public:
    ErrorsContainer()
      : Current(Container.end())
    {
    }

    void Add(const Error& err)
    {
      Container.push_back(err);
      FixLast();
    }

    const Error& Get() const
    {
      assert(!Container.empty());
      return *Current;
    }

    bool IsFirst() const
    {
      return Current == Container.begin();
    }

    bool IsLast() const
    {
      ContainerType::const_iterator cur = Current;
      return cur != Container.end() && ++cur == Container.end();
    }

    std::size_t Count() const
    {
      return Container.size();
    }

    void Remove()
    {
      Current = Container.erase(Current);
      FixLast();
    }

    void Clear()
    {
      Container.clear();
      Current = Container.end();
    }

    void Backward()
    {
      assert(!IsFirst());
      --Current;
    }

    void Forward()
    {
      assert(!IsLast());
      ++Current;
    }
  private:
    void FixLast()
    {
      if (Current == Container.end() && Container.size())
      {
        --Current;
      }
    }
  private:
    typedef std::list<Error> ContainerType;
    ContainerType Container;
    ContainerType::iterator Current;
  };

  class ErrorText : public QLabel
  {
  public:
    explicit ErrorText(QWidget& parent)
      : QLabel(&parent)
    {
      setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    }

    void Set(const Error& err)
    {
      setText(ToQString(err.GetText()));
      setToolTip(ToQString(err.ToString()));
    }

    void paintEvent(QPaintEvent*)
    {
      QPainter p(this);
      QFontMetrics fm(font());
      const QString& fullText = text();
      const QSize& fullSize = fm.size(Qt::TextShowMnemonic, fullText);
      const QRect& availRect = contentsRect();
      const QRect& drawRect = availRect.translated(0, (availRect.height() - fullSize.height()) / 2);
      if (fullSize.width() > availRect.width())
      {
        const QString& elidedText = fontMetrics().elidedText(fullText, Qt::ElideRight, availRect.width(), Qt::TextShowMnemonic);
        p.drawText(drawRect, elidedText);
      }
      else
      {
        p.drawText(drawRect, fullText);
      }
    }
  private:
    QString FullText;
  };

  class SimpleErrorsWidget : public UI::ErrorsWidget
                           , public Ui::ErrorsWidget
  {
  public:
    explicit SimpleErrorsWidget(QWidget& parent)
      : UI::ErrorsWidget(parent)
      , Current(new ErrorText(*this))
    {
      //setup self
      setupUi(this);
      horizontalLayout->insertWidget(1, Current);

      Require(connect(prevError, SIGNAL(clicked(bool)), SLOT(Previous())));
      Require(connect(nextError, SIGNAL(clicked(bool)), SLOT(Next())));
      Require(connect(dismissCurrent, SIGNAL(clicked(bool)), SLOT(Dismiss())));
      Require(connect(dismissAll, SIGNAL(clicked(bool)), SLOT(DismissAll())));

      UpdateUI();
    }

    virtual void AddError(const Error& err)
    {
      Errors.Add(err);
      UpdateUI();
    }

    virtual void Previous()
    {
      Errors.Backward();
      UpdateUI();
    }

    virtual void Next()
    {
      Errors.Forward();
      UpdateUI();
    }

    virtual void Dismiss()
    {
      Errors.Remove();
      UpdateUI();
    }

    virtual void DismissAll()
    {
      Errors.Clear();
      UpdateUI();
    }
  private:
    void UpdateUI()
    {
      if (const std::size_t count = Errors.Count())
      {
        show();
        prevError->setDisabled(Errors.IsFirst());
        nextError->setDisabled(Errors.IsLast());
        dismissAll->setText(QString::number(count));
        const Error& cur = Errors.Get();
        Current->Set(cur);
      }
      else
      {
        hide();
      }
    }
  private:
    ErrorsContainer Errors;
    ErrorText* const Current;
  };
}

namespace UI
{
  ErrorsWidget::ErrorsWidget(QWidget& parent) : QWidget(&parent)
  {
  }

  ErrorsWidget* ErrorsWidget::Create(QWidget& parent)
  {
    return new SimpleErrorsWidget(parent);
  }
}
