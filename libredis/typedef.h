
#ifndef _REDIS_TYPEDEF_H_
#define _REDIS_TYPEDEF_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <stdint.h>

typedef long long redis_integer_t;
typedef std::vector<std::string> redis_array_t;
typedef std::set<std::string> redis_set_t;
typedef std::string string_t;
typedef std::map<string_t, string_t> redis_map_t;
typedef std::map<int64_t, string_t> redis_map_int_str_t;
typedef std::map<int64_t, string_t, std::greater<int64_t> > redis_desc_map_int_str_t;
typedef redis_desc_map_int_str_t::iterator redis_desc_map_int_str_itor;

#define C_STR(str) str.c_str()
#define REPLY_STATUS_OK(s) (s && (s[0] == 'O') && (s[1] == 'K'))

#endif // !_REDIS_TYPEDEF_H_