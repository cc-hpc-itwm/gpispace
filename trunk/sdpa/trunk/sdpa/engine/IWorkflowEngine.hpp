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
	#include <we/we.hpp>
	#include <we/mgmt/layer.hpp>
	#include <we/util/codec.hpp>
	#include <we/loader/putget.hpp>
#else
	#include <vector>
	#include <boost/unordered_set.hpp>
	#include <boost/unordered_map.hpp>
#endif

// Assume ids of type string
typedef std::string id_type;
typedef std::string result_type;
typedef std::string reason_type;
typedef std::string encoded_type;

enum ExecutionState
 {
	  ACTIVITY_FINISHED
	, ACTIVITY_FAILED
	, ACTIVITY_CANCELLED
 };

typedef std::pair<ExecutionState, result_type> execution_result_t;

#ifdef USE_REAL_WE
	typedef we::preference_t preference_t;
	typedef we::mgmt::activity_information_t activity_information_t;
	typedef we::mgmt::basic_layer<id_type, result_type, reason_type, encoded_type> IWorkflowEngine;
#else
		//typedef int preference_t;
		struct preference_t
		{
			typedef int rank_type;
			typedef rank_type value_type;
			typedef rank_type argument_type;

			typedef boost::unordered_set<value_type> exclude_set_type;
			typedef std::vector<value_type> rank_list_type;

			preference_t (const bool _mandatory = false) : mandatory_(_mandatory)  { }

			bool is_mandatory (void) const { return mandatory_; }
			bool is_wanted (const rank_type rank) const { return false;}
			bool is_excluded (rank_type rank) const { return false;}

			rank_list_type & ranks(void)
			{
				return ranks_;
			}

			rank_list_type const & ranks(void) const
			{
				return ranks_;
			}

			exclude_set_type & exclusion (void)
			{
				return excluded_ranks_;
			}

			exclude_set_type const & exclusion (void) const
			{
				return excluded_ranks_;
			}

			bool empty (void) const
			{
				return ranks_.empty();
			}

		   preference_t & want (const rank_type rank)
		   {
			   ranks_.push_back (rank);
			   return *this;
		   }

		private:
		   bool mandatory_;
		   rank_list_type ranks_;
		   exclude_set_type excluded_ranks_;
		};

		inline std::ostream & operator << (std::ostream & os, const preference_t & p)
		{
			os << "not implementd!";
			return os;
		}

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

		struct IWorkflowEngine
		{
			virtual void submit(const id_type & id, const encoded_type & ) = 0;
			virtual bool cancel(const id_type & id, const reason_type & reason) = 0;

			virtual bool finished(const id_type & id, const result_type & result) = 0;
			virtual bool failed(const id_type & id, const result_type & result) = 0;
			virtual bool cancelled(const id_type & id) = 0;

			virtual bool fill_in_info (const id_type & id, activity_information_t & info) const = 0;

			virtual ~IWorkflowEngine() {}

			friend class boost::serialization::access;

			template <class Archive>
			void serialize(Archive&, const unsigned int ){}
		};

#endif

#endif //IWORKFLOWENGINE_HPP
