/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef EDGE_H_
#define EDGE_H_
//std
#include <string>
//gwdl
#include <gwdl/Place.h>

namespace gwdl
{

/**
 * An Edge represents an arc from a place to a transition or vice versa.
 * It is used to specify the control or data flow in a Petri net.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Edge
{
private:
	/**
     * place the edge is pointing to.
     */
    Place* place;

    /**
     * expression of edge.
     */
    std::string expression;
    
public:
	Edge(Place* _place=NULL, std::string _expression = "") { place = _place; expression = _expression; }
	
	~Edge(){};

    /**
     * set Place the Edge is pointing to.
     *
     * @param p Place Edge should point to
     */
    void setPlace(Place* p) { place = p;}

    /**
     * get place the edge is pointing to.
     *
     * @return Place Edge points to
     */
    Place* getPlace() { return place; }

    std::string getPlaceID() { return place != NULL ? place->getID() : "";}


    /**
     * set Edge's expression.
     *
     * @param ex Edge expression
     */
    void setExpression(std::string ex) { expression = ex; }


    /**
     * get Edge's expression.
     *
     * @return Edge expression
     */
    std::string& getExpression() { return expression; } 
	
};

}

#endif /*EDGE_H_*/
