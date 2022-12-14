/*============================================================================
  Copyright 2017-2022 Koichi Murakami

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file License for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#ifndef JSON_PARSER_H_
#define JSON_PARSER_H_

#include <map>
#include <string>
#include <vector>
#include "picojson.h"

namespace kut {

class JsonParser {
public:
  typedef std::vector<bool> barray_t;
  typedef std::vector<int> iarray_t;
  typedef std::vector<long> larray_t;
  typedef std::vector<float> farray_t;
  typedef std::vector<double> darray_t;
  typedef std::vector<std::string> sarray_t;

  static JsonParser* GetJsonParser();
  ~JsonParser() = default;

  JsonParser(const JsonParser&) = delete;
  void operator=(const JsonParser&) = delete;

  bool LoadFile(const std::string& fname,
                const std::string& json_name = "default",
                bool raw_mode = false);

  bool LoadString(const std::string& jstring,
                 const std::string& json_name = "default",
                 bool raw_mode = false);

  bool Contains(const char* key) const;

  bool GetBoolValue(const char* key) const;
  int GetIntValue(const char* key) const;
  long GetLongValue(const char* key) const;
  float GetFloatValue(const char* key) const;
  double GetDoubleValue(const char* key) const;
  std::string GetStringValue(const char* key) const;

  std::size_t GetBoolArray(const char* key, barray_t& barray) const;
  std::size_t GetIntArray(const char* key, iarray_t& iarray) const;
  std::size_t GetLongArray(const char* key, larray_t& larray) const;
  std::size_t GetFloatArray(const char* key, farray_t& farray) const;
  std::size_t GetDoubleArray(const char* key, darray_t& darray) const;
  std::size_t GetStringArray(const char* key, sarray_t& sarray) const;

  void SelectJsonData(const std::string& name);

  void DumpAll() const;

private:
  JsonParser() = default;

  picojson::value data_map_;
  std::map<std::string, picojson::value> json_list;

};

} // end of namespace

#endif
