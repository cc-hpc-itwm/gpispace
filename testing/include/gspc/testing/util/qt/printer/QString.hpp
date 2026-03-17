#pragma once

#include <gspc/testing/printer/generic.hpp>

#include <QtCore/QString>

GSPC_BOOST_TEST_LOG_VALUE_PRINTER (QString, os, val)
{
  os << val.toStdString();
}
