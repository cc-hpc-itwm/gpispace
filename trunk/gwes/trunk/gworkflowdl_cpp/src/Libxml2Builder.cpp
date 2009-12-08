/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Libxml2Builder.h>

using namespace std;

namespace gwdl
{

// Data
Data Libxml2Builder::deserializeData(const string& xmlstring) {
	/// ToDo: implement!
	return Data(xmlstring);
}

string Libxml2Builder::serialize(const Data& data) {
	/// ToDo: implement!
	Data mydata = data;
	return "<data/>";
}

// Token
Token deserializeToken(const string& xmlstring) {
	/// ToDo: implement!
	string str = xmlstring;
	return Token(true);
}

string serialize(const Token& token) {
	/// ToDo: implement!
	Token mytoken = token;
	return "<token/>";
}

// Place
Place deserializePlace(const string& xmlstring) {
	/// ToDo: implement!
	return Place(xmlstring);
}

string serialize(const Place& place) {
	/// ToDo: implement!
	Place myplace = place;
	return "<place/>";
}


} // end namespace gwdl

