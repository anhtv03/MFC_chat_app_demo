#include "pch.h"
#include "Message.h"

Message::Message()
    : isSend(0), messageType(0)
{
}

Message::Message(const CString& _id, const CString& _myId, const CString& _friendId,
    const CString& _content,
    const std::vector<CString>& _files,
    const std::vector<CString>& _images,
    int _isSend,
    const CTime& _createdAt,
    int _messageType)
    : id(_id), myId(_myId), friendId(_friendId), content(_content),
    files(_files), images(_images), isSend(_isSend),
    createdAt(_createdAt), messageType(_messageType)
{
}