#pragma once
class CIniReader
{
public:
	CIniReader();
	int ReadInteger(const char* szSection, const char* szKey, int iDefaultValue);
	float ReadFloat(const char* szSection, const char* szKey, float iDefaultValue);
private:
	char m_szFileName[255];
};
