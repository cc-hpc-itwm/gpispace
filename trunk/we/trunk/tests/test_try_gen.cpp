#include <we/type/value.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/get.hpp>
#include <string>
#include <iostream>

// this is the structure we want to generate automatically, including
// the constructor and the method

struct triangle {
  struct {
    struct {
      double x;
      double y;
    } a;
    struct {
      double x;
      double y;
    } b;
    struct {
      double x;
      double y;
    } c;
  } point;
  std::string name;

  triangle () {}

  triangle (const value::type & v)
  {
    {
      value::type point (value::get_field ("point", v));

      {
        value::type a (value::get_field ("a", point));

        this->point.a.x = value::get<double> ("x", a);
        this->point.a.y = value::get<double> ("y", a);
      }

      {
        value::type b (value::get_field ("b", point));

        this->point.b.x = value::get<double> ("x", b);
        this->point.b.y = value::get<double> ("y", b);
      }

      {
        value::type c (value::get_field ("c", point));

        this->point.c.x = value::get<double> ("x", c);
        this->point.c.y = value::get<double> ("y", c);
      }
    }

    this->name = value::get<std::string> ("name", v);
  }

  value::type value (void) const
  {
    value::structured_t triangle;

    {
      value::structured_t point;

      {
        value::structured_t a;

        a["x"] = this->point.a.x;
        a["y"] = this->point.a.y;

        point["a"] = a;
      }

      {
        value::structured_t b;

        b["x"] = this->point.b.x;
        b["y"] = this->point.b.y;

        point["b"] = b;
      }

      {
        value::structured_t c;

        c["x"] = this->point.c.x;
        c["y"] = this->point.c.y;

        point["c"] = c;
      }

      triangle["point"] = point;
    }

    triangle["name"] = this->name;

    return triangle;
  }
};

int main ()
{
  triangle t;

  t.name = "TRIANGLE";
  t.point.a.x = 0.0;
  t.point.a.y = 1.0;
  t.point.b.x = 2.0;
  t.point.b.y = 3.0;
  t.point.c.x = 4.0;
  t.point.c.y = 5.0;

  std::cout << t.value() << std::endl;

  triangle t2 (t.value());

  std::cout << t2.value() << std::endl;

  return EXIT_SUCCESS;
}
