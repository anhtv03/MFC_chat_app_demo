#pragma once

#include "Message.h"
#include "models/json.hpp" 
#include <fstream>

using json = nlohmann::json;

class JsonHelper
{
public:
	static std::vector<Message> ReadMessagesFromJsonFile(const CString& filePath, const CString myId, const CString friendId)
	{
		std::vector<Message> result;

		CT2CA pszConvertedAnsiString(filePath);
		std::string filePathStr(pszConvertedAnsiString);

		std::ifstream file(filePathStr);
		if (!file.is_open()) return result;

		json data;
		file >> data;

		for (const auto& item : data)
		{
			/*Message msg;
			msg.id = CString(item["id"].get<std::string>().c_str());
			msg.myId = CString(item["myId"].get<std::string>().c_str());
			msg.friendId = CString(item["friendId"].get<std::string>().c_str());

			if ((msg.myId == myId && msg.friendId == friendId) ||
				(msg.myId == friendId && msg.friendId == myId))
			{
				msg.content = CString(item.value("content", "").c_str());

				for (const auto& f : item["files"])
					msg.files.push_back(CString(f.get<std::string>().c_str()));

				for (const auto& img : item["images"])
					msg.images.push_back(CString(img.get<std::string>().c_str()));

				msg.isSend = item["isSend"].get<int>();

				CString createdAtStr(item["createdAt"].get<std::string>().c_str());
				createdAtStr.Replace(_T("T"), _T(" "));
				COleDateTime oleTime;
				oleTime.ParseDateTime(createdAtStr);
				SYSTEMTIME sysTime;
				oleTime.GetAsSystemTime(sysTime);
				msg.createdAt = CTime(sysTime);

				msg.messageType = item["messageType"].get<int>();

				result.push_back(msg);*/
			//}
		}

		return result;
	}
};
