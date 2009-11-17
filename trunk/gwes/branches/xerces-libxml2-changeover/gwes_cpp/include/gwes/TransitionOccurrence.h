/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TRANSITIONOCCURRENCE_H_
#define TRANSITIONOCCURRENCE_H_
//gwes
#include <gwes/Types.h>
#include <gwes/Activity.h>
//gwdl
#include <gwdl/Transition.h>
#include <gwdl/CapacityException.h>
//fhglog
#include <fhglog/fhglog.hpp>
//std
#include <map>
#include <string>
#include <ostream>

namespace gwes 
{

class TransitionOccurrence 
{

public:

	/**
	 * Constructor.
	 * Uses next unlocked tokens on input/read places to build maps for input/read tokens.
	 */
	explicit TransitionOccurrence(gwdl::Transition::ptr_t transition);

	/**
	 * Destructor. 
	 */
	virtual ~TransitionOccurrence();

	/**
	 * pointer to transition
	 */
	gwdl::Transition::ptr_t transitionP;
	
	/**
	 * pointer to activity
	 */
	Activity* activityP;
	
	/**
	 * list of read/input/write/output parameter tokens
	 */
	parameter_list_t tokens;

    /**
     * true if one or more output or write edgeExpressions correspond to XPath, i.e., contain "$".
     */
    bool hasXPathEdgeExpressions;
    
    /**
     * true if the transition occurrence only simulates the invokation of an activity.
     */
    bool simulation;

    void lockTokens();
	
	void unlockTokens();
	
	bool checkConditions(int step) const;
	
	void removeInputTokens();
	
	void putOutputTokens() throw(gwdl::CapacityException);
	
	void writeWriteTokens() throw(gwdl::CapacityException);
	
	std::string getID() const; 
	
	gwes::parameter_list_t* getTokens();
    
    void evaluateXPathEdgeExpressions(int step);
    
private:
	
	std::string _id;

	fhg::log::logger_t _logger;

};
	
} // end namespace gwes

std::ostream& operator<< (std::ostream &out, gwes::TransitionOccurrence &transitionOccurrence);

#endif /*TRANSITIONOCCURRENCE_H_*/
