// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <string>

std::string base64_encode(const char*);
std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);