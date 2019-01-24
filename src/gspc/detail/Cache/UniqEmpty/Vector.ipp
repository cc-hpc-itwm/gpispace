namespace gspc
{
  namespace detail
  {
    namespace Cache
    {
      namespace UniqEmpty
      {
        template<typename ID>
          bool Vector<ID>::empty() const
        {
          return _.empty();
        }

        template<typename ID>
          void Vector<ID>::push (ID x)
        {
          return _.emplace_back (std::move (x));
        }

        template<typename ID>
          ID Vector<ID>::pop()
        {
          auto x {_.back()};
          _.pop_back();

          return x;
        }
      }
    }
  }
}
