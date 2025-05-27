#pragma once
#include <afx.h>
#include <vector>
#include "util.h"
#include "models/json.hpp"

using json = nlohmann::json;

class Message
{
public:
    Message() : isSend(0), messageType(0) {}

    Message(
        const CString& id,
        const CString& content,
        const std::vector<CString>& files,
        const std::vector<CString>& images,
        int isSend,
        const CTime& createdAt,
        int messageType)
        : id(id)
        , content(content)
        , files(files)
        , images(images)
        , isSend(isSend)
        , createdAt(createdAt)
        , messageType(messageType)
    {
    }

    static Message FromJson(const json& item) {
        Message msg;
        msg.id = item.contains("id") ? Utf8ToCString(item["id"].get<std::string>()) : CString(_T(""));
		msg.content = item.contains("Content") ? Utf8ToCString(item["Content"].get<std::string>()) : CString(_T(""));
        msg.isSend = item.contains("isSend") ? item["isSend"].get<int>() : 0;
        msg.messageType = item.contains("MessageType") ? item["MessageType"].get<int>() : 0;

        if (item.contains("Files") && item["Files"].is_array()) {
            for (const auto& file : item["Files"]) {
                if (file.is_string()) {
                    msg.files.push_back(Utf8ToCString(file.get<std::string>()));
                }
            }
        }

        if (item.contains("Images") && item["Images"].is_array()) {
            for (const auto& image : item["Images"]) {
                if (image.is_string()) {
                    msg.images.push_back(Utf8ToCString(image.get<std::string>()));
                }
            }
        }

        if (item.contains("CreatedAt") && item["CreatedAt"].is_string()) {
            CString isoTime = Utf8ToCString(item["CreatedAt"].get<std::string>());
            if (isoTime.GetLength() >= 19) {
                int year, month, day, hour, minute, second;
                _stscanf_s(isoTime, _T("%d-%d-%dT%d:%d:%d"),
                    &year, &month, &day, &hour, &minute, &second);
                msg.createdAt = CTime(year, month, day, hour, minute, second);
            }
        }

        return msg;
    }

    CString GetFormattedTime() const {
        CTime today = CTime::GetCurrentTime();
        if (createdAt.GetYear() == today.GetYear() &&
            createdAt.GetMonth() == today.GetMonth() &&
            createdAt.GetDay() == today.GetDay())
        {
            return _T("Hôm nay ") + createdAt.Format(_T("%H:%M"));
        }
        return createdAt.Format(_T("%d/%m/%Y %H:%M"));
    }

    CString GetId() const { return id; }
    void SetId(const CString& value) { id = value; }

    CString GetContent() const { return content; }
    void SetContent(const CString& value) { content = value; }

    const std::vector<CString>& GetFiles() const { return files; }
    void SetFiles(const std::vector<CString>& value) { files = value; }

    const std::vector<CString>& GetImages() const { return images; }
    void SetImages(const std::vector<CString>& value) { images = value; }

    int GetIsSend() const { return isSend; }
    void SetIsSend(int value) { isSend = value; }

    CTime GetCreatedAt() const { return createdAt; }
    void SetCreatedAt(const CTime& value) { createdAt = value; }

    int GetMessageType() const { return messageType; }
    void SetMessageType(int value) { messageType = value; }

private:
    CString id;
    CString content;
    std::vector<CString> files;
    std::vector<CString> images;
    int isSend;
    CTime createdAt;
    int messageType;
};