/*
 * File:   Aligned_Allocator.hpp
 * Author: atlas
 *
 * Created on July 5, 2013, 6:52 PM


Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/


#include <cstddef>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <vector>


//! \def BOOST_AFIO_PACKEDTYPE(typedecl) The markup this compiler uses to pack a structure as tightly as possible
#ifndef BOOST_AFIO_PACKEDTYPE
#ifdef BOOST_MSVC
#define BOOST_AFIO_PACKEDTYPE(typedecl) __pragma(pack(push, 1)) typedecl __pragma(pack(pop))
#elif defined(__GNUC__)
#define BOOST_AFIO_PACKEDTYPE(typedecl) typedecl __attribute__((packed))
#else
#define BOOST_AFIO_PACKEDTYPE(typedecl) unknown_type_pack_markup_for_this_compiler
#endif
#endif


BOOST_AFIO_V1_NAMESPACE_BEGIN
namespace detail
{
  enum class allocator_alignment : size_t
  {
    Default = sizeof(void *),  //!< The default alignment on this machine.
    SSE = 16,                  //!< The alignment for SSE. Better to use M128 for NEON et al support.
    M128 = 16,                 //!< The alignment for a 128 bit vector.
    AVX = 32,                  //!< The alignment for AVX. Better to use M256 for NEON et al support.
    M256 = 32                  //!< The alignment for a 256 bit vector.
  };
#ifdef BOOST_WINDOWS
  extern "C" void *_aligned_malloc(size_t size, size_t alignment);
  extern "C" void _aligned_free(void *blk);
#else
  extern "C" int posix_memalign(void **memptr, size_t alignment, size_t size);
#endif
  inline void *allocate_aligned_memory(size_t align, size_t size)
  {
#ifdef BOOST_WINDOWS
    return _aligned_malloc(size, align);
#else
    void *ret = nullptr;
    if(posix_memalign(&ret, align, size))
      return nullptr;
    return ret;
#endif
  }
  inline void deallocate_aligned_memory(void *ptr) BOOST_NOEXCEPT_OR_NOTHROW
  {
#ifdef BOOST_WINDOWS
    _aligned_free(ptr);
#else
    free(ptr);
#endif
  }


  /*! \class aligned_allocator
  \brief An STL allocator which allocates aligned memory

  Stolen from http://stackoverflow.com/questions/12942548/making-stdvector-allocate-aligned-memory
  */
  template <typename T, size_t Align = std::alignment_of<T>::value, bool initialize = true> class aligned_allocator
  {
  public:
    typedef T value_type;
    typedef T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    enum
    {
      alignment = Align
    };

    typedef std::true_type propagate_on_container_move_assignment;

    template <class U> struct rebind
    {
      typedef aligned_allocator<U, Align, initialize> other;
    };

  public:
    aligned_allocator() BOOST_NOEXCEPT_OR_NOTHROW {}

    template <class U> aligned_allocator(const aligned_allocator<U, Align, initialize> &) BOOST_NOEXCEPT_OR_NOTHROW {}

    size_type max_size() const BOOST_NOEXCEPT_OR_NOTHROW { return (size_type(~0) - size_type(Align)) / sizeof(T); }

    pointer address(reference x) const BOOST_NOEXCEPT_OR_NOTHROW { return std::addressof(x); }

    const_pointer address(const_reference x) const BOOST_NOEXCEPT_OR_NOTHROW { return std::addressof(x); }

    pointer allocate(size_type n, typename aligned_allocator<void, Align, initialize>::const_pointer = 0)
    {
      const size_type alignment = static_cast<size_type>(Align);
      void *ptr = detail::allocate_aligned_memory(alignment, n * sizeof(T));
      if(ptr == nullptr)
      {
        throw std::bad_alloc();
      }

      return reinterpret_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type) BOOST_NOEXCEPT_OR_NOTHROW { return detail::deallocate_aligned_memory(p); }

    template <class U, class... Args> void construct(U *p, Args &&... args)
    {
      if(initialize || !std::is_same<char, U>::value)
        ::new(reinterpret_cast<void *>(p)) U(std::forward<Args>(args)...);
    }

    void destroy(pointer p)
    {
      (void) p;
      p->~T();
    }
  };

  template <size_t Align, bool initialize> class aligned_allocator<void, Align, initialize>
  {
  public:
    typedef void value_type;
    typedef void *pointer;
    typedef const void *const_pointer;
    typedef void reference;
    typedef const void const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    enum
    {
      alignment = Align
    };
  };
  template <size_t Align, bool initialize> class aligned_allocator<const void, Align, initialize>
  {
  public:
    typedef const void value_type;
    typedef const void *pointer;
    typedef const void *const_pointer;
    typedef void reference;
    typedef const void const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    enum
    {
      alignment = Align
    };
  };

  template <typename T, size_t Align, bool initialize> class aligned_allocator<const T, Align, initialize>
  {
  public:
    typedef T value_type;
    typedef const T *pointer;
    typedef const T *const_pointer;
    typedef T &reference;
    typedef const T &const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;
    enum
    {
      alignment = Align
    };

    typedef std::true_type propagate_on_container_move_assignment;

    template <class U> struct rebind
    {
      typedef aligned_allocator<U, Align, initialize> other;
    };

  public:
    aligned_allocator() BOOST_NOEXCEPT_OR_NOTHROW {}

    template <class U> aligned_allocator(const aligned_allocator<U, Align, initialize> &) BOOST_NOEXCEPT_OR_NOTHROW {}

    size_type max_size() const BOOST_NOEXCEPT_OR_NOTHROW { return (size_type(~0) - size_type(Align)) / sizeof(T); }

    const_pointer address(const_reference x) const BOOST_NOEXCEPT_OR_NOTHROW { return std::addressof(x); }

    pointer allocate(size_type n, typename aligned_allocator<void, Align, initialize>::const_pointer = 0)
    {
      const size_type alignment = static_cast<size_type>(Align);
      void *ptr = detail::allocate_aligned_memory(alignment, n * sizeof(T));
      if(ptr == nullptr)
      {
        throw std::bad_alloc();
      }

      return reinterpret_cast<pointer>(ptr);
    }

    void deallocate(pointer p, size_type) BOOST_NOEXCEPT_OR_NOTHROW { return detail::deallocate_aligned_memory(p); }

    template <class U, class... Args> void construct(U *p, Args &&... args)
    {
      if(initialize || !std::is_same<char, U>::value)
        ::new(reinterpret_cast<void *>(p)) U(std::forward<Args>(args)...);
    }

    void destroy(pointer p) { p->~T(); }
  };

  template <typename T, size_t TAlign, bool Tinit, typename U, size_t UAlign, bool Uinit> inline bool operator==(const aligned_allocator<T, TAlign, Tinit> &, const aligned_allocator<U, UAlign, Uinit> &) BOOST_NOEXCEPT_OR_NOTHROW { return TAlign == UAlign; }

  template <typename T, size_t TAlign, bool Tinit, typename U, size_t UAlign, bool Uinit> inline bool operator!=(const aligned_allocator<T, TAlign, Tinit> &, const aligned_allocator<U, UAlign, Uinit> &) BOOST_NOEXCEPT_OR_NOTHROW { return TAlign != UAlign; }


}  // namespace detail
BOOST_AFIO_V1_NAMESPACE_END
