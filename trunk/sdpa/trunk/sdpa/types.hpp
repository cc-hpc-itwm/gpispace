#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <string>
#include <sdpa/JobId.hpp>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <iostream>
#include <boost/foreach.hpp>

namespace sdpa {
	typedef sdpa::JobId job_id_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef worker_id_t agent_id_t;
	typedef std::string status_t;
	typedef std::string job_result_t;
	typedef std::list<sdpa::worker_id_t> worker_id_list_t;
	typedef worker_id_list_t agent_id_list_t;
	typedef std::map<sdpa::worker_id_t, unsigned int> map_degs_t;

	class MasterInfo
	{
	public:
		MasterInfo(const std::string& name  = "", bool registered = false )
		: name_(name)
		, registered_(registered)
		{}

		std::string name() const { return name_; }
		bool is_registered() const { return registered_; }
		void set_registered(bool b) { registered_ = b; }

		template <class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar & name_;
		}
	private:
		std::string name_;
		bool registered_;
	};

	typedef std::vector<MasterInfo> master_info_list_t;

	// type, name
	class Capability
	{
	    public:
			Capability(const std::string& name = "", const std::string& type = "", const std::string& owner = "" )
			: name_(name)
			, type_(type)
			, owner_(owner)
			{}

			Capability(const Capability& cpb)
			{
				name_  = cpb.name();
				type_  = cpb.type();
				owner_ = cpb.owner();
			}

			Capability& operator=(const Capability& cpb)
			{
				if(this!=&cpb)
				{
					name_  = cpb.name();
					type_  = cpb.type();
					owner_ = cpb.owner();
				}

				return *this;
			}

			virtual ~Capability () {}

			std::string name() const { return name_;}
			void setName(const std::string& name) { name_ = name;}
			std::string type() const { return type_;}
			void setType(const std::string& type) { type_ = type;}


			std::string owner() const { return owner_; }
			void setOwner(const std::string& owner) { owner_ = owner; }

			template <class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				ar & name_;
				ar & type_;
				ar & owner_;
			}

			bool operator<(const Capability& b) const
			{
				if( type() != b.type() ) {
					return type() < b.type();
				}
				else if( name() != b.name() ) {
					return name() < b.name();
				}
				else {
					return owner() < b.owner();
				}

				return false;
			}

			bool operator==(const Capability& b) const
			{
				return ( (type() == b.type()) && (name() == b.name()) && (owner() == b.owner()));
			}


	    private:
			std::string name_;
			std::string type_;
			std::string owner_;
	};

	typedef Capability capability_t;

	/**
	 * compare the workers
	 */
	struct Compare
	{
		bool operator()(const  capability_t& a, const capability_t& b)
		{
			return a<b;
		}
	};


	typedef std::set<capability_t /*,Compare*/ > capabilities_set_t;
	//typedef std::map<capability_t, unsigned int > capabilities_map_t;
}

inline std::ostream& operator<<(std::ostream& os, const sdpa::Capability& cpb)
{
	os<<"name: "<<cpb.name()<<", type: "<<cpb.type()<</*", depth: "<<cpb.depth()<<*/", owner = "<<cpb.owner();
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const sdpa::capabilities_set_t& cpbSet)
{
	os<<std::endl<<"--------------------------------------------------------"<<std::endl;

	BOOST_FOREACH(const sdpa::capability_t& cpb, cpbSet)
	{
		os<<cpb<<std::endl;
	}

	os<<"--------------------------------------------------------"<<std::endl;
	return os;
}

#endif
