#define RAPIDJSON_ASSERT(x)

#include "JsonParser.hpp"
#include "Core/Exception/NotImplementedException.hpp"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace Prog3::Api::Parser;
using namespace Prog3::Core::Model;
using namespace Prog3::Core::Exception;
using namespace rapidjson;
using namespace std;

string JsonParser::convertToApiString(Board &board) {
    throw NotImplementedException();
}

string JsonParser::convertToApiString(Column &column) {
    Document d(kObjectType);

    Value jsonColumn = getJsonValueFromModel(column, d.GetAllocator());
    return jsonValueToString(jsonColumn);
}

rapidjson::Value JsonParser::getJsonValueFromModel(Column const &column, rapidjson::Document::AllocatorType &allocator) {
    Value jsonColumn(kObjectType);
    jsonColumn.AddMember("id", column.getId(), allocator);
    jsonColumn.AddMember("name", Value(column.getName().c_str(), allocator), allocator);
    jsonColumn.AddMember("position", column.getPos(), allocator);

    Value jsonItems(kArrayType);

    for (Item const &item : column.getItems()) {
        Value jsonItem = getJsonValueFromModel(item, allocator);
        jsonItems.PushBack(jsonItem, allocator);
    }

    jsonColumn.AddMember("items", jsonItems, allocator);
    return jsonColumn;
}

rapidjson::Value JsonParser::getJsonValueFromModel(Item const &item, rapidjson::Document::AllocatorType &allocator) {
    Value jsonItem(kObjectType);
    jsonItem.AddMember("id", item.getId(), allocator);
    jsonItem.AddMember("title", Value(item.getTitle().c_str(), allocator), allocator);
    jsonItem.AddMember("position", item.getPos(), allocator);
    jsonItem.AddMember("timestamp", Value(item.getTimestamp().c_str(), allocator), allocator);

    return jsonItem;
}

string JsonParser::jsonValueToString(rapidjson::Value const &json) {
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    json.Accept(writer);
    return buffer.GetString();
}

string JsonParser::convertToApiString(std::vector<Column> &columns) {
    throw NotImplementedException();
}

string JsonParser::convertToApiString(Item &item) {
    string result = EMPTY_JSON;
    Document d(kObjectType);
    Value jsonItem = getJsonValueFromModel(item, d.GetAllocator());
    result = jsonValueToString(jsonItem);
    return result;
}

string JsonParser::convertToApiString(std::vector<Item> &items) {
    string result = EMPTY_JSON;
    Document d(kObjectType);

    Value jsonItems(kArrayType);
    for (Item const &item : items) {
        Value jsonItem = getJsonValueFromModel(item, d.GetAllocator());
        jsonItems.PushBack(jsonItem, d.GetAllocator());
    }
    string s = jsonValueToString(jsonItems);
    return s;
}

std::optional<Column> JsonParser::convertColumnToModel(int columnId, std::string &request) {
    std::optional<Column> resultColumn;
    Document d;
    d.Parse(request.c_str());

    if (true == isValidColumn(d)) {
        string name = d["name"].GetString();
        auto position = d["position"].GetInt64();
        resultColumn = Column(columnId, name, position);
    }

    return resultColumn;
}

bool JsonParser::isValidColumn(rapidjson::Document const &document) {
    bool isValid = true;
    if (document.HasParseError()) {
        isValid = false;
    }
    if (false == document["name"].IsString()) {
        isValid = false;
    }
    if (false == document["position"].IsInt()) {
        isValid = false;
    }
    return isValid;
}

bool JsonParser::isValidItem(rapidjson::Document const &document) {
    bool isValid = true;
    if (document.HasParseError()) {
        isValid = false;
    }
    if (false == document["title"].IsString()) {
        isValid = false;
    }
    if (false == document["position"].IsInt()) {
        isValid = false;
    }
    return isValid;
}

std::optional<Item> JsonParser::convertItemToModel(int itemId, std::string &request) {
    std::optional<Item> resultItem;
    Document d;
    d.Parse(request.c_str());
    if (true == isValidItem(d)) {
        string title = d["title"].GetString();
        auto position = d["position"].GetInt64();
        resultItem = Item(itemId, title, position, "");
    }

    return resultItem;
}
