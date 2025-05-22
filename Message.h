#pragma once
#include <afx.h>
#include <vector>

class Message
{
public:

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