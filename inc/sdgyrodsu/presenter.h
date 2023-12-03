#ifndef _KMICKI_SDGYRODSU_PRESENTER_H_
#define _KMICKI_SDGYRODSU_PRESENTER_H_

#include "sdhidframe.h"
#include <vector>

namespace kmicki::sdgyrodsu {
class Presenter {
public:
  static void Initialize();
  static void Present(SdHidFrame const &frame);
  static void Finish();
};
} // namespace kmicki::sdgyrodsu

#endif