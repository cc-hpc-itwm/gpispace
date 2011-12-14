/*
 * =====================================================================================
 *
 *       Filename:  test.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/02/2010 05:30:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <iostream>
#include "timer.hpp"

struct eced
{
  virtual void filter(std::size_t num_iter) = 0;
};

struct A : eced
{
  // public interface
  void filter(std::size_t num_iter)
  {
//	std::cout << "A::filter()" << std::endl;
	pre_work();
	work(num_iter);
	post_work();
  }

protected:
  // can be overridded
  virtual void pre_work() { }
  virtual void work(const std::size_t num_iter) = 0;
  virtual void post_work() { }

  // disallow direct instantiation
  A() {}
  A(A const &) {}
};

struct B : public A
{
  void pre_work() { }

  void work(const std::size_t num_iter)
  {
	for (std::size_t i (0); i < num_iter; ++i)
	{
	  do_this();
	  do_that();
	}
  }

  // specialized algorithm version 
  void do_this() { }
  void do_that() { }
};

struct C : public A
{
  void post_work() { }

  void work(const std::size_t num_iter)
  {
	for (std::size_t i (0); i < num_iter; ++i)
	{
	  do_that();
	  do_this();
	}
  }

  void do_this() { }
  void do_that() { }
};

struct D1 : eced
{
  // public interface
  void filter(std::size_t num_iter)
  {
	pre_work();
	work(num_iter);
	post_work();
  }

  void pre_work() { }
  void post_work() { }
  void work(const std::size_t num_iter)
  {
	for (std::size_t i (0); i < num_iter; ++i)
	{
	  do_this();
	  do_that();
	}
  }

  void do_this() { }
  void do_that() { }
};

struct D2 : eced
{
  // public interface
  void filter(std::size_t num_iter)
  {
	pre_work();
	work(num_iter);
	post_work();
  }

  void pre_work() { }
  void post_work() { }
  void work(const std::size_t num_iter)
  {
	for (std::size_t i (0); i < num_iter; ++i)
	{
	  do_this();
	  do_that();
	}
  }

  void do_this() { }
  void do_that() { }
};

struct D : eced
{
  explicit
  D(int v)
  {
	if ( (v % 2) == 0) pimpl = new D1();
	else               pimpl = new D2();
  }

  ~D() { delete pimpl; }

  void filter(const std::size_t iter)
  {
	pimpl->filter(iter);
  }

  private:
	eced * pimpl;
};

// factory method
static eced* createAlgorithm(int variant)
{
  if (0 == variant) return new B();
  if (1 == variant) return new C();
  if (2 == variant) return new D(variant);
  if (3 == variant) return new D(variant);
}


int main()
{
  for (int variant(0); variant < 4; ++variant)
  {
	Timer_t timer("t");
	eced * a = createAlgorithm(variant);
	for (std::size_t i (0); i < 1000; ++i) a->filter(1000000);
	delete a;
  }

  return 0;
}
