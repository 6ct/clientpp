#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <dpapi.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include "./Log.h"
#include "./AccountManager.h"
#include "../utils/Base64.h"
#include "../utils/IOUtil.h"

rapidjson::Value Account::dump(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> allocator)
{
  rapidjson::Value output(rapidjson::kObjectType);
  output.AddMember("order", rapidjson::Value(order), allocator);
  output.AddMember("color", rapidjson::Value(color.data(), color.size()), allocator);
  output.AddMember("password", rapidjson::Value(password.data(), password.size()), allocator);
  return output;
}

Account::Account() : order(0), color("#000000"), password("") {}

Account::Account(rapidjson::Value &data)
    : order(data["order"].GetInt()), color(data["color"].GetString(), data["color"].GetStringLength()), password(data["password"].GetString(), data["password"].GetStringLength()) {}

bool AccountManager::encrypt(std::string input, std::string &output)
{
  DATA_BLOB in;
  DATA_BLOB out;

  in.pbData = (BYTE *)input.data();
  in.cbData = input.size();

  if (CryptProtectData(&in, NULL, NULL, NULL, NULL, 0, &out))
  {
    std::string encrypted;
    encrypted.resize(out.cbData);
    memcpy(encrypted.data(), out.pbData, out.cbData);

    output = Base64::Encode(encrypted);

    return true;
  }

  return false;
}

bool AccountManager::decrypt(std::string input, std::string &output)
{
  std::string encrypted = Base64::Decode(input);

  DATA_BLOB in;
  DATA_BLOB out;

  in.pbData = (BYTE *)encrypted.data();
  in.cbData = encrypted.size();

  if (CryptUnprotectData(&in, NULL, NULL, NULL, NULL, 0, &out))
  {
    output.resize(out.cbData);
    memcpy(output.data(), out.pbData, out.cbData);

    return true;
  }

  return false;
}

std::string AccountManager::dump()
{
  rapidjson::Document output(rapidjson::kObjectType);
  rapidjson::Document::AllocatorType allocator = output.GetAllocator();

  for (auto &[name, acc] : data)
    output.AddMember(rapidjson::Value(name.data(), name.size(), allocator), acc.dump(allocator), allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  output.Accept(writer);

  return {buffer.GetString(), buffer.GetSize()};
}

bool AccountManager::save()
{
  return IOUtil::write_file(folder.directory + path, dump());
}

bool AccountManager::load()
{
  std::string read;
  if (IOUtil::read_file(folder.directory + path, read))
  {
    rapidjson::Document document;
    rapidjson::ParseResult ok = document.Parse(read.data(), read.size());

    if (ok)
    {
      for (rapidjson::Value::MemberIterator it = document.MemberBegin(); it != document.MemberEnd(); ++it)
        data[{it->name.GetString(), it->name.GetStringLength()}] = {it->value};
    }
    else
    {
      clog::error << "Error parsing default config: " << GetParseError_En(ok.Code()) << " (" << ok.Offset() << ")" << clog::endl;
    }
  }

  return true;
}

AccountManager::AccountManager(ClientFolder &f) : folder(f) {}