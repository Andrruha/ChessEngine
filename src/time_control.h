#ifndef SRC_TIME_CONTROL_H_
#define SRC_TIME_CONTROL_H_

#include <cstdint>
#include <string>

namespace chess_engine {

struct TimeControl {
  int32_t period;
  double seconds_per_period;
  double increment;
  double GuaranteedTimePerMove() const;
};

}  // namespace chess_engine

#endif  // SRC_TIME_CONTROL_H_
