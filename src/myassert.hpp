//
// Created by ayrat on 24/05/16.
//

#ifndef SDF_MYASSERT_H
#define SDF_MYASSERT_H

#define MASSERT(condition, message)                                          \
{                                                                            \
  if(!(condition))                                                           \
  {                                                                          \
    std::cerr << __FILE__ << " (" << __LINE__ << ") : " << message << std::endl;  \
    abort();                                                                 \
  }                                                                          \
}

#endif //SDF_MYASSERT_H
