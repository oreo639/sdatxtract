#ifndef __SDATXTRACT_MEMFILE_H__
#define __SDATXTRACT_MEMFILE_H__

#include <string.h>
#include <stdlib.h>
#include <memory.h>

class MemFile {
	public:

		MemFile(const std::string& filename, uint32_t length) : m_FileName(filename), m_Length(length) {
			m_Data = new uint8_t[length];
		}

		MemFile(uint32_t length) : m_Length(length) {
			m_Data = new uint8_t[length];
		}

		MemFile(const std::string& filename, uint8_t* ptr, uint32_t length) : m_FileName(filename), m_Data(ptr), m_Length(length) {}

		MemFile(uint8_t* ptr, uint32_t length) : m_Data(ptr), m_Length(length) {}

		~MemFile() {
			delete[] m_Data;
		}

		std::string GetFilename() {
			return m_FileName;
		}

		uint32_t GetSize() {
			return m_Length;
		}

		uint8_t* GetRawPtr() {
			return m_Data;
		}

		uint8_t* GetCurPos() {
			return m_Data+m_Offset;
		}

		long unsigned GetOffset() {
			return m_Offset;
		}

		template<typename T>
		T Read(bool littleEndian = true) {
			if (m_Offset+sizeof(T) > m_Length)
				return 0;

			T a;
			memcpy(&a, m_Data + m_Offset, sizeof(T));
			m_Offset+=sizeof(T);
			return a;
		}

		int32_t ReadFixLen(size_t bytes, bool littleEndian = true, bool isSigned = false)
		{
			int32_t result = 0;

			for (size_t i = 0; i < bytes; ++i)
			{
				result |= *(m_Data+(m_Offset++)) << ((littleEndian ? i : bytes - i - 1) * 8);
			}

			if (isSigned && (result >= (1 << ((bytes * 8) - 1))))
			{
				result -= 1 << (bytes * 8);
			}

			return result;
		}

		int32_t ReadVarLen()
		{
			int32_t result = 0;

			do
			{
				result = (result << 7) | (*GetCurPos() & 0x7F);
			} while (*(m_Data+(m_Offset++)) & 0x80);

			return result;
		}

		template<typename T>
		size_t ReadArray(T* in_buff, size_t size) {
			if (m_Offset+size > m_Length)
				return 0;

			memcpy(static_cast<void*>(in_buff), (void*)(m_Data + m_Offset), size);
			m_Offset+=size;
			return size;
		}

		long unsigned m_Offset;

	private:
		std::string m_FileName = "filename not avaliable";
		uint8_t* m_Data = nullptr;
		uint32_t m_Length;
};

#endif
