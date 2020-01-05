#pragma once
#include <utility>

/** scope-exit handler */
template <typename TOp>
class Finally
{
public:
   Finally(TOp op) : m_op(std::move(op)) {}
   void Dismiss() { m_dismissed = true; }
   ~Finally()
   {
      if (!m_dismissed)
         m_op();
   }

private:
   TOp m_op;
   bool m_dismissed = false;
};
