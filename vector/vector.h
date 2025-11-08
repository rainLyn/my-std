#pragma once

#include <cstddef>
#include <iostream>
#include <utility>
#include <valarray>

namespace rainLyn {
template <typename T> class vector {
public:
  vector() : m_data(), m_size(), m_capacity() {}

  // 复制构造
  vector(const vector &origin) {
    m_data = static_cast<T *>(operator new(origin.m_capacity * sizeof(T)));

    // 通过size记录已经复制了多少个对象
    m_size = 0;
    m_capacity = origin.m_capacity;

    try {
      for (size_t i = 0; i < origin.m_size; i++) {
        // 直接在指定位置new
        new (&m_data[i]) T(origin.m_data[i]);
        m_size++;
      }
    } catch (...) {
      // 如果复制报错 就依次析构已复制的对象并释放申请到的内存
      release_memory(m_data, m_size);
      throw;
    }
  }

  // 移动构造
  vector(vector &&origin) {
    // 不能用move
    // move相当于让两个vector指向同一块内存
    // 当前vector析构后 再释放origin就会对一块内存释放两次
    m_data = origin.m_data;
    m_size = origin.m_size;
    m_capacity = origin.m_capacity;

    origin.m_data = nullptr;
    origin.m_size = 0;
    origin.m_capacity = 0;
  }

  ~vector() {
    // 依次调用m_data内data的析构函数
    // 在申请内存时申请的是原始内存 之后再进行类型转换
    // 如果直接delete不会正确调用析构函数
    // 所以手动调用

    // 如果分配内存大小了就删除掉
    release_memory(m_data, m_size);
  }

  // 返回vector开头的地址
  T *begin() { return m_data; }
  const T *cbegin() const { return m_data; }

  // 返回最后一个元素后方的地址
  T *end() { return m_data + m_size; }
  const T *cend() const { return m_data + m_size; }

  // 返回第一个元素
  T *front() {
    if (m_size > 0)
      return &m_data[0];
    else
      return nullptr;
  }
  const T *front() const {
    if (m_size > 0)
      return &m_data[0];
    else
      return nullptr;
  }

  // 返回最后一个元素
  T *back() {
    if (m_size > 0)
      return &m_data[m_size - 1];
    else
      return nullptr;
  }
  const T *back() const {
    if (m_size > 0)
      return &m_data[m_size - 1];
    else
      return nullptr;
  }

  // 返回第一个元素
  T *data() { return m_data; }
  const T *data() const { return m_data; }

  // 返回vector大小
  std::size_t size() { return m_size; }
  const std::size_t size() const { return m_size; }

  // 返回vector当前容量
  std::size_t capacity() { return m_capacity; }
  const std::size_t capacity() const { return m_capacity; }

  // 将vector内的数据清除
  void clear() {
    // 依次调用析构函数
    for (size_t i = 0; i < m_size; i++)
      m_data[i].~T();

    m_size = 0;
  }

  // 弹出末尾数据
  void pop_back() {
    if (m_size <= 0)
      return;
    std::size_t newSize = m_size - 1;
    m_data[newSize].~T();
    m_size = newSize;
  }

  // 在末尾追加元素
  void push_back(const T &newData) { emplace_back(newData); }
  void push_back(const T &&newData) { emplace_back(std::move(newData)); }

  // 在末尾追加元素并返回追加元素的地址
  template <typename... ArgsT> T &emplace_back(ArgsT &&...args) {
    if (m_size < m_capacity) {
      std::size_t newDataPtr = m_size;
      new (&m_data[newDataPtr]) T(std::forward<ArgsT>(args)...);
      m_size++;
      return m_data[newDataPtr];
    }

    // 如果分配内存不足先扩容2倍
    std::size_t newCapacity = (1 + m_size) * 2;
    auto newData = static_cast<T *>(operator new(newCapacity * sizeof(T)));

    m_capacity = newCapacity;
    std::size_t newSize = 0;

    try {
      // 逐一从原本data中move
      for (size_t i = 0; i < m_size; i++) {
        new (&newData[i]) T(std::move(m_data[i]));
        newSize++;
      }

      new (&newData[newSize]) T(std::forward<ArgsT>(args)...);
      newSize++;
    } catch (...) {
      release_memory(newData, newSize);
      throw;
    }

    release_memory(m_data, m_size);

    m_data = newData;
    m_size = newSize;
    m_capacity = newCapacity;

    return m_data[m_size];
  }

  T &operator[](size_t index) { return m_data[index]; }

  T &at(size_t index) {
    try {
      return m_data[index];
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
    }
  }

  bool empty() { return m_size == 0; }

  void insert(size_t index, const T &insertData) {
    if (m_size + 1 < m_capacity) {
      for (int i = m_size + 1; i > index; i--) {
        new (&m_data[i]) T(std::move(m_data[i - 1]));
        m_data[i - 1].~T();
      }

      new (&m_data[index]) T(std::move(insertData));

      m_size++;
      return;
    }

    // 如果分配内存不足先扩容2倍
    std::size_t newCapacity = m_size * 2;
    auto newData = static_cast<T *>(operator new(newCapacity * sizeof(T)));

    m_capacity = newCapacity;
    std::size_t newSize = 0;

    try {
      // 逐一从原本data中move
      for (size_t i = 0; i < index; i++) {
        new (&newData[i]) T(std::move(m_data[i]));
        newSize++;
      }

      for (int i = m_size + 1; i > index; i--) {
        new (&newData[i]) T(std::move(m_data[i - 1]));
        newSize++;
      }

      new (&newData[index]) T(std::move(insertData));
      newSize++;
    } catch (...) {
      release_memory(newData, newSize);
      throw;
    }

    release_memory(m_data, m_size);

    m_data = newData;
    m_size = newSize;
    m_capacity = newCapacity;

    return;
  }
  void insert(size_t index, const T &&insertData) {
    if (m_size + 1 < m_capacity) {
      for (int i = m_size + 1; i > index; i--) {
        new (&m_data[i]) T(std::move(m_data[i - 1]));
        m_data[i - 1].~T();
      }

      new (&m_data[index]) T(std::move(insertData));

      m_size++;
      return;
    }

    // 如果分配内存不足先扩容2倍
    std::size_t newCapacity = m_size * 2;
    auto newData = static_cast<T *>(operator new(newCapacity * sizeof(T)));

    m_capacity = newCapacity;
    std::size_t newSize = 0;

    try {
      // 逐一从原本data中move
      for (size_t i = 0; i < index; i++) {
        new (&newData[i]) T(std::move(m_data[i]));
        newSize++;
      }

      for (int i = m_size + 1; i > index; i--) {
        new (&newData[i]) T(std::move(m_data[i - 1]));
        newSize++;
      }

      new (&newData[index]) T(std::move(insertData));
      newSize++;
    } catch (...) {
      release_memory(newData, newSize);
      throw;
    }

    release_memory(m_data, m_size);

    m_data = newData;
    m_size = newSize;
    m_capacity = newCapacity;

    return;
  }

  void erase(size_t index) {
    if (index >= m_size)
      return;

    for (int i = index + 1; i < m_size; i++) {
      m_data[i - 1].~T();
      new (&m_data[i - 1]) T(std::forward<T>(m_data[i]));
    }

    pop_back();
  }

private:
  T *m_data;
  std::size_t m_size;
  std::size_t m_capacity;

  template <typename ReleaseT>
  void release_memory(ReleaseT *data, std::size_t size) {
    for (size_t i = 0; i < size; i++)
      data[i].~ReleaseT();
    operator delete(data);
  }
};

} // namespace rainLyn
