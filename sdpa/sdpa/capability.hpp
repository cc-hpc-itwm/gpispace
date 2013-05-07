#ifndef SDPA_CAPABILITY_HPP_
#define SDPA_CAPABILITY_HPP_ 1

#include <string>
#include <set>
#include <iostream>
#include <boost/foreach.hpp>
#include <sdpa/events/id_generator.hpp>

namespace sdpa
{

	// type, name
	class Capability
	{
	    public:
			explicit
			Capability(	const std::string& name = "",
						const std::string& type = "",
						const std::string& owner = "",
						const std::string& uuid = sdpa::events::id_generator::instance().next(),
						size_t depth= 0)
			: name_(name)
			, type_(type)
			, depth_(depth)
			, owner_(owner)
			, uuid_(uuid)
			{}

			Capability(const Capability& cpb)
			{
				name_  = cpb.name();
				type_  = cpb.type();
				depth_ = cpb.depth();
				owner_ = cpb.owner();
				uuid_  = cpb.uuid();
			}

			Capability& operator=(const Capability& cpb)
			{
				if(this!=&cpb)
				{
					name_  = cpb.name();
					type_  = cpb.type();
					depth_ = cpb.depth();
					owner_ = cpb.owner();
					uuid_  = cpb.uuid();
				}

				return *this;
			}

			/*bool operator()(const  Capability& a, const Capability& b)
			{
				//Capability cpb(name_, type_, owner_);
				return (a ==*this) && (b==*this)  && (a.depth() < b.depth());
			}*/

			~Capability () {}

			std::string name() const { return name_;}
			void setName(const std::string& name) { name_ = name;}

			std::string type() const { return type_;}
			void setType(const std::string& type) { type_ = type;}

			size_t depth() const { return depth_;}
			void setDepth(size_t depth) { depth_ = depth;}
			void incDepth() { depth_++; }

			std::string owner() const { return owner_; }
			void setOwner(const std::string& owner) { owner_ = owner; }

			std::string uuid() const { return uuid_; }
			void setUuid(const std::string& uuid) { uuid_ = uuid; }
			void assignUuid()
			{
				uuid_ = sdpa::events::id_generator::instance().next();
			}

			template <class Archive>
			void serialize(Archive& ar, const unsigned int)
			{
				ar & name_;
				ar & type_;
				ar & depth_;
				ar & owner_;
				ar & uuid_;
			}

			bool operator<(const Capability& b) const
			{
				return uuid_ < b.uuid();
			}

			bool operator==(const Capability& b) const
			{
				// ignore depth
				return uuid_ == b.uuid();
			}

	    private:
			std::string name_;
			std::string type_;
			mutable size_t depth_;
			std::string owner_;
			std::string uuid_;
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
}

inline std::ostream& operator<<(std::ostream& os, const sdpa::Capability& cpb)
{
	os<<"("<<cpb.name()<<", "<<cpb.type()<<", "<<cpb.depth()<<", "<<cpb.owner()<<", "<<cpb.uuid()<<")";
	return os;
}

inline std::ostream& operator<<(std::ostream& os, const sdpa::capabilities_set_t& cpbSet)
{
	//os<<"----------------------------------------------------------------------------"<<std::endl;
	for(sdpa::capabilities_set_t::iterator it = cpbSet.begin(); it!= cpbSet.end(); it++)
	{
		os<<*it<<std::endl;
	}

	//os<<"----------------------------------------------------------------------------"<<std::endl;
	return os;
}

#endif
