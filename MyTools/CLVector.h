#ifndef __CLVECTOR_H__
#define __CLVECTOR_H__

#include "Character.h"
#include <vector>

template<class T>
class CLVector
{
public:
	CLVector()
	{
		hMutex = NULL;
	}
	~CLVector()
	{
		if (hMutex != NULL)
		{
			ReleaseMutex(hMutex);
			::CloseHandle(hMutex);
			hMutex = NULL;
		}
	}

	typedef typename std::vector<T>::size_type size_type;
	typedef typename std::vector<T>::value_type value_type;
	typedef typename std::vector<T>::reference reference;
	typedef typename std::vector<T>::const_reference const_reference;
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;

	HANDLE& GetMutex()
	{
		if (hMutex != NULL)
			return hMutex;

		hMutex = ::CreateMutexW(NULL, FALSE, NULL);
		return hMutex;
	}

	void Lock()
	{
		WaitForSingleObject(GetMutex(), 3000);
	}

	void UnLock()
	{
		ReleaseMutex(GetMutex());
	}

	// 返回内置
	T& at(UINT uPos)
	{
		Lock();
		T& itr = vlst.at(uPos);
		UnLock();
		return itr;
	}

	// 复制内存
	void assign(iterator _Start, iterator _End)
	{
		Lock();
		vlst.assign(_Start, _End);
		UnLock();
	}

	// 添加
	void push_back(const value_type& _Val)
	{
		Lock();
		vlst.push_back(_Val);
		UnLock();
	}

	// 返回开始
	iterator begin()
	{
		Lock();
		iterator itr = vlst.begin();
		UnLock();
		return itr;
	}

	// 返回结束
	iterator end()
	{
		Lock();
		iterator itr = vlst.end();
		UnLock();
		return itr;
	}

	// 返回大小
	UINT size()
	{
		Lock();
		UINT uSize = vlst.size();
		UnLock();
		return uSize;
	}

	// 清空
	void clear()
	{
		Lock();
		vlst.clear();
		UnLock();
	}

	// 删除
	iterator erase(iterator _Pos)
	{
		Lock();
		iterator itr = vlst.erase(_Pos);
		UnLock();
		return itr;
	}

private:
	std::vector<T> vlst;
	HANDLE hMutex;
};


#endif