#ifndef __MYTOOLS_SOCKET_BYTEBUFFER_H__
#define __MYTOOLS_SOCKET_BYTEBUFFER_H__

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#include "ToolsPublic.h"
#include <vector>
#include <map>
#include <list>

class ByteBuffer
{
protected:
	size_t _rpos, _wpos;
	std::vector<BYTE> _storage;

public:
	const static size_t DEFAULT_SIZE = 0x1000;

	ByteBuffer() : _rpos(0), _wpos(0){
		_storage.reserve(DEFAULT_SIZE);
	}

	ByteBuffer(size_t res) : _rpos(0), _wpos(0){
		_storage.reserve(res);
	}

	ByteBuffer(const ByteBuffer &buf) : _rpos(buf._rpos), _wpos(buf._wpos), _storage(buf._storage){}

	~ByteBuffer() = default;

	void clear(){
		_storage.clear();
		_rpos = _wpos = 0;
	}

public:
	__inline size_t size() const { return _storage.size(); };
	const BYTE *contents() const { return &_storage[0]; };
	const BYTE *data() const { return &_storage[_rpos]; };

	// д��
	template<typename T>
	void append(T value){
		append((BYTE *)&value, sizeof(value));
	}

	template<typename T>
	void put(DWORD pos, T value){
		put(pos, (BYTE *)&value, sizeof(value));
	}

	ByteBuffer &operator<<(bool value){
		append<char>((char)value);

		return *this;
	}

	ByteBuffer &operator<<(BYTE value){
		append<BYTE>(value);

		return *this;
	}

	ByteBuffer &operator<<(WORD value){
		append<WORD>(value);

		return *this;
	}

	ByteBuffer &operator<<(DWORD value){
		append<DWORD>(value);

		return *this;
	}

	ByteBuffer &operator<<(unsigned int value){
		append<unsigned int>(value);

		return *this;
	}

	ByteBuffer &operator<<(unsigned __int64 value){
		append<unsigned __int64>(value);

		return *this;
	}

	ByteBuffer &operator<<(char value){
		append<char>(value);

		return *this;
	}

	ByteBuffer &operator<<(short value){
		append<short>(value);

		return *this;
	}

	ByteBuffer &operator<<(int value){
		append<int>(value);

		return *this;
	}

	ByteBuffer &operator<<(__int64 value){
		append<__int64>(value);

		return *this;
	}

	ByteBuffer &operator<<(float value){
		append<float>(value);

		return *this;
	}

	ByteBuffer &operator<<(double value){
		append<double>(value);

		return *this;
	}

	ByteBuffer &operator<<(const std::string &value){
		append((BYTE *)value.c_str(), value.length());
		append((BYTE)0);

		return *this;
	}

	ByteBuffer &operator<<(const char *str){
		append((BYTE *)str, strlen(str));
		append((BYTE)0);

		return *this;
	}

	ByteBuffer &operator<<(const std::wstring &value){
		append((BYTE *)value.c_str(), value.length() * 2);
		append((WORD)0);

		return *this;
	}

	ByteBuffer &operator<<(const wchar_t *str){
		append((BYTE *)str, wcslen(str) * 2);
		append((WORD)0);

		return *this;
	}

	ByteBuffer & operator << (const POINT & pt)
	{
		append<long>(pt.x);
		append<long>(pt.y);

		return *this;
	}

	// ��ȡ
	template<typename T>
	T read(){
		T r = read<T>(_rpos);
		_rpos += sizeof(T);

		return r;
	}

	template<typename T>
	T read(size_t pos) const{
		if (pos + sizeof(T) > size()){
			return (T)0;
		}
		else{
			return *((T *)&_storage[pos]);
		}
	}

	ByteBuffer &operator>>(bool &value){
		value = read<char>() > 0 ? true : false;
		return *this;
	}

	ByteBuffer &operator>>(BYTE &value) {
		value = read<BYTE>();
		return *this;
	}

	ByteBuffer &operator>>(WORD &value) {
		value = read<WORD>();
		return *this;
	}

	ByteBuffer &operator>>(DWORD &value) {
		value = read<DWORD>();
		return *this;
	}

	ByteBuffer &operator>>(unsigned int &value) {
		value = read<unsigned int>();
		return *this;
	}

	ByteBuffer &operator>>(unsigned __int64 &value) {
		value = read<unsigned __int64>();
		return *this;
	}

	ByteBuffer &operator>>(char &value) {
		value = read<char>();
		return *this;
	}

	ByteBuffer &operator>>(short &value) {
		value = read<short>();
		return *this;
	}

	ByteBuffer &operator>>(int &value) {
		value = read<int>();
		return *this;
	}

	ByteBuffer &operator>>(__int64 &value) {
		value = read<__int64>();
		return *this;
	}

	ByteBuffer &operator>>(float &value) {
		value = read<float>();
		return *this;
	}

	ByteBuffer &operator>>(double &value) {
		value = read<double>();
		return *this;
	}

	ByteBuffer &operator>>(std::string &value){
		value.clear();
		for (;;)
		{
			char c = read<char>();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}

	ByteBuffer &operator>>(std::wstring &value){
		value.clear();
		for (;;){
			wchar_t c = read<wchar_t>();
			if (c == 0)
				break;
			value += c;
		}
		return *this;
	}

	ByteBuffer & operator >> (POINT & pt)
	{
		pt.x = read<long>();
		pt.y = read<long>();
		return *this;
	}


	// other operators
	BYTE operator[](size_t pos){
		return read<BYTE>(pos);
	}

	size_t rpos(){
		return _rpos;
	}

	size_t wpos(){
		return _wpos;
	}

	void resize(size_t newsize){
		_storage.resize(newsize * 2);
		_rpos = 0;
		_wpos = size();
	}

	void reserve(size_t ressize){
		if (ressize > size()) _storage.reserve(ressize);
	}



	// ������ĩβ׷������
	void append(const BYTE *src, size_t cnt){
		if (!cnt) return;

		// size() > 10m crash
		//assert(size() < 10000000);

		if (_storage.size() <= _wpos + cnt)
			_storage.resize(_wpos + cnt);
		memcpy(&_storage[_wpos], src, cnt);
		_wpos += cnt;

	}

	void append(const char *src, size_t cnt){
		return append((const BYTE *)src, cnt);
	}

	void append(const std::string &str){
		append((const BYTE *)str.c_str(), str.size() + 1);
	}

	// ��ָ��λ�ÿ�ʼ׷������
	void put(size_t pos, const BYTE *src, size_t cnt){
		//assert(pos + cnt <= size());
		memcpy(&_storage[pos], src, cnt);
	}

	// ��ȡ����
	void read(BYTE *dest, size_t len){
		if (_rpos + len <= size()){
			memcpy(dest, &_storage[_rpos], len);
		}
		else{
			memset(dest, 0, len);
		}
		_rpos += len;
	}

	void read(char *dest, size_t len)
	{
		read((BYTE *)dest, len);
	}


	__inline void reverse()
	{
		std::reverse(_storage.begin(), _storage.end());
	}

	size_t rpos(size_t rpos) {
		_rpos = rpos;
		return _rpos;
	}

	size_t wpos(size_t wpos) {
		_wpos = wpos;
		return _wpos;
	}
};


//////////////////////////////////////////////////////////////////////////
template <typename T>
ByteBuffer &operator<<(ByteBuffer &b, std::vector<T> v)
{
	b << (DWORD)v.size();
	for (std::vector<T>::iterator i = v.begin(); i != v.end(); i++) {
		b << *i;
	}
	return b;
}

template <typename T>
ByteBuffer &operator>>(ByteBuffer &b, std::vector<T> &v){
	DWORD vsize;
	b >> vsize;
	v.clear();

	while (vsize--)
	{
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}

template <typename T>
ByteBuffer &operator<<(ByteBuffer &b, std::list<T> v)
{
	b << (DWORD)v.size();
	for (list<T>::iterator i = v.begin(); i != v.end(); i++) {
		b << *i;
	}
	return b;
}

template <typename T>
ByteBuffer &operator>>(ByteBuffer &b, std::list<T> &v)
{
	DWORD vsize;
	b >> vsize;
	v.clear();
	while (vsize--) {
		T t;
		b >> t;
		v.push_back(t);
	}
	return b;
}


template <typename K, typename V>
ByteBuffer &operator<<(ByteBuffer &b, std::map<K, V> &m)
{
	b << (DWORD)m.size();
	for (map<K, V>::iterator i = m.begin(); i != m.end(); i++) {
		b << i->first << i->second;
	}
	return b;
}

template <typename K, typename V>
ByteBuffer &operator>>(ByteBuffer &b, std::map<K, V> &m)
{
	DWORD msize;
	b >> msize;
	m.clear();
	while (msize--) {
		K k;
		V v;
		b >> k >> v;
		m.insert(make_pair(k, v));
	}
	return b;
}

#endif