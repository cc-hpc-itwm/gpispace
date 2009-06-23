/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef CHANNEL_H_
#define CHANNEL_H_
//gwes
#include <gwes/Observer.h>

namespace gwes
{

/**
 * Channel for the communication between the GWES and external entities.
 * This channel uses two observers aka listerners (-> Observer) for the bidirectional communication.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Channel
{
public:
	
	/**
	 * Constructor for Channel.
	 * @param sourceP The local observer which is called by the destination.
	 */
	Channel(Observer* sourceP) {_sourceP = sourceP;}
	
	/**
	 * Empty Destructor.
	 */
	virtual ~Channel(){}
	
	/**
	 * Pointer to the local observer which is called by the destination.
	 */ 
	Observer* _sourceP;
	
	/**
	 * Pointer to the external observer which is called by the source.
	 */
	Observer* _destinationP;
	
	/**
	 * Set the destination observer.
	 * @param destinationP Pointer to the destination observer.
	 */
	void setDestination(Observer* destinationP) {_destinationP = destinationP;}
	
};

}

#endif /*CHANNEL_H_*/
