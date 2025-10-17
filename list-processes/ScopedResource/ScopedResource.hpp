#ifndef SCOPEDRESOURCE_HPP
#define SCOPEDRESOURCE_HPP

#include <dirent.h>

class ScopedResource
{
public:
	explicit ScopedResource(const char* path);
	~ScopedResource();

	ScopedResource(const ScopedResource&) = delete;
	ScopedResource& operator=(const ScopedResource&) = delete;

	ScopedResource(ScopedResource&& other) noexcept;
	ScopedResource& operator=(ScopedResource&& other) noexcept;

	DIR* get() const;
	void reset(DIR* res = nullptr);

private:
	DIR* m_res;
};

#endif // SCOPEDRESOURCE_HPP
