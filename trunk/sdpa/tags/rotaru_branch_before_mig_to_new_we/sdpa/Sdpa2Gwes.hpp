/*
 * =====================================================================================
 *
 *       Filename:  Sdpa2Gwes.hpp
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
#ifndef SDPA2GWES_HPP_
#define SDPA2GWES_HPP_

#include <gwes/Sdpa2Gwes.h>

// comment this for tets
#include <gwes/TokenParameter.h>

namespace sdpa {
	// replace this with gwes::ParameterT*
	typedef gwes::TokenParameter parameter_t;
	typedef gwes::Sdpa2Gwes<parameter_t> Sdpa2Gwes;
	typedef gwes::Sdpa2Gwes<parameter_t>::parameter_list_t parameter_list_t;
}

#endif /*SDPA2GWES_HPP_*/
