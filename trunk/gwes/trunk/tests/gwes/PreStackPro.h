#ifndef PRESTACKPRO_H_
#define PRESTACKPRO_H_
// std
#include <iostream>
#include <ostream>
#include <pthread.h>
// gwes
#include <gwes/Observer.h>

class PreStackPro : public gwes::Observer
{
public:
	gwes::Observer* _destinationObserverP;
	PreStackPro();
	virtual ~PreStackPro();
	
	/**
	 * This method is called by the WorkflowHandler each time the workflow or an activity changes.
	 */
	virtual void update(const gwes::Event& event);
	
	/**
	 * For the back channel communication.
	 */
	void setDestinationObserver(gwes::Observer* destinationP);
	
	/**
	 * Execute an algorithm. The inputEvent contains the algorithm name and the input data.
	 */ 
	void execute(const gwes::Event& inputEvent);
	
	pthread_t _thread;
	struct thread_data {
	   PreStackPro* psp;
	   gwes::Event event;
	};
	struct thread_data _data;

};

/**
 * This method is invoked using new pthread_create.
 */
void* executeAlgorithm(void*); 

#endif /*PRESTACKPRO_H_*/
