/*
 * =====================================================================================
 *
 *       Filename:  TORUSGwes.hpp
 *
 *    Description:  Simulate simple gwes behavior
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
#ifndef TOKEN_HPP
#define TOKEN_HPP 1
#include <sdpa/engine/Token.hpp>

#include <boost/numeric/ublas/assignment.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/numeric/ublas/io.hpp> //for test

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

enum direction_t {LEFT, RIGHT, UP, DOWN};
enum color_t {YELLOW, RED, BLUE, BLACK};
typedef boost::numeric::ublas::matrix<int> matrix_t;

// a token contains
//   color
//   rank (optional)
//   owner (very imp-> for temination)
// printing:

void print(const matrix_t& A)
{ // Print a uBLAS matrix

	for (unsigned i = 0; i < A.size1(); ++i)
	{
		for (unsigned j = 0; j < A.size2(); ++j)
			std::cout<< std::fixed << std::setw(5) << A(i, j) << ", ";
		std::cout << std::endl;
	}
}

class Token
{
public:
	 typedef sdpa::shared_ptr<Token> ptr_t;

	Token(	const color_t& c = BLACK,
			const std::string& owner="",
			int rankOwner=0,
			const int torusDim = 3,
			const matrix_t& m1=matrix_t(),
			const matrix_t& m2=matrix_t() )

		: m_col(c)
		, m_owner(owner)
		, m_rankOwner(rankOwner)
		, m_block_1(m1)
		, m_block_2(m2)
	{
	}

	Token(const Token& t)
	{
		m_col  = t.color();
		m_owner = t.owner();
		m_rankOwner = t.rankOwner() ;
		m_block_1 = t.block_1();
		m_block_2 = t.block_2();
		m_activityId = t.activityId();
	}

	Token& operator=(const Token& t)
	{
		if(this!=&t)
		{
			m_col  = t.color();
			m_owner = t.owner();
			m_rankOwner = t.rankOwner() ;
			m_block_1 = t.block_1();
			m_block_2 = t.block_2();
			m_activityId = t.activityId();
		}

		return *this;
	}

	~Token() {}

	template <class Archive>
	void serialize(Archive& ar, const unsigned int)
	{
		ar & color();
		ar & owner();
		ar & rankOwner();
		ar & block_1();
		ar & block_2();
		ar & activityId();
	}

	void decode( const std::string& strInput )
	{
		std::string strOutput;
		std::stringstream sstr(strInput);
		boost::archive::text_iarchive ar(sstr);
		ar >> *this;
	}

	std::string encode()
	{
		std::stringstream sstr;
		boost::archive::text_oarchive ar(sstr);
		ar << *this;
		return sstr.str();
	}

	matrix_t& block_1() { return m_block_1; }
	const matrix_t& block_1() const { return m_block_1; }

	matrix_t& block_2() { return m_block_2; }
	const matrix_t& block_2() const { return m_block_2; }

	std::string& owner() { return m_owner; }
	const std::string& owner() const { return m_owner; }

	int& rankOwner() { return m_rankOwner; }
	const int& rankOwner() const { return m_rankOwner; }

	color_t& color() { return m_col; }
	const color_t& color() const { return m_col; }

	id_type& activityId() { return m_activityId; }
	const id_type& activityId() const { return m_activityId; }

	private:
	color_t  m_col;
	std::string m_owner;
	int m_rankOwner;
	matrix_t m_block_1;
	matrix_t m_block_2;
	id_type  m_activityId;
};


#endif //TOKEN_HPP
