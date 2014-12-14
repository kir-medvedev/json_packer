#ifndef JSONPACKER_H
#define JSONPACKER_H

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <json_spirit.h>

/**
 * @brief Class provides indexing and convertion of JSON objects from UTF-8 text
 * form to a compressed TLV binary. Interface is similar to std::ofstream and
 * uses it as a background. */
class JsonPacker
{
public:
    /**
     * @brief Constructs a JsonPacker instance, which will write output
     * to @a filename.
     * @param filename - name of the output file. */
    JsonPacker(const std::string& filename);

    virtual ~JsonPacker();

    /**
     * @brief Append input string to the JsonPacker.
     * @param jsonString - input string
     * @return Link to the updated JsonPacker object like std::ostream do. */
    JsonPacker& operator<<(const std::string& jsonString);

    /**
     * @brief Flushes data to the disk and closes the output stream. */
    void close();

private:
    typedef uint8_t pack_key_t;      ///< Type of the index key
    typedef uint8_t tlv_size_t;     ///< Type of the size variable in TLV output

    /// Enum with all supported TLV types. Must be size of char.
    enum TLV_TYPE : char {
        JSON_NULL = 0,              // NB: can't use NULL as it's a C macro
        BOOLEAN,
        INTEGER,
        REAL,
        STRING,
        DICTIONARY
    };

private:
    pack_key_t index(const std::string& value);
    std::vector<std::pair<pack_key_t, json_spirit::Value>> indexObject(
            const json_spirit::Object& jsonObject);

    /**
     * @brief Support function which writes a variable in TLV encoding
     * (value_type|value_size|value_data).
     * Have separate <std::string> specialization. */
    template<class T> void writeTlvVar(TLV_TYPE type, const T& value);

    /**
     * @brief Writes {key_id, value} pair in TLV encoding into output stream.
     * Encoding scheme: <key_id><value_type_id><value_size><value_data>,
     * where value_type_id is taken from TLV_TYPE enum.
     * @param key - index of the key in the dictionary.
     * @param value - value associated with this key. */
    void writeTlvPair(pack_key_t key, const json_spirit::Value& value);

    /**
     * @brief Writes a dictionary in TLV encoding into end of the output stream.
     * Encoding scheme: <dictionary_type_id><dict_elem>...<dict_elem><eof>
     * where:
     * - <dictionary_type_id> - a constant from TLV_TYPE enum;
     * - <dict_elem> - an encoded record of the dictionary element using scheme:
     * <key_id><key_type><key_size><key_data>.
     * This function is called in the end of data encoding. */
    void writeDictionary();

private:
    std::ofstream outStream_;
    std::map<std::string, pack_key_t> dict_;
    pack_key_t dictIndex_;
};

#endif // JSONPACKER_H
