#ifndef HTTP_HEADER_HPP
#define HTTP_HEADER_HPP

#include <string>


namespace ews {

struct header
{
  std::string name;
  std::string value;
};

} // namespace ews


#endif // HTTP_HEADER_HPP
