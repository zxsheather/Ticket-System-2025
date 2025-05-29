#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;

template <class T, int info_len = 2>
class MemoryRiver {
 private:
  mutable fstream file;
  string file_name;
  mutable bool file_opened = false;
  int sizeofT = sizeof(T);

  void ensureFileOpen() const {
    if (!file_opened) {
      file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
      if (!file.is_open()) {
        file.clear();
        file.open(file_name, std::ios::out | std::ios::binary);
        file.close();
        file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
      }
      file_opened = true;
    }
  }

 public:
  MemoryRiver() = default;

  MemoryRiver(const string& file_name) : file_name(file_name) {}

  ~MemoryRiver() {
    if (file_opened && file.is_open()) {
      file.close();
    }
  }

  // 禁止拷贝构造和赋值，避免文件句柄冲突
  MemoryRiver(const MemoryRiver&) = delete;
  MemoryRiver& operator=(const MemoryRiver&) = delete;

  // 支持移动语义
  MemoryRiver(MemoryRiver&& other) noexcept
      : file_name(std::move(other.file_name)), file_opened(other.file_opened) {
    if (file_opened) {
      file = std::move(other.file);
      other.file_opened = false;
    }
  }

  void initialise(string FN = "") {
    if (FN != "") file_name = FN;

    // 关闭已打开的文件
    if (file_opened && file.is_open()) {
      file.close();
      file_opened = false;
    }

    file.open(file_name, std::ios::out | std::ios::binary);
    int tmp = 0;
    for (int i = 0; i < info_len; ++i) {
      file.write(reinterpret_cast<char*>(&tmp), sizeof(int));
    }
    file.close();
    file_opened = false;
  }

  void get_info(int& tmp, int n) {
    if (n > info_len) return;
    ensureFileOpen();
    file.seekg((n - 1) * sizeof(int), std::ios::beg);
    file.read(reinterpret_cast<char*>(&tmp), sizeof(int));
  }

  void write_info(int tmp, int n) {
    if (n > info_len) return;
    ensureFileOpen();
    file.seekp((n - 1) * sizeof(int), std::ios::beg);
    file.write(reinterpret_cast<char*>(&tmp), sizeof(int));
    file.flush();  // 确保写入磁盘
  }

  int write(T& t) {
    ensureFileOpen();
    file.seekp(0, std::ios::end);
    int index = file.tellp();
    if (index == -1) {
      return -1;
    }
    file.write(reinterpret_cast<char*>(&t), sizeof(T));
    file.flush();
    return index;
  }

  void update(T& t, const int index) {
    ensureFileOpen();
    file.seekp(index, std::ios::beg);
    file.write(reinterpret_cast<char*>(&t), sizeof(T));
    file.flush();
  }

  void read(T& t, const int index) {
    ensureFileOpen();
    file.seekg(index, std::ios::beg);
    file.read(reinterpret_cast<char*>(&t), sizeof(T));
  }

  void Delete(int index) {
    ensureFileOpen();
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    if (index + sizeof(T) >= size) return;

    file.seekg(index + sizeof(T), std::ios::beg);
    char* buffer = new char[size - index - sizeof(T)];
    file.read(buffer, size - index - sizeof(T));
    file.seekp(index, std::ios::beg);
    file.write(buffer, size - index - sizeof(T));
    file.flush();
    delete[] buffer;
  }

  bool exist() const {
    std::ifstream test_file(file_name, std::ios::binary);
    return test_file.good();
  }

  void flush() {
    if (file_opened && file.is_open()) {
      file.flush();
    }
  }

  void close() {
    if (file_opened && file.is_open()) {
      file.close();
      file_opened = false;
    }
  }
};

#endif  // BPT_MEMORYRIVER_HPP