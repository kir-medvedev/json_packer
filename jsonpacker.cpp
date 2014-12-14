#include "jsonpacker.h"
#include <fstream>
#include <limits>


using namespace std;
using namespace json_spirit;

template<>
void JsonPacker::writeTlvVar<string>(TLV_TYPE type, const string& value)
{
    assert(value.size() <= numeric_limits<tlv_size_t>::max());
    const tlv_size_t stringSize = value.size();
    outStream_ << type;
    outStream_.write((char*)&stringSize, sizeof(stringSize));
    outStream_.write(value.data(), stringSize);
}

template<class T>
void JsonPacker::writeTlvVar(TLV_TYPE type, const T& value)
{
    const tlv_size_t valueSize = sizeof(value);
    outStream_ << type;
    outStream_.write((char*)&valueSize, sizeof(valueSize));
    outStream_.write((char*)&value, valueSize);
}

JsonPacker::JsonPacker(const string& filename)
{
    dictIndex_ = 0;
    outStream_.open(filename, ios_base::in|ios_base::binary|ios_base::trunc);
}

JsonPacker::~JsonPacker()
{
    close();
}

JsonPacker& JsonPacker::operator<<(const string& jsonString)
{
    if (outStream_.is_open()) {
        Value jsonVal;

        if (read(jsonString, jsonVal) && (jsonVal.type() == obj_type)) {
            const Object& jsonObj = jsonVal.get_obj();
            vector<pair<pack_key_t, Value>> indexed = indexObject(jsonObj);

            outStream_ << '{';
            for (const auto& i : indexed)
                writeTlvPair(i.first, i.second);
            outStream_ << '}';
        } else {
            cerr << "Error reading line '" << jsonString << "'" << endl;
        }
    }

    return *this;
}

void JsonPacker::close()
{
    if (outStream_.is_open()) {
        writeDictionary();
        outStream_.close();
    }
}

JsonPacker::pack_key_t JsonPacker::index(const string& value)
{
    pack_key_t res;
    const auto iter = dict_.find(value);

    if (iter == dict_.cend()) {
        dict_[value] = dictIndex_;
        res = dictIndex_;
        dictIndex_++;
    } else {
        res = iter->second;
    }

    return res;
}

vector<pair<JsonPacker::pack_key_t, Value>> JsonPacker::indexObject(
        const Object& jsonObject)
{
    vector<pair<pack_key_t, Value>> res;
    res.reserve(jsonObject.size());

    // Object is basically a vector<json_spirit::Pair> typedef
    for (const Pair& i : jsonObject) {
        const string& key = i.name_;
        const Value& val = i.value_;
        const pack_key_t idx = index(key);
        res.push_back({idx, val});
    }

    return res;
}

void JsonPacker::writeDictionary()
{
    assert(dict_.size() <= numeric_limits<tlv_size_t>::max());
    const tlv_size_t dictSize = dict_.size();

    outStream_ << TLV_TYPE::DICTIONARY;
    outStream_.write((char*)&dictSize, sizeof(dictSize));

    for (const auto& i : dict_) {
        const pack_key_t& idx = i.second;
        const string& val = i.first;
        assert(val.size() <= numeric_limits<tlv_size_t>::max());
        const tlv_size_t valSize = val.size();
        outStream_.write((char*)&idx, sizeof(idx));  // write index
        outStream_.write((char*)&valSize, sizeof(valSize));
        outStream_.write(val.data(), valSize);
    }
}

void JsonPacker::writeTlvPair(pack_key_t key, const Value& value)
{
    outStream_.write((char*)&key, sizeof(key));
    switch (value.type()) {
    case json_spirit::null_type:
        // write only type in case of null
        outStream_ << TLV_TYPE::JSON_NULL;
        break;
    case json_spirit::bool_type:
        writeTlvVar(TLV_TYPE::BOOLEAN, value.get_bool());
        break;
    case json_spirit::int_type:
        writeTlvVar(TLV_TYPE::INTEGER, value.get_int());
        break;
    case json_spirit::real_type:
        writeTlvVar(TLV_TYPE::REAL, value.get_real());
        break;
    case json_spirit::str_type:
        writeTlvVar(TLV_TYPE::STRING, value.get_str());
        break;
    default:
        // value type not supported
        std::cerr << "Error: value of unsupported type provided!" << endl;
    }
    outStream_.flush();
}

