#pragma once
#include <afx.h>
#include <vector>

class Message
{
public:
    CString id;
    CString myId;
    CString friendId;
    CString content;
    std::vector<CString> files;
    std::vector<CString> images;
    int isSend;
    CTime createdAt;
    int messageType;

    Message();

    Message(const CString& id, const CString& myId, const CString& friendId,
        const CString& content,
        const std::vector<CString>& files,
        const std::vector<CString>& images,
        int isSend,
        const CTime& createdAt,
        int messageType);
};
