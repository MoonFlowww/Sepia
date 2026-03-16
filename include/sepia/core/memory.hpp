#pragma once

#include "types.hpp"
#include <cstdlib>
#include <new>
#include <utility>
namespace sepia {

// ---------- aligned allocator for SIMD-friendly data ----------
inline void* aligned_alloc_impl(usize alignment, usize bytes) {
  void* ptr = nullptr;
#if defined(_WIN32)
  ptr = _aligned_malloc(bytes, alignment);
#else
  if (posix_memalign(&ptr, alignment, bytes) != 0) ptr = nullptr;
#endif
  if (!ptr) throw std::bad_alloc();
  return ptr;
}

inline void aligned_free_impl(void* ptr) {
#if defined(_WIN32)
  _aligned_free(ptr);
#else
  std::free(ptr);
#endif
}

// Aligned buffer: cache-line (64-byte) aligned by default.
// Owns its memory. Move-only.
template <typename T, usize Alignment = 64>
class AlignedBuffer {
public:
  AlignedBuffer() = default;

  explicit AlignedBuffer(usize count)
    : size_(count)
    , data_(static_cast<T*>(aligned_alloc_impl(Alignment, count * sizeof(T))))
  {}

  ~AlignedBuffer() { release(); }

  AlignedBuffer(const AlignedBuffer&) = delete;
  AlignedBuffer& operator=(const AlignedBuffer&) = delete;

  AlignedBuffer(AlignedBuffer&& o) noexcept
  : size_(o.size_), data_(o.data_) { o.data_ = nullptr; o.size_ = 0; }

  AlignedBuffer& operator=(AlignedBuffer&& o) noexcept {
    if (this != &o) { release(); size_ = o.size_; data_ = o.data_; o.data_ = nullptr; o.size_ = 0; }
    return *this;
  }

  void resize(usize count) {
    if (count == size_) return;
    release();
    size_ = count;
    data_ = static_cast<T*>(aligned_alloc_impl(Alignment, count * sizeof(T)));
  }

  T* data() { return data_; }
  const T* data() const { return data_; }
  usize size() const { return size_; }
  bool empty() const { return size_ == 0; }

  T& operator[](usize i) { return data_[i]; }
  const T& operator[](usize i) const { return data_[i]; }

  T* begin() { return data_; }
  T* end() { return data_ + size_; }
  const T* begin() const { return data_; }
  const T* end() const { return data_ + size_; }

private:
  void release() { if (data_) { aligned_free_impl(data_); data_ = nullptr; size_ = 0; } }
  usize size_ = 0;
  T* data_ = nullptr;
};

// Pool arena - bump allocator for short-lived per-frame work
class Arena {
public:
  explicit Arena(usize capacity)
    : capacity_(capacity)
    , buf_(static_cast<char*>(aligned_alloc_impl(64, capacity)))
  {}

  ~Arena() { aligned_free_impl(buf_); }

  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

  void* alloc(usize bytes, usize align = 8) {
    usize padding = (align - (offset_ % align)) % align;
    usize new_offset = offset_ + padding + bytes;
    if (new_offset > capacity_) throw std::bad_alloc();
    void* ptr = buf_ + offset_ + padding;
    offset_ = new_offset;
    return ptr;
  }

  template <typename T, typename... Args>
  T* create(Args&&... args) {
    void* mem = alloc(sizeof(T), alignof(T));
    return new (mem) T(std::forward<Args>(args)...);
  }

  void reset() { offset_ = 0; }
  usize used() const { return offset_; }
  usize capacity() const { return capacity_; }

private:
  usize capacity_;
  usize offset_ = 0;
  char* buf_;
};

} // NS sepia
