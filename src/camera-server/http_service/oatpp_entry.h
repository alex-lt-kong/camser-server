#ifndef CS_OATPP_ENTRY_HPP
#define CS_OATPP_ENTRY_HPP

#include <string>

void initialize_http_service(std::string host, int port);
void stop_http_service();

#endif /* CS_OATPP_ENTRY_HPP */