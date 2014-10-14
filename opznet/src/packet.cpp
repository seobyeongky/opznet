#include "common.h"
#include <opznet/shared_structs.h>
#include <cstring>

namespace opznet
{
	
////////////////////////////////////////////////////////////
Packet::Packet() :
m_readPos(0),
m_isValid(true)
{

}

////////////////////////////////////////////////////////////
Packet::~Packet()
{

}


////////////////////////////////////////////////////////////
void Packet::Append(const void* data, std::size_t byte_size)
{
    if (data && (byte_size > 0))
    {
        std::size_t start = m_data.size();
        m_data.resize(start + byte_size);
        std::memcpy(&m_data[start], data, byte_size);
    }
}


////////////////////////////////////////////////////////////
void Packet::Clear()
{
    m_data.clear();
    m_readPos = 0;
    m_isValid = true;
}


////////////////////////////////////////////////////////////
const void* Packet::GetData() const
{
    return !m_data.empty() ? &m_data[0] : NULL;
}


////////////////////////////////////////////////////////////
std::size_t Packet::GetByteSize() const
{
    return m_data.size();
}

////////////////////////////////////////////////////////////
void Packet::Extract(void * dest, size_t byte_size)
{
	memcpy(dest, &m_data[m_readPos], byte_size);
	m_readPos += byte_size;
}

////////////////////////////////////////////////////////////
bool Packet::EndOfPacket() const
{
    return m_readPos >= m_data.size();
}


////////////////////////////////////////////////////////////
Packet::operator BoolType() const
{
    return m_isValid ? &Packet::checkSize : NULL;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(bool& data)
{
    Uint8 value;
    if (*this >> value)
        data = (value != 0);

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(Int8& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const Int8*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(Uint8& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const Uint8*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(Int16& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohs(*reinterpret_cast<const Int16*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(Uint16& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohs(*reinterpret_cast<const Uint16*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(Int32& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohl(*reinterpret_cast<const Int32*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(Uint32& data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohl(*reinterpret_cast<const Uint32*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
    }

    return *this;
}

////////////////////////////////////////////////////////////
Packet& Packet::operator >>(char& data)
{
	if(checkSize(sizeof(data)))
	{
		data = *reinterpret_cast<const char*>(&m_data[m_readPos]);
		m_readPos += sizeof(data);
	}

	return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(wchar_t&  data)
{
    if (checkSize(sizeof(data)))
    {
        data = ntohs(*reinterpret_cast<const wchar_t*>(&m_data[m_readPos]));
        m_readPos += sizeof(data);
    }

    return *this;
}

////////////////////////////////////////////////////////////
Packet& Packet::operator >>(float& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const float*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(double& data)
{
    if (checkSize(sizeof(data)))
    {
        data = *reinterpret_cast<const double*>(&m_data[m_readPos]);
        m_readPos += sizeof(data);
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(char* data)
{
    // First extract string length
    Uint32 length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        std::memcpy(data, &m_data[m_readPos], length);
        data[length] = '\0';

        // Update reading position
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(std::string& data)
{
    // First extract string length
    Uint32 length = 0;
    *this >> length;

    data.clear();
    if ((length > 0) && checkSize(length))
    {
        // Then extract characters
        data.assign(&m_data[m_readPos], length);

        // Update reading position
        m_readPos += length;
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(wchar_t* data)
{
    // First extract string length
    Uint32 length = 0;
    *this >> length;

    if ((length > 0) && checkSize(length * sizeof(wchar_t)))
    {
        // Then extract characters
        for (Uint32 i = 0; i < length; ++i)
        {
            *this >> data[i];
        }
        data[length] = L'\0';
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator >>(std::wstring& data)
{
    // First extract string length
    Uint32 length = 0;
    *this >> length;

    data.clear();
	data.reserve(length);
    if ((length > 0) && checkSize(length * sizeof(wchar_t)))
    {
        // Then extract characters
        for (Uint32 i = 0; i < length; ++i)
        {
            wchar_t character = 0;
            *this >> character;
			data.push_back(character);
		}
    }

    return *this;
}

////////////////////////////////////////////////////////////
Packet& Packet::operator <<(bool data)
{
    *this << static_cast<Uint8>(data);
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(Int8 data)
{
    Append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(Uint8 data)
{
    Append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(Int16 data)
{
    Int16 toWrite = htons(data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(Uint16 data)
{
    Uint16 toWrite = htons(data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(Int32 data)
{
    Int32 toWrite = htonl(data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(Uint32 data)
{
    Uint32 toWrite = htonl(data);
    Append(&toWrite, sizeof(toWrite));
    return *this;
}

////////////////////////////////////////////////////////////
Packet& Packet::operator <<(char data)
{
	Append(&data, sizeof(data));
	return *this;
}

////////////////////////////////////////////////////////////
Packet& Packet::operator <<(wchar_t data)
{
	wchar_t toWrite = htons(data);
	Append(&toWrite, sizeof(toWrite));
	return *this;
}

////////////////////////////////////////////////////////////
Packet& Packet::operator <<(float data)
{
    Append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(double data)
{
    Append(&data, sizeof(data));
    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(const char* data)
{
    // First insert string length
    Uint32 length = 0;
    for (const char* c = data; *c != '\0'; ++c)
        ++length;
    *this << length;

    // Then insert characters
    Append(data, length * sizeof(char));

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(const std::string& data)
{
    // First insert string length
    Uint32 length = static_cast<Uint32>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        Append(data.c_str(), length * sizeof(std::string::value_type));
    }

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(const wchar_t* data)
{
    // First insert string length
    Uint32 length = 0;
    for (const wchar_t* c = data; *c != L'\0'; ++c)
        ++length;
    *this << length;

    // Then insert characters
    for (const wchar_t* c = data; *c != L'\0'; ++c)
        *this << *c;

    return *this;
}


////////////////////////////////////////////////////////////
Packet& Packet::operator <<(const std::wstring& data)
{
    // First insert string length
    Uint32 length = static_cast<Uint32>(data.size());
    *this << length;

    // Then insert characters
    if (length > 0)
    {
        for (std::wstring::const_iterator c = data.begin(); c != data.end(); ++c)
            *this << *c;
    }

    return *this;
}


////////////////////////////////////////////////////////////
bool Packet::checkSize(std::size_t size)
{
    m_isValid = m_isValid && (m_readPos + size <= m_data.size());

    return m_isValid;
}

} // namespace