#include "handle_generator.hpp"

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      namespace detail
      {
        struct cat_global {};
        struct cat_local {};
        struct cat_shared {};

        template <typename T, size_t N>
        struct is_bit_size_valid
        {
          static const bool value = ((N+2) < (sizeof(T) * 8));
        };

        template <typename T, size_t N, typename Cat>
        struct compute_N
        {
        };

        template <typename T, size_t N>
        struct compute_N<T, N, cat_global>
        {
          static const int value = N;
        };

        template <typename T, size_t N>
        struct compute_N<T, N, cat_local>
        {
          static const int value = 1;
        };

        template <typename T, size_t N>
        struct compute_N<T, N, cat_shared>
        {
          static const int value = 1;
        };

        template <typename T, size_t N, size_t C = (sizeof(T)*8 - 2 - N)>
        struct encode_helper_t
        {
          union enc_t
          {
            struct
            {
              T segment : 2;
              T node    : N;
              T count   : C;
            } in;
            T handle;
          };
        };

        template <typename T, size_t N, bool>
        struct encoder_t
        {
        };

        template <typename T, size_t N>
        struct encoder_t<T, N, true>
        {
          typedef typename encode_helper_t<T, N>::enc_t enc_t;
        };

        template <typename T, size_t N>
        T encode ( const gpi::pc::type::size_t node_id
                 , const gpi::pc::type::segment_id_t seg_id
                 , const gpi::pc::type::size_t count
                 )
        {
          typedef typename encoder_t
            < T
            , N
            , is_bit_size_valid<T, N>::value
            >::enc_t enc_t;
          enc_t encoder;
          encoder.in.segment = 3;
          encoder.in.node = node_id;
          encoder.in.count = count;
          return encoder.handle;
        }
      }

      handle_generator_t * handle_generator_t::instance = 0;

      handle_generator_t::handle_generator_t(const gpi::pc::type::size_t identifier)
        : m_node_identifier (identifier)
        , m_counter (0)
      {}

      void handle_generator_t::create (const gpi::pc::type::size_t identifier)
      {
        assert (instance == 0);
        instance = new handle_generator_t (identifier);
      }

      handle_generator_t & handle_generator_t::get ()
      {
        assert (instance != 0);
        return *instance;
      }

      void handle_generator_t::destroy ()
      {
        if (instance)
        {
          delete instance;
          instance = 0;
        }
      }

      gpi::pc::type::handle_id_t handle_generator_t::increment ()
      {
        lock_type lock (m_mutex);
        gpi::pc::type::size_t new_count (m_counter + 1);
        if (! new_count) // TODO: branch hint unlikely
        {
          throw std::runtime_error ("cannot create new handle: would overflow");
        }
        m_counter = new_count;
        return new_count;
      }

      gpi::pc::type::handle_id_t handle_generator_t::next (const gpi::pc::type::segment_id_t seg_id)
      {
        gpi::pc::type::size_t counter (increment());
        return detail::encode
          <gpi::pc::type::handle_id_t, 14> (m_node_identifier, seg_id, counter);
      }
    }
  }
}
