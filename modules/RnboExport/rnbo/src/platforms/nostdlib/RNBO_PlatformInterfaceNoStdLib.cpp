//
//  PlatformInterfaceNoStdLib.cpp
//
//  Created by Jeremy Bernstein on July 12, 2016
//
//

#ifdef RNBO_NOSTDLIB
#if 0
void	*memcpy(void *, const void *, size_t);
void	*memmove(void *, const void *, size_t);
void	*memset(void *, int, size_t);

char	*strcat(char *, const char *);
int		 strcmp(const char *, const char *);
size_t	 strlen(const char *);
char	*strcpy(char *, const char *);
char	*strchr(const char *, int);

int		 printf(const char * __restrict, ...);
int		 sprintf(char * __restrict, const char * __restrict, ...);
int		 snprintf(char * __restrict, size_t, const char * __restrict, ...);
#endif

#ifndef ARM_CORTEX
void* operator new (size_t size) throw()
{
	void *p = malloc(size);
	return p;
}

void *operator new[](size_t size) throw()
{
	void *p = malloc(size);
	return p;
}

void operator delete (void *p) throw()
{
	free(p);
}

void operator delete[](void *p) throw()
{
	free(p);
}

void __cxa_pure_virtual()
{
	while(1);
}
#else
void * operator new(size_t size) throw() { return ::pvPortMalloc(size); }
void * operator new(size_t, void * p) throw() { return p ; }
void * operator new[](size_t size) throw() { return ::pvPortMalloc(size); }
void operator delete(void* ptr) throw() { ::vPortFree(ptr); }
void operator delete[](void * ptr) throw() { ::vPortFree(ptr); }
//int _gettimeofday(struct timeval *__p, void *__tz){return 0;}
#endif // ARM_CORTEX

#endif // RNBO_NOSTDLIB

#include "RNBO_Common.h" // this must come after the definitions above

namespace RNBO {

	class PlatformInterfaceNoStdLib : public PlatformInterface
	{
	public:
		PlatformInterfaceNoStdLib()
		{
			Platform::set(this);
		}

		~PlatformInterfaceNoStdLib() override
		{

		}

		// memory allocation
		void* malloc(size_t bytes) override
		{
#ifdef ARM_CORTEX
			return ::pvPortMalloc(bytes);
#else
			return ::malloc(bytes);
#endif
		}

		void* calloc(size_t num, size_t size) override;
		{
#ifdef ARM_CORTEX
			void* newptr = malloc(num * size);
			memset(newptr, 0, num * size);
			return newptr;
#else
			return ::calloc(num, size);
#endif
		}

		void* realloc(void* ptr, size_t bytes) override
		{
#ifdef ARM_CORTEX
			void* newptr = malloc(bytes); // does not copy existing bytes at this point -- how do we know the pointer size on ARM?
			if (ptr) free(ptr);
			return newptr;
#else
			if (bytes == 0) {
				if (ptr)
					free(ptr);
				return nullptr;
			}
			return ::realloc(ptr, bytes);
#endif
		}

		void free(void* ptr) override
		{
#ifdef ARM_CORTEX
			return vPortFree(ptr);
#else
			return ::free(ptr);
#endif
		}

		// nonoptimized nostdlib implementations of memcpy/memmove
		void* memcpy(void* dest, const void* src, size_t n) override
		{
			char* pd = (char*)dest;
			char* ps = (char*)src;

			while (n--) {
				*pd++ = *ps++;
			}
			return dest;
			// return ::memcpy(dest, src, n);
		}

		void* memmove(void* dest, const void* src, size_t n) override
		{
			unsigned char tmp[n];
			memcpy(tmp,src,n);
			memcpy(dest,tmp,n);
			return dest;
#if 0 //  worth considering, as well
			if (src < dst) { // copy from the back
				char *pd = (char*)dest + n;
				char *ps = (char*)src + n;

				while (n--) {
					*--pd-- = *--ps;
				}
				return dest;
			} else if (src > dst) { // copy from the front
				return memcpy(dest, src, n);
			}
			// otherwise just return
			return dest;
#endif
			// return ::memmove(dest, src, n);
		}

		void* memset(void *dest, int value, size_t n) override
		{
			unsigned char* pd = (unsigned char*)dest;
			unsigned char v = (unsigned char)value;

			while (n--) {
				*pd++ = v;
			}
			return dest;
		}

		size_t strlen(const char *s) override
		{
			size_t i;
			for (i = 0; s[i] != '\0'; i++) ;
			return i;
			// return ::strlen(s);
		}

		int strcmp(const char *s1, const char *s2) override
		{
			while(*s1 && (*s1==*s2))
				s1++,s2++;
			return *(const unsigned char*)s1-*(const unsigned char*)s2;
			// return ::strcmp(s1, s2);
		}

		char* strcpy(char* dest, const char* src)
		{
			char* temp = dest;
			while((*dest++ = *src++)) ;
			return temp;
		}

		// printing & string formatting
		void printMessage(const char *message) override
		{
			// no stdio without stdlib, sorry
			// ::printf(message);
		}

		void toString(char *str, size_t maxlen, number val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%f", val);
		}

		void toString(char *str, size_t maxlen, int val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%d", val);
		}

		void toString(char *str, size_t maxlen, unsigned int val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%u", val);
		}

		void toString(char *str, size_t maxlen, long val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%ld", val);
		}

		void toString(char *str, size_t maxlen, long long val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%lld", val);
		}

		void toString(char *str, size_t maxlen, unsigned long val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%lu", val);
		}

		void toString(char *str, size_t maxlen, unsigned long long val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%llu", val);
		}

		void toString(char *str, size_t maxlen, void* val) override
		{
			// no stdio without stdlib, sorry
			// snprintf(str, maxlen, "%p", val);
		}

		void abort() override {
			::abort();
		}

		void error(RuntimeError, char*) override {
			abort();
		}

		void assertTrue(bool v, const char*) override {
			if (!v) {
				abort();
			}
		}
	};

	PlatformInterfaceNoStdLib platformInstance;

} // namespace RNBO

