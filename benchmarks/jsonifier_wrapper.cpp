#include "jsonifier_wrapper.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <jsonifier/hash_map.hpp>
#include "data/fqcn_keys.inc"

namespace {

// Key order matches the hit_keys order in bench_protocol.cpp, so a key's index
// is the value every other method returns for it.
using protocol_map = jsonifier::hash_map<"http", "https", "ftp", "ws", "wss", "file">;

using ticker_map = jsonifier::hash_map<
    "AAPL","ABBV","ABT","ACN","ADBE","AIG","AMD","AMGN","AMT","AMZN",
    "AVGO","AXP","BA","BAC","BK","BKNG","BLK","BMY","C","CAT",
    "CHTR","CL","CMCSA","COF","COP","COST","CRM","CSCO","CVS","CVX",
    "DE","DHR","DIS","DOW","DUK","EMR","EXC","F","FDX","GD",
    "GE","GILD","GM","GOOG","GS","HD","HON","IBM","INTC","INTU",
    "ISRG","JNJ","JPM","KHC","KO","LIN","LLY","LMT","LOW","MA",
    "MCD","MDLZ","MDT","MET","META","MMM","MO","MRK","MS","MSFT",
    "NEE","NFLX","NKE","NVDA","ORCL","PEP","PFE","PG","PM","PYPL",
    "QCOM","RTX","SBUX","SCHW","SO","SPG","T","TGT","TMO","TMUS",
    "TSLA","TXN","UNH","UNP","UPS","USB","V","VZ","WFC","WMT">;

using keyword_map = jsonifier::hash_map<
    "auto","bool","break","case","char","class","const",
    "do","double","else","enum","float","for","goto","if">;

using header_map = jsonifier::hash_map<
    "Accept","Accept-Encoding","Authorization","Cache-Control",
    "Connection","Content-Length","Content-Type","Cookie",
    "Date","Host","If-None-Match","Location","Origin","Referer",
    "Server","Set-Cookie","User-Agent","Vary","Via","X-Forwarded-For">;

using mime_map = jsonifier::hash_map<
    "text/html","text/plain","text/css","application/json",
    "application/xml","application/pdf","application/zip",
    "image/png","image/jpeg","image/gif","image/webp",
    "audio/mpeg","video/mp4","font/woff2","font/woff">;

using letters_map = jsonifier::hash_map<
    "a","b","c","d","e","f","g","h","i","j","k","l","m",
    "n","o","p","q","r","s","t","u","v","w","x","y","z">;

using headers50_map = jsonifier::hash_map<
    "Accept","Accept-Charset","Accept-Encoding","Accept-Language",
    "Accept-Ranges","Age","Allow","Authorization",
    "Cache-Control","Connection","Content-Encoding","Content-Language",
    "Content-Length","Content-Location","Content-Type","Cookie",
    "Date","ETag","Expect","Expires",
    "From","Host","If-Match","If-Modified-Since",
    "If-None-Match","If-Range","If-Unmodified-Since","Keep-Alive",
    "Last-Modified","Location","Max-Forwards","Origin",
    "Pragma","Proxy-Authenticate","Proxy-Authorization","Range",
    "Referer","Retry-After","Server","Set-Cookie",
    "TE","Trailer","Transfer-Encoding","Upgrade",
    "User-Agent","Vary","Via","WWW-Authenticate",
    "Warning","X-Forwarded-For">;

using jsreserved_map = jsonifier::hash_map<
    "await","break","case","catch","class","const","continue","debugger","default","delete",
    "do","else","enum","export","extends","false","finally","for","function","if",
    "import","in","instanceof","new","null","return","super","switch","this","throw",
    "true","try","typeof","var","void","while","with","yield","implements","interface",
    "package","private","protected","public","static">;

using counters_map = jsonifier::hash_map<
    "total_items","successful_operations","failed_operations","warnings","errors",
    "retries","bytes_processed","records_read","records_matched","records_written">;

using fqcn_map = jsonifier::hash_map<FQCN_JSONIFIER_LIST>;

template <typename Map> struct JsonifierKeyset {
  static std::optional<int> lookup(std::string_view key) {
    const uint64_t index = Map::findIndex(key);
    if (index < Map::keyCount) return static_cast<int>(index);
    return std::nullopt;
  }

  static void bench_lookup(const std::vector<std::string_view> &input,
                           std::vector<int> &results) {
    for (size_t i = 0; i < input.size(); i++) {
      const uint64_t index = Map::findIndex(input[i]);
      if (index < Map::keyCount) results[i] = static_cast<int>(index);
    }
  }
};

struct KeysetEntry {
  std::string_view name;
  JsonifierWrapper::LookupFn lookup;
  JsonifierWrapper::BenchFn bench;
};

template <typename Map> constexpr KeysetEntry make_entry(std::string_view name) {
  return {name, &JsonifierKeyset<Map>::lookup, &JsonifierKeyset<Map>::bench_lookup};
}

constexpr std::array<KeysetEntry, 10> keysets = {
    make_entry<protocol_map>("URL Protocols"),
    make_entry<ticker_map>("S&P 100 Tickers"),
    make_entry<keyword_map>("C++ Keywords"),
    make_entry<header_map>("HTTP Headers"),
    make_entry<mime_map>("MIME Types"),
    make_entry<letters_map>("Letters a-z"),
    make_entry<headers50_map>("HTTP Headers 50"),
    make_entry<jsreserved_map>("JavaScript Reserved Words"),
    make_entry<counters_map>("Counters"),
    make_entry<fqcn_map>("Java FQCNs"),
};

} // namespace

JsonifierWrapper::JsonifierWrapper(const std::string &keyset_name) {
  for (const KeysetEntry &entry : keysets) {
    if (entry.name == keyset_name) {
      lookup_ = entry.lookup;
      bench_ = entry.bench;
      return;
    }
  }
  throw std::invalid_argument("JsonifierWrapper: unknown keyset '" + keyset_name + "'");
}
