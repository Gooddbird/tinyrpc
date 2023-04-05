#include <memory>
#include <sys/mman.h>
#include <assert.h>
#include <stdlib.h>
#include "tinyrpc/comm/log.h"
#include "tinyrpc/coroutine/memory.h"

namespace tinyrpc {

Memory::Memory(int block_size, int block_count) : m_block_size(block_size), m_block_count(block_count) {
  m_size = m_block_count * m_block_size;
  m_start = (char*)malloc(m_size);
  assert(m_start != (void*)-1);
  InfoLog << "succ mmap " << m_size << " bytes memory";
  m_end = m_start + m_size - 1;
  m_blocks.resize(m_block_count);
  for (size_t i = 0; i < m_blocks.size(); ++i) {
    m_blocks[i] = false;
  }
  m_ref_counts = 0;
}

// void Memory::free() {
//   if (!m_start || m_start == (void*)-1) {
//     return;
//   }
//   int rt = free(m_start);
//   if (rt != 0) {
//     ErrorLog << "munmap error, error=" << strerror(errno);
//   }
//   InfoLog << "~succ free munmap " << m_size << " bytes memory";
//   m_start = NULL;
//   m_ref_counts = 0;
// }

Memory::~Memory() {
  if (!m_start || m_start == (void*)-1) {
    return;
  }
  free(m_start);
  InfoLog << "~succ free munmap " << m_size << " bytes memory";
  m_start = NULL;
  m_ref_counts = 0;
}

char* Memory::getStart() {
  return m_start;
}

char* Memory::getEnd() {
  return m_end;
}

int Memory::getRefCount() {
  return m_ref_counts;
}

char* Memory::getBlock() {
  int t = -1;
  Mutex::Lock lock(m_mutex);
  for (size_t i = 0; i < m_blocks.size(); ++i) {
    if (m_blocks[i] == false) {
      m_blocks[i] = true;  
      t = i;
      break;
    }
  }
  lock.unlock();
  if (t == -1) {
    return NULL;
  }
  m_ref_counts++;
  return m_start + (t * m_block_size);
}

void Memory::backBlock(char* s) {
  if (s > m_end || s < m_start) {
    ErrorLog << "error, this block is not belong to this Memory";
    return;
  }
  int i = (s - m_start) / m_block_size;
  Mutex::Lock lock(m_mutex);
  m_blocks[i] = false;
  lock.unlock();
  m_ref_counts--;
}

bool Memory::hasBlock(char* s) {
  return ((s >= m_start) && (s <= m_end));
}


}
