/**
*
* @file
*
* @brief  Format syntax implementation
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include "grammar.h"
#include "syntax.h"
//common includes
#include <contract.h>
#include <iterator.h>
#include <make_ptr.h>
//std includes
#include <cctype>
#include <stack>
#include <utility>

namespace Binary
{
namespace FormatDSL
{
  inline uint_t ParseDecimalValue(const std::string& num)
  {
    Require(!num.empty());
    uint_t res = 0;
    for (RangeIterator<std::string::const_iterator> it(num.begin(), num.end()); it; ++it)
    {
      Require(0 != std::isdigit(*it));
      res = res * 10 + (*it - '0');
    }
    return res;
  }

  struct Token
  {
    LexicalAnalysis::TokenType Type;
    std::string Value;

    Token()
      : Type(DELIMITER)
      , Value(" ")
    {
    }

    Token(LexicalAnalysis::TokenType type, std::string lexeme)
      : Type(type)
      , Value(std::move(lexeme))
    {
    }
  };

  class State
  {
  public:
    virtual ~State() = default;

    virtual const State* Transition(const Token& tok, FormatTokensVisitor& visitor) const = 0;

    static const State* Initial();
    static const State* Quantor();
    static const State* QuantorEnd();
    static const State* Error();
  };

  class InitialStateType : public State
  {
  public:
    InitialStateType()
      : State()
    {
    }

    const State* Transition(const Token& tok, FormatTokensVisitor& visitor) const override
    {
      switch (tok.Type)
      {
      case DELIMITER:
        return this;
      case OPERATION:
        return ParseOperation(tok, visitor);
      case CONSTANT:
      case MASK:
        return ParseValue(tok, visitor);
      default:
        return State::Error();
      }
    }
  private:
    const State* ParseOperation(const Token& tok, FormatTokensVisitor& visitor) const
    {
      Require(tok.Value.size() == 1);
      switch (tok.Value[0])
      {
      case GROUP_BEGIN:
        visitor.GroupStart();
        return this;
      case GROUP_END:
        visitor.GroupEnd();
        return this;
      case QUANTOR_BEGIN:
        return State::Quantor();
      case RANGE_TEXT:
      case CONJUNCTION_TEXT:
      case DISJUNCTION_TEXT:
        visitor.Operation(tok.Value);
        return this;
      default:
        return State::Error();
      }
    }

    const State* ParseValue(const Token& tok, FormatTokensVisitor& visitor) const
    {
      if (tok.Value[0] == BINARY_MASK_TEXT ||
          tok.Value[0] == MULTIPLICITY_TEXT ||
          tok.Value[0] == ANY_BYTE_TEXT ||
          tok.Value[0] == SYMBOL_TEXT)
      {
        visitor.Match(tok.Value);
        return this;
      }
      else
      {
        Require(tok.Value.size() % 2 == 0);
        std::string val = tok.Value;
        while (!val.empty())
        {
          visitor.Match(val.substr(0, 2));
          val = val.substr(2);
        }
        return this;
      }
    }
  };

  class QuantorStateType : public State
  {
  public:
    QuantorStateType()
      : State()
    {
    }

    const State* Transition(const Token& tok, FormatTokensVisitor& visitor) const override
    {
      if (tok.Type == CONSTANT)
      {
        const uint_t num = ParseDecimalValue(tok.Value);
        visitor.Quantor(num);
        return State::QuantorEnd();
      }
      else
      {
        return State::Error();
      }
    }
  };

  class QuantorEndType : public State
  {
  public:
    QuantorEndType()
      : State()
    {
    }

    const State* Transition(const Token& tok, FormatTokensVisitor& /*visitor*/) const override
    {
      Require(tok.Type == OPERATION);
      Require(tok.Value == std::string(1, QUANTOR_END));
      return State::Initial();
    }
  };

  class ErrorStateType : public State
  {
  public:
    ErrorStateType()
      : State()
    {
    }

    const State* Transition(const Token& /*token*/, FormatTokensVisitor& /*visitor*/) const override
    {
      return this;
    }
  };

  const State* State::Initial()
  {
    static const InitialStateType instance;
    return &instance;
  }

  const State* State::Quantor()
  {
    static const QuantorStateType instance;
    return &instance;
  }

  const State* State::QuantorEnd()
  {
    static const QuantorEndType instance;
    return &instance;
  }

  const State* State::Error()
  {
    static const ErrorStateType instance;
    return &instance;
  }

  class ParseFSM : public LexicalAnalysis::Grammar::Callback
  {
  public:
    explicit ParseFSM(FormatTokensVisitor& visitor)
      : CurState(State::Initial())
      , Visitor(visitor)
    {
    }

    void TokenMatched(const std::string& lexeme, LexicalAnalysis::TokenType type) override
    {
      CurState = CurState->Transition(Token(type, lexeme), Visitor);
      Require(CurState != State::Error());
    }

    void MultipleTokensMatched(const std::string& /*lexeme*/, const LexicalAnalysis::TokenTypesSet& /*types*/) override
    {
      Require(false);
    }

    void AnalysisError(const std::string& /*notation*/, std::size_t /*position*/) override
    {
      Require(false);
    }
  private:
    const State* CurState;
    FormatTokensVisitor& Visitor;
  };

  const std::string GROUP_START(1, GROUP_BEGIN);

  struct Operator
  {
  public:
    Operator()
      : Val()
      , Prec(0)
    {
    }

    explicit Operator(std::string op)
      : Val(std::move(op))
      , Prec(0)
    {
      Require(!Val.empty());
      switch (Val[0])
      {
      case RANGE_TEXT:
        ++Prec;
      case CONJUNCTION_TEXT:
        ++Prec;
      case DISJUNCTION_TEXT:
        ++Prec;
      }
    }

    std::string Value() const
    {
      return Val;
    }

    bool IsOperation() const
    {
      return Prec > 0;
    }

    std::size_t Precedence() const
    {
      return Prec;
    }

    std::size_t Parameters() const
    {
      return 2;
    }

    bool LeftAssoc() const
    {
      return true;
    }
  private:
    const std::string Val;
    std::size_t Prec;
  };

  class RPNTranslation : public FormatTokensVisitor
  {
  public:
    RPNTranslation(FormatTokensVisitor& delegate)
      : Delegate(delegate)
      , LastIsMatch(false)
    {
    }

    void Match(const std::string& val) override
    {
      if (LastIsMatch)
      {
        FlushOperations();
      }
      Delegate.Match(val);
      LastIsMatch = true;
    }

    void GroupStart() override
    {
      FlushOperations();
      Ops.push(Operator(GROUP_START));
      Delegate.GroupStart();
      LastIsMatch = false;
    }

    void GroupEnd() override
    {
      FlushOperations();
      Require(!Ops.empty() && Ops.top().Value() == GROUP_START);
      Ops.pop();
      Delegate.GroupEnd();
      LastIsMatch = false;
    }

    void Quantor(uint_t count) override
    {
      FlushOperations();
      Delegate.Quantor(count);
      LastIsMatch = false;
    }

    void Operation(const std::string& op) override
    {
      const Operator newOp(op);
      FlushOperations(newOp);
      Ops.push(newOp);
      LastIsMatch = false;
    }

    void Flush()
    {
      while (!Ops.empty())
      {
        Require(Ops.top().IsOperation());
        FlushOperations();
      }
    }
  private:
    void FlushOperations()
    {
      while (!Ops.empty())
      {
        const Operator& topOp = Ops.top();
        if (!topOp.IsOperation())
        {
          break;
        }
        Delegate.Operation(topOp.Value());
        Ops.pop();
      }
    }
    void FlushOperations(const Operator& newOp)
    {
      while (!Ops.empty())
      {
        const Operator& topOp = Ops.top();
        if (!topOp.IsOperation())
        {
          break;
        }
        if ((newOp.LeftAssoc() && newOp.Precedence() <= topOp.Precedence())
          || (newOp.Precedence() < topOp.Precedence()))
        {
          Delegate.Operation(topOp.Value());
          Ops.pop();
        }
        else
        {
          break;
        }
      }
    }
  private:
    FormatTokensVisitor& Delegate;
    std::stack<Operator> Ops;
    bool LastIsMatch;
  };

  class SyntaxCheck : public FormatTokensVisitor
  {
  public:
    explicit SyntaxCheck(FormatTokensVisitor& delegate)
      : Delegate(delegate)
      , Position(0)
    {
    }

    void Match(const std::string& val) override
    {
      Delegate.Match(val);
      ++Position;
    }

    void GroupStart() override
    {
      GroupStarts.push(Position);
      Delegate.GroupStart();
    }

    void GroupEnd() override
    {
      Require(!GroupStarts.empty());
      Require(GroupStarts.top() != Position);
      Groups.push(Group(GroupStarts.top(), Position));
      GroupStarts.pop();
      Delegate.GroupEnd();
    }

    void Quantor(uint_t count) override
    {
      Require(Position != 0);
      Require(count != 0);
      Delegate.Quantor(count);
    }

    void Operation(const std::string& op) override
    {
      const std::size_t usedVals = Operator(op).Parameters();
      CheckAvailableParameters(usedVals, Position);
      Position = Position - usedVals + 1;
      Delegate.Operation(op);
    }
  private:
    void CheckAvailableParameters(std::size_t parameters, std::size_t position)
    {
      if (!parameters)
      {
        return;
      }
      const std::size_t start = GroupStarts.empty() ? 0 : GroupStarts.top();
      if (Groups.empty() || Groups.top().End < start)
      {
        Require(parameters + start <= position);
        return;
      }
      const Group top = Groups.top();
      const std::size_t nonGrouped = position - top.End;
      if (nonGrouped < parameters)
      {
        if (nonGrouped)
        {
          CheckAvailableParameters(parameters - nonGrouped, top.End);
        }
        else
        {
          Require(top.Size() == 1);
          Groups.pop();
          CheckAvailableParameters(parameters - 1, top.Begin);
          Groups.push(top);
        }
      }
    }
  private:
    struct Group
    {
      Group(std::size_t begin, std::size_t end)
        : Begin(begin)
        , End(end)
      {
      }

      Group()
        : Begin()
        , End()
      {
      }

      std::size_t Size() const
      {
        return End - Begin;
      }

      std::size_t Begin;
      std::size_t End;
    };
  private:
    FormatTokensVisitor& Delegate;
    std::size_t Position;
    std::stack<std::size_t> GroupStarts;
    std::stack<Group> Groups;
  };
}
}

namespace Binary
{
namespace FormatDSL
{
  void ParseFormatNotation(const std::string& notation, FormatTokensVisitor& visitor)
  {
    ParseFSM fsm(visitor);
    CreateFormatGrammar()->Analyse(notation, fsm);
  }

  void ParseFormatNotationPostfix(const std::string& notation, FormatTokensVisitor& visitor)
  {
    RPNTranslation rpn(visitor);
    ParseFormatNotation(notation, rpn);
    rpn.Flush();
  }

  FormatTokensVisitor::Ptr CreatePostfixSyntaxCheckAdapter(FormatTokensVisitor& visitor)
  {
    return MakePtr<SyntaxCheck>(visitor);
  }
}
}
