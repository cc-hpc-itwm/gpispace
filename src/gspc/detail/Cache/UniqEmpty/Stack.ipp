namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqEmpty
      {
        template<typename ID>
          bool Stack<ID>::empty() const
        {
          return _.empty();
        }

        template<typename ID>
          void Stack<ID>::push (ID x)
        {
          return _.push (std::move (x));
        }

        template<typename ID>
          ID Stack<ID>::pop()
        {
          auto x (_.top());
          _.pop();

          return x;
        }

      }
    }
  }
}
