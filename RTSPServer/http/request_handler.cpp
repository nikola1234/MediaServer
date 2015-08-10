//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mime_types_wl.hpp"
#include "reply_wl.hpp"
#include "request.hpp"

namespace http_wl {
namespace server3 {

request_handler::request_handler(RequestHandleFun requestHandleFun)
  : m_RequestHandleFun(requestHandleFun)
{
}

void request_handler::handle_request(const request& req, reply& rep)
{
  // Decode url to path.
  std::string request_cmd;
  if (!url_decode(req.uri, request_cmd))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  // Request path must be absolute and not contain "..".
  if (request_cmd.empty() || request_cmd[0] != '/'
      || request_cmd.find("..") != std::string::npos 
	  || request_cmd.length() < 2 ) 
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }

  request_cmd = request_cmd.substr(1, request_cmd.length() - 1);

  std::string strReponse;
  m_RequestHandleFun( request_cmd, strReponse );
  if( strReponse.length() == 0 ||
	  strReponse.compare("failed") == 0)
  {
	  rep = reply::stock_reply(reply::not_found);
	  return;
  }

  // Fill out the reply to be sent to the client.
  rep.status = reply::ok;
  char buf[1024*64];
  strcpy(buf, strReponse.c_str());
  rep.content.append(buf, strlen(buf));
  rep.headers.resize(2);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
  rep.headers[1].name = "Content-Type";
  rep.headers[1].value = mime_types::extension_to_type("text");
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}

} // namespace server3
} // namespace http
