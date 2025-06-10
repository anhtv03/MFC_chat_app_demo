#pragma once
#include <afx.h>
#include <vector>
#include "util.h"
#include "models/json.hpp"

using json = nlohmann::json;

struct FileInfo {
    CString fileName;
    CString id;
    CString url;

    FileInfo(const CString& fn = _T(""), const CString& i = _T(""), const CString& u = _T(""))
        : fileName(fn), id(i), url(u) {
    }
};

class Message
{
public:
    Message() : isSend(0), messageType(0) {}

    Message(
        const CString& id,
        const CString& content,
        const std::vector<FileInfo>& files,
        const std::vector<FileInfo>& images,
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
                if (file.is_object() && file.contains("urlFile") && file["urlFile"].is_string() &&
                    file.contains("FileName") && file["FileName"].is_string() &&
                    file.contains("_id") && file["_id"].is_string()) {
                    CString fileUrl = Utf8ToCString(file["urlFile"].get<std::string>());
                    fileUrl = _T("http://30.30.30.85:8888/api") + fileUrl;
                    CString fileName = Utf8ToCString(file["FileName"].get<std::string>());
                    CString fileId = Utf8ToCString(file["_id"].get<std::string>());
                    msg.files.emplace_back(fileName, fileId, fileUrl);
                }
            }
        }

        if (item.contains("Images") && item["Images"].is_array()) {
            for (const auto& image : item["Images"]) {
                if (image.is_object() && image.contains("urlImage") && image["urlImage"].is_string() &&
                    image.contains("FileName") && image["FileName"].is_string() &&
                    image.contains("_id") && image["_id"].is_string()) {
                    CString imageUrl = Utf8ToCString(image["urlImage"].get<std::string>());
                    imageUrl = _T("http://30.30.30.85:8888/api") + imageUrl;
                    CString fileName = Utf8ToCString(image["FileName"].get<std::string>());
                    CString imageId = Utf8ToCString(image["_id"].get<std::string>());
                    msg.images.emplace_back(fileName, imageId, imageUrl);
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
        CTime yesterday = today - CTimeSpan(1, 0, 0, 0);

        if (createdAt.GetYear() == today.GetYear() &&
            createdAt.GetMonth() == today.GetMonth() &&
            createdAt.GetDay() == today.GetDay())
        {
            return createdAt.Format(_T("Hôm nay"));
        }
        else if (createdAt.GetYear() == yesterday.GetYear() &&
            createdAt.GetMonth() == yesterday.GetMonth() &&
            createdAt.GetDay() == yesterday.GetDay())
        {
            return createdAt.Format(_T("%H:%M hôm qua"));
        }
        else
        {
            return createdAt.Format(_T("%H:%M %d/%m/%Y"));
        }
    }

    CString GetId() const { return id; }
    void SetId(const CString& value) { id = value; }

    CString GetContent() const { return content; }
    void SetContent(const CString& value) { content = value; }

    const std::vector<FileInfo>& GetFiles() const { return files; }
    void SetFiles(const std::vector<FileInfo>& value) { files = value; }

    const std::vector<FileInfo>& GetImages() const { return images; }
    void SetImages(const std::vector<FileInfo>& value) { images = value; }

    int GetIsSend() const { return isSend; }
    void SetIsSend(int value) { isSend = value; }

    CTime GetCreatedAt() const { return createdAt; }
    void SetCreatedAt(const CTime& value) { createdAt = value; }

    int GetMessageType() const { return messageType; }
    void SetMessageType(int value) { messageType = value; }

    CString GetFileName(size_t index) const {
        if (index < files.size()) return files[index].fileName;
        return CString(_T(""));
    }

    CString GetFileUrl(size_t index) const {
        if (index < files.size()) return files[index].url;
        return CString(_T(""));
    }

    CString GetFileId(size_t index) const {
        if (index < files.size()) return files[index].id;
        return CString(_T(""));
    }

private:
    CString id;
    CString content;
    std::vector<FileInfo> files;
    std::vector<FileInfo> images;
    int isSend;
    CTime createdAt;
    int messageType;
};