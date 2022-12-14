/*============================================================================
  Copyright 2017-2022 Koichi Murakami

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file License for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>
#include "jsonparser.h"

using namespace picojson;

// --------------------------------------------------------------------------
namespace {

// --------------------------------------------------------------------------
void ThrowException(const std::string& message)
{
  std::stringstream ss;
  ss << "[ERROR] " << message << std::endl;
  throw std::runtime_error(ss.str());
}

// --------------------------------------------------------------------------
void Tokenize(const std::string& str, std::vector<std::string>& tokens,
             const char* delimiter = " ", bool del_included = false)
{
  auto pos0 = str.find_first_not_of(delimiter);
  auto pos = str.find_first_of(delimiter, pos0);

  if ( del_included && pos0 == std::string::npos ) {
    for ( std::size_t i = 0; i < str.size() ; i++ ) {
      tokens.push_back(str.substr(i,1));
    }
    return;
  }

  if ( del_included && pos0 != 0 ) {
    for ( std::size_t i = 0; i < pos0 ; i++ ) {
      tokens.push_back(str.substr(i,1));
    }
  }

  while ( pos0 != std::string::npos ) {
    tokens.push_back(str.substr(pos0, pos-pos0));
    pos0 = str.find_first_not_of(delimiter, pos);
    if ( del_included && pos != std::string::npos ) {
      if ( pos0 == std::string::npos ) {
        pos0 = str.size();
      }
      for ( auto i = pos; i < pos0 ; i++ ) {
        tokens.push_back(str.substr(i, 1));
      }
    }
    pos = str.find_first_of(delimiter, pos0);
  }
}

// --------------------------------------------------------------------------
std::string Trim(const std::string& str)
{
  auto str0 = str;

  // replace TAB -> space
  std::size_t pos = 0;
  while ( ( pos = str0.find('\t', pos) ) != std::string::npos ) {
    str0.replace(pos, 1, " ");
  }

  // trimming first/last white space
  auto idx_first = str0.find_first_not_of(" ");
  auto idx_last = str0.find_last_not_of(" ");

  if ( idx_first == std::string::npos ) {
    return "";
  } else {
    return str0.substr(idx_first, idx_last-idx_first+1);
  }
}

 // --------------------------------------------------------------------------
 std::string DoubleQuote(const std::string& str, bool forced = true)
 {
  // convert to double quoted string
  // forced flag for unquoted strings
  auto obj_str = Trim(str);

  if ( forced && obj_str.find_first_of("\'\"") == std::string::npos ) {
    obj_str = "\"" + obj_str + "\"";
  }
  auto idx0 = obj_str.find_first_of('\'');
  auto idx1 = obj_str.find_last_of('\'');
  if ( idx0 != std::string::npos && idx1 != std::string::npos ) {
    obj_str[idx0] = '\"';
    obj_str[idx1] = '\"';
  }
  return obj_str;
}

// --------------------------------------------------------------------------
void RemoveComments(std::string& str)
{
  // remove block comments
  std::size_t pos0, pos1;
  while ( ( pos0 = str.find("/*") ) != std::string::npos ) {
    bool q_unterminated = false;
    if ( pos0 + 3 >= str.size() ) {
      q_unterminated = true;
    } else if ( ( pos1 = str.find("*/", pos0+2) ) == std::string::npos ) {
      q_unterminated = true;
    }
    if ( q_unterminated ) {
      auto pos_eol = str.find('\n', pos0);
      std::stringstream message;
      message << "jsonparser : unterminated comment" << std::endl
              << str.substr(pos0, pos_eol-pos0+1);
      ThrowException(message.str());
    }

    std::string null_str {""};
    str.replace(pos0, pos1-pos0+2, null_str);
  }

  // remove line comments
  std::stringstream iss, oss;
  std::string linebuf {""};
  iss << str;
  while ( !iss.eof() ) {
    std::getline(iss, linebuf);
    auto idx = linebuf.find("//");
    std::string str_wo_comment {""};
    if ( idx > 0 ) str_wo_comment = linebuf.substr(0, idx);
    auto idx_last = str_wo_comment.find_last_not_of(" \t");
    oss << str_wo_comment.substr(0, idx_last+1);
  }

  str = oss.str();
  str = Trim(str);
}

// --------------------------------------------------------------------------
void ConvertToJson(std::string& str)
{
  RemoveComments(str);

  std::vector<std::string> token_vec;
  Tokenize(str, token_vec, ",");

  std::vector<std::string> subtoken_vec, obj_vec;
  std::string obj_str {""}, value_str {""};
  std::stringstream ss;

  bool has_value {false};
  bool write_comma {false};
  bool is_first_obj {true};
  int depth {0};

  for ( auto& item : token_vec ) {
    subtoken_vec.clear();
    Tokenize(item, subtoken_vec, "{}", true);
    int nloop {0};
    for (auto& item2 : subtoken_vec) {
      item2 = Trim(item2);
      if ( item2 == "" ) continue;
      nloop++;
      if ( item2 == "{" ) {
        depth++;
        if ( !is_first_obj && nloop == 1 ) ss << ", {";
        else ss << "{";
        write_comma = false;
        continue;
      } else if ( item2 == "}" ) {
        depth--;
        ss << "}";
        continue;
      }
      if ( item2 == "]") {
        write_comma = false;
      }

      if ( write_comma ) ss << ",";

      if ( item2.find(':') == std::string::npos ) {
        std::string value_only_str = DoubleQuote(item2, false);
        if ( value_only_str[0] == '+' ) value_only_str.replace(0, 1, "");
        if ( value_only_str[0] == '.' ) value_only_str.replace(0, 1, "0.");
        ss << value_only_str;
        write_comma = true;
        continue;
      }

      // parse string:value
      obj_vec.clear();
      Tokenize(item2, obj_vec, ":", true);
      auto nvec = obj_vec.size();
      if  ( nvec == 0 ) {
        std::stringstream message;
        message << "jsonparser : unexpected error (no token)";
        ThrowException(message.str());
      }
      obj_str = DoubleQuote(obj_vec[0]);
      if ( nvec == 1 ) {
        has_value = false;
      } else if ( nvec == 2 ) {
        value_str = DoubleQuote(obj_vec[1], false);
        has_value = true;
      } else {
        value_str = "";
        for ( auto it = begin(obj_vec)+1; it != end(obj_vec); ++it ) {
          value_str += DoubleQuote(*it, false);
          has_value = true;
        }
      }

      if ( ! has_value ) {
        ss << obj_str;
      } else {
        if ( value_str[0] == '+' ) value_str.replace(0, 1, "");
        if ( value_str[0] == '.' ) value_str.replace(0, 1, "0.");

        if ( value_str[0] == ':' ) {
          if ( value_str[1] == '+' ) value_str.replace(1, 1, "");
          if ( value_str[1] == '.' ) value_str.replace(1, 1, "0.");
        }
        ss << obj_str << value_str;
        write_comma = true;
      }
    }  // tokenize with "{}"
    is_first_obj = false;
  }

  if ( depth != 0 ) {
    std::stringstream message;
    message << "jsonparser : unterminated brackets";
    ThrowException(message.str());
  }

  str = ss.str();
}

// --------------------------------------------------------------------------
value SearchKeyValue(const char* key, const object& obj, bool& is_found)
{
  std::vector<std::string> key_vec;
  Tokenize(key, key_vec, "/");
  picojson::value aval;
  picojson::object aobj = obj;

  for ( const auto& akey : key_vec ) {
    try {
      aval = aobj.at(akey);
    } catch ( std::out_of_range& ) {
      is_found = false;
      return picojson::value();
    }
    if ( aval.is<picojson::object>() ) {
      aobj = aval.get<picojson::object>();
    } else {
      break;
    }
  }

  is_found = true;
  return aval;
}

// --------------------------------------------------------------------------
void DumpObject(const picojson::object& obj, int level)
{
  if (level != 0 ) std::cout << std::endl;
  level++;
  for ( auto it = obj.begin(); it != obj.end(); ++it ) {
    for ( auto i = 0; i < level-1; i++ ) std::cout << "  ";
    std::cout << it->first << " : ";
    if ( it-> second.to_str() == "object") {
      DumpObject(it->second.get<picojson::object>(),level);
    } else {
      std::cout << it->second << std::endl;
    }
  }
}

} // end of namespace

namespace kut {

// --------------------------------------------------------------------------
JsonParser* JsonParser::GetJsonParser()
{
  static JsonParser the_jsonparser;
  return &the_jsonparser;
}

// --------------------------------------------------------------------------
bool JsonParser::LoadFile(const std::string& fname,
                          const std::string& json_name,
                          bool raw_mode)
{
  std::fstream fs;
  fs.open(fname.c_str());
  if ( ! fs.is_open() ) {
    std::cout << "[WARNING] JsonParser::LoadFile() cannot open file. "
              << fname << std::endl;
    return false;
  }

  // read file
  std::stringstream ss;
  std::string linebuf {""};
  while ( !fs.eof() ) {
    std::getline(fs, linebuf);
    ss << linebuf << std::endl;
  }
  fs.close();

  auto qok = LoadString(ss.str(), json_name, raw_mode);
  return qok;
}

// --------------------------------------------------------------------------
bool JsonParser::LoadString(const std::string& jstring,
                            const std::string& json_name,
                            bool raw_mode)
{
  auto json_string = jstring;

  if ( ! raw_mode ) {
    try {
      ::ConvertToJson(json_string);
    } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }

#ifdef DEBUG
    std::cout << "*** JSON string:" << std::endl
              << json_string << std::endl << std::endl;
#endif

  picojson::value json_data;
  std::string error = picojson::parse(json_data, json_string);
  if ( ! error.empty() ) {
    std::cerr << "[ERROR] JsonParser::LoadString() error in JSON parsing. "
    << std::endl << error << std::endl;
    return false;
  }

  json_list[json_name] = json_data;

  if ( json_name == "default") {
    data_map_ = json_data;
  }

  return true;
}

// --------------------------------------------------------------------------
bool JsonParser::Contains(const char* key) const
{
  bool is_found {false};
  const auto& obj = data_map_.get<picojson::object>();

  ::SearchKeyValue(key, obj, is_found);
  return is_found;
}

// --------------------------------------------------------------------------
bool JsonParser::GetBoolValue(const char* key) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetBoolValue() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<bool>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetBoolValue() not a double/int." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  return val.get<bool>();
}

// --------------------------------------------------------------------------
int JsonParser::GetIntValue(const char* key) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetIntValue() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<double>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetIntValue() not a double/int." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  auto int_value = static_cast<int>(val.get<double>());
  return int_value;
}

// --------------------------------------------------------------------------
long JsonParser::GetLongValue(const char* key) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetLongValue() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<double>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetLongValue() not a double/int." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  auto long_value = static_cast<long>(val.get<double>());
  return long_value;
}

// --------------------------------------------------------------------------
float JsonParser::GetFloatValue(const char* key) const
{
  return static_cast<float>(GetDoubleValue(key));
}

// --------------------------------------------------------------------------
double JsonParser::GetDoubleValue(const char* key) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetDoubleValue() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<double>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetDoubleValue() not a double/int." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  return val.get<double>();
}

// --------------------------------------------------------------------------
std::string JsonParser::GetStringValue(const char* key) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetStringValue() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<std::string>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetStringValue() not a string." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  return val.get<std::string>();
}

// --------------------------------------------------------------------------
std::size_t JsonParser::GetBoolArray(const char* key, barray_t& barray) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetBoolArray() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<picojson::array>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetBoolArray() not an array." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  const auto& array = val.get<picojson::array>();
  barray.clear();

  auto size = array.size();
  for ( auto& item : array ) {
    if ( ! item.is<bool>() ) {
      std::cout << "JsonParser::GetBoolArray() not a bool. "
                << key << std::endl;
      return 0;
    }
    barray.push_back(item.get<bool>());
  }

  return size;
}

// --------------------------------------------------------------------------
std::size_t JsonParser::GetIntArray(const char* key, iarray_t& iarray) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetIntArray() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<picojson::array>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetIntArray() not an array." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  const auto& array = val.get<picojson::array>();
  iarray.clear();

  auto size = array.size();
  for ( auto& item : array ) {
    if ( ! item.is<double>() ) {
      std::cout << "JsonParser::GetIntArray() not a double/int. "
                << key << std::endl;
      return 0;
    }
    iarray.push_back(static_cast<int>(item.get<double>()));
  }

  return size;
}

// --------------------------------------------------------------------------
std::size_t JsonParser::GetLongArray(const char* key, larray_t& larray) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetLongArray() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<picojson::array>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetLongArray() not an array." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  const auto& array = val.get<picojson::array>();
  larray.clear();

  auto size = array.size();
  for ( auto& item : array ) {
    if ( ! item.is<double>() ) {
      std::cout << "JsonParser::GetLongArray() not a double/long. "
                << key << std::endl;
      return 0;
    }
    larray.push_back(static_cast<long>(item.get<double>()));
  }

  return size;
}

// --------------------------------------------------------------------------
std::size_t JsonParser::GetFloatArray(const char* key, farray_t& farray) const
{
  darray_t darray;
  auto size = GetDoubleArray(key, darray);

  for ( auto it : darray ) {
    farray.push_back(static_cast<float>(it));
  }

  return size;
}

// --------------------------------------------------------------------------
std::size_t JsonParser::GetDoubleArray(const char* key, darray_t& darray) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetDoubleArray() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<picojson::array>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetDoubleArray() not an array." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  const auto& array = val.get<picojson::array>();
  darray.clear();

  auto size = array.size();
  for ( auto& item : array ) {
    if ( ! item.is<double>() ) {
      std::cout << "JsonParser::GetDoubleArray() not a double/int. "
                << key << std::endl;
      return 0;
    }
    darray.push_back(item.get<double>());
  }

  return size;
}

// --------------------------------------------------------------------------
std::size_t JsonParser::GetStringArray(const char* key, sarray_t& sarray) const
{
  const auto& obj = data_map_.get<picojson::object>();
  bool is_found {false};
  auto val = ::SearchKeyValue(key, obj, is_found);

  if ( ! is_found ) {
    std::stringstream ss;
    ss << "JsonParser::GetStringArray() key is not found." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  if ( ! val.is<picojson::array>() ) {
    std::stringstream ss;
    ss << "JsonParser::GetStringArray() not an array." << std::endl
       << key;
    ::ThrowException(ss.str());
  }

  const auto& array = val.get<picojson::array>();
  sarray.clear();

  auto size = array.size();
  for ( auto& item : array ) {
    if ( ! item.is<std::string>() ) {
      std::cout << "JsonParser::GetStringArray() not a string. "
                << key << std::endl;
      return 0;
    }
    sarray.push_back(item.get<std::string>());
  }

  return size;
}

// --------------------------------------------------------------------------
void JsonParser::SelectJsonData(const std::string& name)
{
  auto it = json_list.find(name);
  if ( it != json_list.end() ) {
    data_map_ = json_list[name];
  } else {
    std::stringstream ss;
    ss << "JsonParser::SetJsonData() JSON data not found. " << name << std::endl;
    ::ThrowException(ss.str());
  }
}

// --------------------------------------------------------------------------
void JsonParser::DumpAll() const
{
  const auto& obj = data_map_.get<picojson::object>();
  ::DumpObject(obj, 0);
}

} // end of namespace
