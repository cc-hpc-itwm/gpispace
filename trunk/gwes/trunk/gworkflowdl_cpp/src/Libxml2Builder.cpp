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

/**
 * Constructor implementation.
 */
Libxml2Builder::Libxml2Builder() {}

/**
 * Destructor implementation.
 */
Libxml2Builder::~Libxml2Builder() {}

// Data
const Data::ptr_t Libxml2Builder::deserializeData(const string &xmlstring) const {
	Data::ptr_t data(new Data(xmlstring));
	return data;
}

const Data::content_t Libxml2Builder::serializeData(const Data::ptr_t &data) const {
	return data->serialize();
}

///dataToElement()
///dataFromElement()

//// Token
//Token deserializeToken(const string& xmlstring) {
//	/// ToDo: implement!
//	string str = xmlstring;
//	return Token(true);
//}
//
//string serializeToken(const Token& token) {
//	/// ToDo: implement!
//	Token mytoken = token;
//	return "<token/>";
//}
//
//// Place
//Place deserializePlace(const string& xmlstring) {
//	/// ToDo: implement!
//	return Place(xmlstring);
//}
//
//string serializePlace(const Place& place) {
//	/// ToDo: implement!
//	Place myplace = place;
//	return "<place/>";
//}
//

} // end namespace gwdl

