namespace gspc
{
  namespace util
  {
   namespace serialization
   {
      namespace detail
      {
        template<typename T> struct serialize_by_member;
      }
    }
  }
}

#define FHG_UTIL_SERIALIZATION_ACCESS_IMPL(type_)                       \
  friend ::gspc::util::serialization::detail::serialize_by_member<type_>
