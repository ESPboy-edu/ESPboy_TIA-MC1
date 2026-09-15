#pragma once

namespace IOList {

#define IO_LIST_MAX_LEN	400

struct IOListItem {
  unsigned short int time;
  unsigned char reg;
  unsigned char val;
};

IOListItem entry[IO_LIST_MAX_LEN];

int ptr = 0;

inline void clear(void)
{
  ptr = 0;
}

inline int length(void)
{
  return ptr;
}

inline void add(int time, unsigned char reg, unsigned char val)
{
  if (ptr < IO_LIST_MAX_LEN)
  {
    entry[ptr].time = time;
    entry[ptr].reg = reg;
    entry[ptr].val = val;

    ++ptr;
  }
}

}