#include "LogEvent.hpp"
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <iterator>
#include "util.hpp"
#include <fhg/util/thread/atomic.hpp>

#include <boost/foreach.hpp>

#include <json_spirit_value.h>
#include <json_spirit_reader_template.h>
#include <json_spirit_writer_template.h>

using namespace fhg::log;

static LogEvent::tstamp_type now()
{
//  struct timeval tv;
//  gettimeofday (&tv, NULL);
//  return tv.tv_sec * 1000 * 1000 + tv.tv_usec;
  return time(NULL);
}

static std::size_t next_id ()
{
  static fhg::thread::atomic<std::size_t> counter;
  return ++counter;
}

static std::string get_hostname_ ()
{
  char buf [4096];
  gethostname (buf, sizeof(buf));
  return buf;
}

static std::string get_hostname ()
{
  static std::string h (get_hostname_ ());
  return h;
}

LogEvent::LogEvent(const severity_type &a_severity
                 , const file_type &a_path
                 , const function_type &a_function
                 , const line_type &a_line
                 , const std::string &a_message)
  : id_ (next_id ())
  , severity_(a_severity)
  , path_(a_path)
  , function_(a_function)
  , line_(a_line)
  , message_(a_message)
  , tstamp_(now())
  , pid_(getpid())
  , tid_(gettid())
  , host_ (get_hostname ())
  , trace_ ()
  , tags_ ()
{}

LogEvent::LogEvent()
  : id_ (next_id ())
  , severity_()
  , path_()
  , function_()
  , line_()
  , message_()
  , tstamp_()
  , pid_()
  , tid_()
  , host_ ()
  , trace_ ()
  , tags_ ()
{
}

LogEvent::LogEvent(const LogEvent &e)
  : id_ (e.id ())
  , severity_(e.severity())
  , path_(e.path())
  , function_(e.function())
  , line_(e.line())
  , message_(e.message())
  , tstamp_(e.tstamp())
  , pid_(e.pid())
  , tid_(e.tid())
  , host_ (e.host ())
  , trace_ (e.trace ())
  , tags_ (e.tags ())
{
}

LogEvent::~LogEvent()
{
}

LogEvent &LogEvent::operator=(const LogEvent &e)
{
  if (this != &e) {
    id_ = e.id ();
    severity_ = e.severity();
    path_ = e.path();
    function_ = e.function();
    line_ = e.line();
    message_ = e.message();
    tstamp_ = e.tstamp();
    pid_ = e.pid();
    tid_ = e.tid();
    host_ = e.host ();
    trace_ = e.trace ();
    tags_ = e.tags ();
  }
  return *this;
}

bool LogEvent::operator==(const LogEvent &e) const
{
  if (this == &e) return true;
  if (
       (id () == e.id ())
    && (path() == e.path())
    && (severity() == e.severity())
    && (function() == e.function())
    && (line() == e.line())
    && (message() == e.message())
    && (tstamp() == e.tstamp())
    && (pid() == e.pid())
    && (tid() == e.tid())
    && (host () == e.host ())
    && (trace () == e.trace ())
    && (tags () == e.tags ())
    )
  {
    return true;
  }
  return false;
}

bool LogEvent::operator<(const LogEvent &rhs) const
{
  return tstamp() < rhs.tstamp();
}

std::ostream & LogEvent::encode (std::ostream &os, int flags) const
{
  finish ();

  json_spirit::Object evt;

  evt.push_back (json_spirit::Pair ("version", 1));
  evt.push_back (json_spirit::Pair ("id", id_));
  evt.push_back (json_spirit::Pair ("level", static_cast<int>(severity_.lvl ())));
  evt.push_back (json_spirit::Pair ("message", message_));
  evt.push_back (json_spirit::Pair ("time", static_cast<uint64_t>(tstamp_)));

  {
    json_spirit::Object location;
    location.push_back (json_spirit::Pair ("path", path_));
    location.push_back (json_spirit::Pair ("function", function_));
    location.push_back (json_spirit::Pair ("line", line_));

    evt.push_back (json_spirit::Pair ("location", location));
  }

  {
    json_spirit::Object process;
    process.push_back (json_spirit::Pair ("pid", pid_));
    process.push_back (json_spirit::Pair ("tid", tid_));
    process.push_back (json_spirit::Pair ("host", host_));

    evt.push_back (json_spirit::Pair ("process", process));
  }

  {
    json_spirit::Array tag;
    std::copy ( tags_.begin ()
              , tags_.end ()
              , std::back_inserter (tag)
              );

    evt.push_back (json_spirit::Pair ("tags", tag));
  }

  {
    json_spirit::Array trace;
    std::copy ( trace_.begin ()
              , trace_.end ()
              , std::back_inserter (trace)
              );

    evt.push_back (json_spirit::Pair ("trace", trace));
  }

  json_spirit::write_stream (json_spirit::Value (evt), os, flags);

  return os;
}

static const json_spirit::Value *get_value (json_spirit::Value const &val, std::string const &name)
{
  if (val.type () == json_spirit::obj_type)
  {
    json_spirit::Object const & obj = val.get_obj ();
    json_spirit::Object::const_iterator it = obj.begin ();
    const json_spirit::Object::const_iterator end = obj.end ();
    for (; it != end ; ++it)
    {
      if (it->name_ == name)
        return &it->value_;
    }
  }

  return 0;
}

template <typename T>
static bool
get_value (json_spirit::Value const &val, std::string const &name, T &r)
{
  const json_spirit::Value *v = get_value (val, "id");
  if (v)
  {
    std::stringstream sstr;
    switch (v->type ())
    {
    case json_spirit::str_type:
      sstr << v->get_str ();
      break;
    case json_spirit::int_type:
      sstr << v->get_int ();
      break;
    case json_spirit::bool_type:
      sstr << v->get_bool ();
      break;
    case json_spirit::real_type:
      sstr << v->get_real ();
      break;
    default:
      throw std::runtime_error
        ("log: could not decode non-terminal value in event");
    }

    sstr >> r;
    return !sstr.bad ();
  }

  return false;
}

namespace detail
{
  static void decode_location (LogEvent &evt, json_spirit::Value const &val, int)
  {
    if (val.type () == json_spirit::obj_type)
    {
      json_spirit::Object const & obj = val.get_obj ();
      json_spirit::Object::const_iterator it = obj.begin ();
      const json_spirit::Object::const_iterator end = obj.end ();
      for (; it != end ; ++it)
      {
        if (it->name_ == "path")
        {
          evt.path () = it->value_.get_str ();
        }
        else if (it->name_ == "function")
        {
          evt.function () = it->value_.get_str ();
        }
        else if (it->name_ == "line")
        {
          evt.line () = it->value_.get_int ();
        }
      }
    }
    else
    {
      throw std::runtime_error ("log: could not decode LogEvent location");
    }
  }

  static void decode_process (LogEvent &evt, json_spirit::Value const &val, int)
  {
    if (val.type () == json_spirit::obj_type)
    {
      json_spirit::Object const & obj = val.get_obj ();
      json_spirit::Object::const_iterator it = obj.begin ();
      const json_spirit::Object::const_iterator end = obj.end ();
      for (; it != end ; ++it)
      {
        if (it->name_ == "pid")
        {
          evt.pid () = it->value_.get_int ();
        }
        else if (it->name_ == "tid")
        {
          evt.tid () = it->value_.get_int ();
        }
        else if (it->name_ == "host")
        {
          evt.host () = it->value_.get_str ();
        }
      }
    }
    else
    {
      throw std::runtime_error ("log: could not decode LogEvent process");
    }
  }

  static void decode_trace (LogEvent &evt, json_spirit::Value const &val, int)
  {
    if (val.type () == json_spirit::array_type)
    {
      json_spirit::Array const & arr = val.get_array ();
      json_spirit::Array::const_iterator it = arr.begin ();
      const json_spirit::Array::const_iterator end = arr.end ();
      for (; it != end ; ++it)
      {
        evt.trace (it->get_str ());
      }
    }
    else
    {
      throw std::runtime_error ("log: could not decode LogEvent trace");
    }
  }

  static void decode_tags (LogEvent &evt, json_spirit::Value const &val, int)
  {
    if (val.type () == json_spirit::array_type)
    {
      json_spirit::Array const & arr = val.get_array ();
      json_spirit::Array::const_iterator it = arr.begin ();
      const json_spirit::Array::const_iterator end = arr.end ();
      for (; it != end ; ++it)
      {
        evt.tag (it->get_str ());
      }
    }
    else
    {
      throw std::runtime_error ("log: could not decode LogEvent tags");
    }
  }

  static void decode (LogEvent &evt, json_spirit::Value const &val)
  {
    int version = 0;

    if (val.type () == json_spirit::obj_type)
    {
      json_spirit::Object const & obj = val.get_obj ();
      json_spirit::Object::const_iterator it = obj.begin ();
      const json_spirit::Object::const_iterator end = obj.end ();
      for (; it != end ; ++it)
      {
        if (it->name_ == "version")
        {
          version = it->value_.get_int ();
        }
        else if (it->name_ == "id")
        {
          evt.id () = it->value_.get_int ();
        }
        else if (it->name_ == "level")
        {
          evt.severity ().lvl () = (LogLevel::Level)it->value_.get_int ();
        }
        else if (it->name_ == "message")
        {
          evt.message () = it->value_.get_str ();
        }
        else if (it->name_ == "time")
        {
          evt.tstamp () = it->value_.get_int ();
        }
        else if (it->name_ == "location")
        {
          decode_location (evt, it->value_, version);
        }
        else if (it->name_ == "process")
        {
          decode_process (evt, it->value_, version);
        }
        else if (it->name_ == "trace")
        {
          decode_trace (evt, it->value_, version);
        }
        else if (it->name_ == "tags")
        {
          decode_tags (evt, it->value_, version);
        }
      }
    }
    else
    {
      throw std::runtime_error ("log: could not decode LogEvent");
    }
  }
}

std::istream & LogEvent::decode (std::istream &is)
{
  json_spirit::Value val;
  if (not json_spirit::read_stream (is, val))
  {
    throw std::runtime_error ("log: could not deserialize LogEvent");
  }

  detail::decode (*this, val);
  return is;
}

std::istream & operator >> (std::istream &is, LogEvent &evt)
{
  try
  {
    evt.decode (is);
  }
  catch (...)
  {
    is.setstate (std::ios::failbit);
  }

  return is;
}

std::ostream & operator << (std::ostream &os, LogEvent const &evt)
{
  return evt.encode (os, 0);
}
