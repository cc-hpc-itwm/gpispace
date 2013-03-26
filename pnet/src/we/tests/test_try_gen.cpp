#include <we/type/value.hpp>
#include <we/type/value/show.hpp>
#include <we/type/value/cpp/get.hpp>
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

  triangle (const value::type & v_0)
  {
    {
      const value::type & v_1 (value::get_level ("point", v_0));
      {
        const value::type & v_2 (value::get_level ("a", v_1));
        {
          const value::type & v_3 (value::get_level ("x", v_2));
          this->point.a.x = value::get<double> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("y", v_2));
          this->point.a.y = value::get<double> (v_3);
        }
      }
      {
        const value::type & v_2 (value::get_level ("b", v_1));
        {
          const value::type & v_3 (value::get_level ("x", v_2));
          this->point.b.x = value::get<double> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("y", v_2));
          this->point.b.y = value::get<double> (v_3);
        }
      }
      {
        const value::type & v_2 (value::get_level ("c", v_1));
        {
          const value::type & v_3 (value::get_level ("x", v_2));
          this->point.c.x = value::get<double> (v_3);
        }
        {
          const value::type & v_3 (value::get_level ("y", v_2));
          this->point.c.y = value::get<double> (v_3);
        }
      }
    }
    {
      const value::type & v_1 (value::get_level ("name", v_0));
      this->name = value::get<std::string> (v_1);
    }
  }

  value::type value (void) const
  {
    value::structured_t v_0;

    {
      value::structured_t v_1;

      {
        value::structured_t v_2;

        v_2["x"] = this->point.a.x;
        v_2["y"] = this->point.a.y;

        v_1["a"] = v_2;
      }

      {
        value::structured_t v_2;

        v_2["x"] = this->point.b.x;
        v_2["y"] = this->point.b.y;

        v_1["b"] = v_2;
      }

      {
        value::structured_t v_2;

        v_2["x"] = this->point.c.x;
        v_2["y"] = this->point.c.y;

        v_1["c"] = v_2;
      }

      v_0["point"] = v_1;
    }

    v_0["name"] = this->name;

    return v_0;
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
