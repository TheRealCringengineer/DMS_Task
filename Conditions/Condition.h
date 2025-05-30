#ifndef CONDITION_H
#define CONDITION_H

#include <cstdint>
#include <string>

#include "../Input/Input.h"

enum class ConditionState : uint8_t
{
  Start = 0,
  NoChange,
  End
};

class Condition
{
public:
  Condition(const std::string& n)
      : name(n)
  {}

  virtual ConditionState CheckCondition(std::shared_ptr<InputData> data,
                                        float time) = 0;

  virtual bool ShouldNotify() = 0;

  virtual ~Condition() = default;

  std::string_view GetName() { return name; }

  float GetStartTime() { return startTime; }
  float GetEndTime() { return endTime; }
  float GetDuration() { return endTime - startTime; }

protected:
  std::string name;

  float startTime = .0f;
  float endTime = .0f;
};

#endif// CONDITION_H
