#ifndef _BUFFER_BYTE_H_
#define _BUFFER_BYTE_H_

#include <stdint.h>
#include <vector>
#include <cstring>
#include <assert.h>
#include <sstream>
#include <stdio.h>
#ifdef _MSC_VER
#include <winSock2.h>
#else
#include <arpa/inet.h>
#endif

#define DEFAULT_BUFFER_SIZE 2048
// 大小端
#ifndef BYTE_ORDER_BIG_ENDIAN
#define BYTE_ORDER_BIG_ENDIAN            1
#endif
#ifndef BYTE_ORDER_LITTLE_ENDIAN
#define BYTE_ORDER_LITTLE_ENDIAN         2
#endif

class ByteBuffer {
public:
    // constructor
	ByteBuffer(): m_nPosition(0), m_nLimit(0), m_nCapacity(DEFAULT_BUFFER_SIZE)
    {
        m_nByteOrder = BYTE_ORDER_LITTLE_ENDIAN;
        m_vecBuffer.reserve(DEFAULT_BUFFER_SIZE);
    }

    // constructor
    ByteBuffer(size_t capacity): m_nPosition(0), m_nLimit(0), m_nCapacity(capacity)
    {
        m_nByteOrder = BYTE_ORDER_LITTLE_ENDIAN;
        m_vecBuffer.reserve(capacity);
    }

    // copy constructor
    ByteBuffer(const ByteBuffer& buf) {
        m_nPosition = buf.m_nPosition;
        m_nLimit = buf.m_nLimit;
        m_nCapacity = buf.m_nCapacity;
        m_vecBuffer = buf.m_vecBuffer;
        m_nByteOrder = buf.m_nByteOrder;
    }

    virtual ~ByteBuffer() {}

    void Clear()
    {
        m_vecBuffer.clear();
        m_nPosition = 0;
        m_nLimit = 0;
        m_nCapacity = 0;
    }
    
    void SetByteOrder(uint32_t byteOrder) 
	{
		m_nByteOrder = byteOrder;
	}

    uint32_t ByteOrder() const {
        return m_nByteOrder;
    }

    void SetPosition(uint32_t position) {
        m_nPosition = position;
    }

    uint32_t Position() const
    {
        return m_nPosition;
    }

    void SetLimit(uint32_t limit) {
        m_nLimit = limit;
    }

    uint32_t Limit() const
    {
        return m_nLimit;
    }

    uint32_t Capacity() const
    {
        return m_nCapacity;
    }

    bool HasRemaining()
    {
        return m_nLimit > m_nPosition;
    }

    uint32_t Remaining() const
    {
        return m_nPosition < m_nLimit ? m_nLimit - m_nPosition : 0;
    }

    void Resize(size_t capacity)
    {
        m_vecBuffer.resize(capacity);
        m_nPosition = 0;
        m_nLimit = 0;
        m_nCapacity = capacity;
    }

    const uint8_t * Contents() const { 
        return &m_vecBuffer[0]; 
    }

    // 读取时设置limit和position的值, 写入则不用设置limit和postion的值
    virtual void Compact() = 0;

    // ============================================================
    ByteBuffer& PutByte(char value)
    {
        append<uint8_t>(value);
        return *this;
    }

    ByteBuffer& PutUint16(uint16_t value)
    {
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = htons(value);
        }
        append<uint16_t>(value);
        return *this;
    }

    ByteBuffer& PutUint32(uint32_t value)
    {
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = htonl(value);
        }
        append<uint32_t>(value);
        return *this;
    }

    ByteBuffer& PutUint64(uint64_t value)
    {
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = htonl(value);
        }
        append<uint64_t>(value);
        return *this;
    }

    ByteBuffer& PutFloat(float value)
    {
        append<float>(value);
        return *this;
    }

    ByteBuffer& PutDouble(double value)
    {
        append<double>(value);
        return *this;
    }

    ByteBuffer& PutBytes(const uint8_t* buf, uint32_t len)
    {
        for (uint32_t i = 0; i < len; i++)
            append<uint8_t>(buf[i]);

        return *this;
    }

    ByteBuffer& PutString(const std::string& value)
    {
        append((uint8_t *)value.c_str(), value.length());
        return *this;
    }

    // ====================================================
    void Textlike()
    {
        std::ostringstream ss;
        printf("ByteBuffer size: %d \n", Capacity());
        for (size_t i = 0; i < Capacity(); ++i)
            ss << read<uint8_t>(i);
        printf("ByteBuffer binary: %s \n", ss.str().c_str());
    }

    void Hexlike() {
        std::ostringstream ss;
        printf("ByteBuffer size: %d \n", Capacity());
        size_t j = 1, k = 1;
        for (size_t i = 0; i < Capacity(); ++i) {
            if ((i == (j * 8)) && ((i != (k * 16))))
            {
                ss << "| ";
                ++j;
            }
            else if (i == (k * 16))
            {
                ss << "\n";

                ++k;
                ++j;
            }

            char buf[4];
            snprintf(buf, 4, "%02X", read<uint8_t>(i));
            ss << buf << " ";
        }
        printf("ByteBuffer hex: %s \n", ss.str().c_str());
    }

    // =================================================================
	char GetByte() 
	{
        // 合法性检测
        if (m_nPosition + 1 > m_nLimit)
            return '\0';
		
		char value = m_vecBuffer[m_nPosition++];
		return value;
	}

    bool GetBytes(char* buf, uint32_t len)
    {
        if (m_nPosition + len > m_nLimit)
            return false;

        for (uint32_t i = 0; i < len; i++)
        {
            buf[i] = m_vecBuffer[m_nPosition++];
        }
		return true;
    }

	std::string GetString(uint32_t len)
	{
		if (m_nPosition + len > m_nLimit)
            return "";
		
		uint32_t nSize = len + 1;
		char *buf = new char[nSize];
		memset(buf, 0, nSize);
        for (uint32_t i = 0; i < len; i++)
        {
            buf[i] = m_vecBuffer[m_nPosition++];
        }

		std::string value = std::string(buf);
		if(buf) {
			delete [] buf;
		}
		
		return value;
	}

	uint32_t GetUInt32() 
	{
		uint32_t len = sizeof(int);
        if (m_nPosition +  len > m_nLimit)
            return 0;

		char buf[5] = {0};
		for (uint32_t i = 0; i < len; i++)
        {
            buf[i] = m_vecBuffer[m_nPosition++];
        }

		if(m_nByteOrder == BYTE_ORDER_LITTLE_ENDIAN) {
			return ((buf[3] & 0xff) << 24) | ((buf[2] & 0xff) << 16) | ((buf[1] & 0xff) << 8) | ((buf[0] & 0xff) << 0);	
		} 

		return ((buf[0] & 0xff) << 24) | ((buf[1] & 0xff) << 16) | ((buf[2] & 0xff) << 8) | ((buf[3] & 0xff) << 0);
	}

	uint16_t GetUint16() 
	{
		uint32_t len = sizeof(uint16_t);
        if (m_nPosition +  len > m_nLimit)
            return 0;

		char buf[5] = {0};
		for (uint32_t i = 0; i < len; i++)
        {
            buf[i] = m_vecBuffer[m_nPosition++];
        }

		if(m_nByteOrder == BYTE_ORDER_LITTLE_ENDIAN) {
			return ((buf[1] & 0xff) << 8) | ((buf[0] & 0xff) << 0);	
		} 

		return ((buf[0] & 0xff) << 8) | ((buf[1] & 0xff) << 0);
	}

	float GetFloat()
	{
		uint32_t len = sizeof(float);
		if (m_nPosition + len > m_nLimit)
			return 0.0;

		char buf[5] = {0};
		for (uint32_t i = 0; i < len; i++)
        {
            buf[i] = m_vecBuffer[m_nPosition++];
        }
		
		int32_t value = ((buf[3] & 0xff) << 24) | ((buf[2] & 0xff) << 16) | ((buf[1] & 0xff) << 8) | ((buf[0] & 0xff) << 0);
		return IntBitsToFloat(value);
	}

    float IntBitsToFloat(int i)
	{
		union {
			int i;
			float f;
		}u;
		u.i = i;
		return u.f;
	}
    
    // =============================================================
    int16_t GetInt16() {
       	uint32_t len = sizeof(int16_t);
		if (m_nPosition + len > m_nLimit)
			return 0;

        int16_t value;
        value = read<int16_t>();
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = ntohs(value);
        }
        return value; 
    }

    int32_t GetInt32() {
       	uint32_t len = sizeof(int32_t);
		if (m_nPosition + len > m_nLimit)
			return 0;

        int32_t value;
        value = read<int32_t>();
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = ntohl(value);
        }
        return value; 
    }

    int64_t GetInt64() {
       	uint32_t len = sizeof(int64_t);
		if (m_nPosition + len > m_nLimit)
			return 0;

        int64_t value;
        value = read<int64_t>();
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = ntohl(value);
        }
        return value; 
    }

    uint64_t GetUInt64() {
       	uint32_t len = sizeof(uint64_t);
		if (m_nPosition + len > m_nLimit)
			return 0;

        uint64_t value;
        value = read<uint64_t>();
        if (m_nByteOrder == BYTE_ORDER_BIG_ENDIAN) {
            value = ntohl(value);
        }
        return value; 
    }

    double GetDouble()
	{
		uint32_t len = sizeof(double);
		if (m_nPosition + len > m_nLimit)
			return 0.0;

        double value;
        value = read<double>();
        return value;
	}

private:
    void append(const uint8_t * src, size_t size)
    {
        if (size <= 0) {
            return;
        }

        if (m_nCapacity < m_nLimit + size) {
            m_vecBuffer.resize(m_nLimit + size);
            m_nCapacity = m_nLimit + size;
        }
            
        memcpy(&m_vecBuffer[m_nPosition], src, size);
        m_nPosition += size;
        m_nLimit += size;
    }

    template<typename T>
    void append(T value)
    {
        append((uint8_t *)&value, sizeof(value));
    }

    template <typename T> T read(size_t pos) const
    {
		assert(pos + sizeof(T) <= Capacity());
        T value;
        memcpy((void*)&value, (void*)&m_vecBuffer[pos], sizeof(T));
        return value;
    }
    
    void read(uint8_t *dest, size_t len)
    {
		assert(m_nPosition + len <= Capacity());
        memcpy(dest, &m_vecBuffer[m_nPosition], len);
        m_nPosition += len;
    }

    template <typename T> T read()
    {
        T r = read<T>(m_nPosition);
        m_nPosition += sizeof(T);
        return r;
    }
protected:
    uint32_t  m_nPosition;
	uint32_t  m_nLimit;
    uint32_t  m_nCapacity;

	uint32_t m_nByteOrder;  
    std::vector<uint8_t> m_vecBuffer;
};


#endif // !_BUFFER_BYTE_H_