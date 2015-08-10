//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef HTTP_SERVER3_HEADER_HPP_WL
#define HTTP_SERVER3_HEADER_HPP_WL

#include <string>

namespace http_wl {
namespace server3 {

struct header
{
  std::string name;
  std::string value;
};

} // namespace server3
} // namespace http

#endif // HTTP_SERVER3_HEADER_HPP
