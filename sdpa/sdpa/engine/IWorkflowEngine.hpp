/*
 * =====================================================================================
 *
 *       Filename:  IWorkflowEngine.hpp
 *
 *    Description:  Redefines the interface to gwes
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef IWORKFLOWENGINE_HPP
#define IWORKFLOWENGINE_HPP 1

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>

#include <sdpa/sdpa-config.hpp>

#ifdef USE_REAL_WE
	#include <we/mgmt/basic_layer.hpp>
	#include <we/mgmt/bits/traits.hpp>
	#include <we/mgmt/bits/signal.hpp>
	#include <we/mgmt/type/requirement.hpp>
	#include <we/we.hpp>
	#include <we/mgmt/layer.hpp>
	#include <we/util/codec.hpp>
	#include <we/loader/putget.hpp>
#else
	#include <vector>
	#include <boost/unordered_set.hpp>
	#include <boost/unordered_map.hpp>
#endif

typedef we::mgmt::basic_layer::id_type id_type;
typedef we::mgmt::basic_layer::result_type result_type;
typedef we::mgmt::basic_layer::reason_type reason_type;
typedef we::mgmt::basic_layer::encoded_type encoded_type;

enum ExecutionState
 {
	  ACTIVITY_FINISHED
	, ACTIVITY_FAILED
	, ACTIVITY_CANCELLED
 };

typedef std::pair<ExecutionState, result_type> execution_result_t;

#ifdef USE_REAL_WE
		typedef we::mgmt::requirement_t<std::string> requirement_t;
		typedef std::list<requirement_t> requirement_list_t;
		typedef we::mgmt::activity_information_t activity_information_t;
		typedef we::mgmt::basic_layer IWorkflowEngine;
#else
	   // template <typename T>
	    struct requirement_t
	    {
	      typedef std::string value_type;
	      typedef value_type argument_type;

	      template <typename U>
	      struct rebind
	      {
	        typedef requirement_t<U> other;
	      };

	      explicit
	      requirement_t (value_type arg, const bool _mandatory = false)
	        : value_(arg)
	        , mandatory_(_mandatory)
	      {}

	      requirement_t (requirement_t<T> const &other)
	        : value_(other.value_)
	        , mandatory_(other.mandatory_)
	      {}

	      requirement_t<T> & operator=(requirement_t<T> const & rhs)
	      {
	        this->value_ = rhs.value_;
	        this->mandatory_ = rhs.mandatory_;
	        return *this;
	      }

	      ~requirement_t () {}

	      virtual bool is_mandatory (void) const
	      {
	        return mandatory_;
	      }

	      const value_type & value(void) const
	      {
	        return value_;
	      }

	      void value(const value_type & val)
	      {
	        value_ = val;
	      }
	    private:
	      bool mandatory_;
	      value_type value_;
	    };

	    typedef std::list<requirement_t> requirement_list_t;

	    template <typename T>
	    requirement_t<T> make_mandatory (T val)
	    {
	      return requirement_t<T> (val, true);
	    }

	    template <typename T>
	    requirement_t<T> make_optional (T val)
	    {
	      return requirement_t<T> (val, false);
	    }

	    /*
		struct activity_information_t
		{
			enum status_t
			{
				UNDEFINED = -1
				, PENDING
				, RUNNING
				, FINISHED
				, FAILED
				, CANCELLED
				, SUSPENDED
			};

			std::string name;
			status_t status;
			int level;

			typedef boost::unordered_map<std::string, std::string> data_t;
			data_t data;
		};
		*/

		struct IWorkflowEngine
		{
			virtual void submit(const id_type & id, const encoded_type & ) = 0;
			virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

			virtual bool finished(const id_type & id, const result_type & result) = 0;
			virtual bool failed(const id_type & id, const result_type & result) = 0;
			virtual bool cancelled(const id_type & id) = 0;

			virtual void set_rank(const unsigned int& rank ){ //to be overridden! }

			virtual bool fill_in_info (const id_type & id, activity_information_t & info) const = 0;
			virtual ~IWorkflowEngine() {}

			friend class boost::serialization::access;

			template <class Archive>
			void serialize(Archive&, const unsigned int ){}
		};

#endif

#endif //IWORKFLOWENGINE_HPP
