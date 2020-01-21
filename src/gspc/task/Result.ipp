#include <gspc/ErrorOr.hpp> //! \todo util-generic/serialization/std/exception_ptr

namespace gspc
{
  namespace task
  {
    namespace result
    {
      template<typename Archive>
        void Success::serialize (Archive& ar, unsigned int)
      {
        ar & output;
      }
      template<typename Archive>
        void Failure::serialize (Archive& ar, unsigned int)
      {
        ar & exception;
      }
      template<typename Archive>
        void CancelIgnored::serialize (Archive& ar, unsigned int)
      {
        ar & after_inject;
      }
      template<typename Archive>
        void CancelOptional::serialize (Archive& ar, unsigned int)
      {
        ar & after_inject;
      }
      template<typename Archive>
        void Postponed::serialize (Archive& ar, unsigned int)
      {
        ar & reason;
      }
    }
  }
}
