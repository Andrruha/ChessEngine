#include "time_control.h"

namespace chess_engine {

  double TimeControl::GuaranteedTimePerMove() const {
    return increment + seconds_per_period/period;
  }

}  // namespace chess_engine