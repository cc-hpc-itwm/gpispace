/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef XPATHEVALUATOR_H_
#define XPATHEVALUATOR_H_
// gwdl
#include <gwdl/Transition.h>
// libxml2
#include <libxml/xpath.h>

namespace gwes
{

/**
 * The XPathEvaluator uses libxml2 for evaluating XPath expressions.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class XPathEvaluator
{
	
private:
	
	/**
	 * Cache for XML context
	 */
	static xmlXPathContextPtr _cacheXmlContextP;
	static xmlDocPtr _cacheXmlContextDocP;
	static gwdl::Transition* _cacheTransitionP;
	static int _cacheStep;
	
	/**
	 * Pointer to XML context for XPath evaluation.
	 */
	xmlXPathContextPtr _xmlContextP;
	
	/**
	 * Pointer to XML document context.
	 */
	xmlDocPtr _xmlContextDocP;
	
	void addTokenToContext(const std::string& edgeExpression, gwdl::Token* tokenP);
	
public:
	
	/**
	 * Constructor for XPathEvaluator.
	 * Note: xmlInitParser() and LIBXML_TEST_VERSION must be invoked only ONCE before invoking this constructor!
	 * @param xmlXpathContextChar The context for the evaluation as const char*.
	 */
	explicit XPathEvaluator(const char* xmlXpathContextChar);
	
	/**
	 * Constructor for XPathEvaluator.
	 * Note: xmlInitParser() and LIBXML_TEST_VERSION must be invoked only ONCE before invoking this constructor!
	 * @param transitionP Pointer to the transition from which to build the context for the evaluation.
	 */
	explicit XPathEvaluator(gwdl::Transition* transitionP, int step);

	/**
	 * Destructor.
	 * Note: xmlCleanupParser() needs to be invoked ONCE after destructing all XPathEvaluators! 
	 */
	virtual ~XPathEvaluator();
	
	/**
	 * Evaluate a boolean XPath expression against the XML XPath context set in the constructor.
	 * @param xPathExprChar The XPath expression to evaluate. 
	 * @return "1" if expression is true, "0" if expression is false, "-1" if there has been an error in the evaluation.
	 */
	int evalCondition(const char* xPathExprChar);
	
	/**
	 * Evaluate a XPath expression against the XML XPath context set in the constructor.
	 * @param xPathExprChar The XPath expression to evaluate. 
	 * @return The XML result of the evaluation.
	 */
	const char* evalExpression(const char* xPathExprChar);
	
	void printXmlNodes(xmlNodeSetPtr nodes);
	
	xmlXPathContextPtr getXmlContext() { return _xmlContextP; }
	
	/** 
	 * Replace "$" by "/token/" in string
	 */
	std::string expandVariables(std::string& str);
	
};

}

#endif /*XPATHEVALUATOR_H_*/

