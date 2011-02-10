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

#ifdef USE_REAL_WE
	#include <we/mgmt/basic_layer.hpp>
	#include <we/mgmt/bits/traits.hpp>
	#include <we/mgmt/bits/signal.hpp>
	#include <we/we.hpp>
	#include <we/mgmt/layer.hpp>
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
		preference_t(): mandatory_(false) {}

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

      private:
        bool mandatory_;
        rank_list_type ranks_;
        exclude_set_type excluded_ranks_;
      };



	typedef void activity_information_t;
    struct IWorkflowEngine{};
#endif

#endif //IWORKFLOWENGINE_HPP
