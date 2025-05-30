#ifndef CUSTOMCONDITIONS_H
#define CUSTOMCONDITIONS_H

#include "Conditions/Condition.h"
#include <cstdint>

// Сюда я вынес кастомные события (условия), которые проверяются в рамках обработки входных данных
// Аналогичным образом можно, например, делать специальные `DebugConditions` и т.д.
//

// Проверка на то, закрыты ли глаза
class EyesClosedCondition : public Condition
{
public:
  EyesClosedCondition()
      : Condition("Eyes closed")
  {}

  // В данных нет длительности больше 3-х секунд даже)
  // Поэтому сделал вот так, чтобы была видна работа
  bool ShouldNotify() override { return this->GetDuration() > 2.0f; }

  ConditionState CheckCondition(std::shared_ptr<InputData> data,
                                float time) override;

  ~EyesClosedCondition() override = default;

private:
  bool prevState = false;

  // Храним последнее состояние глаза. В случае
  // неопредленного сотояния (probUnknown или вероятности у всех состояний < 0.5)
  // мы учитываем его как текущее состояние
  bool wasLeftEyeClosed = false;
  bool wasRightEyeClosed = false;

  const float SUCCESS_VALUE = 0.5f;
  bool IsPerson(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Person.confidence") > SUCCESS_VALUE;
  }

  bool IsHead(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Head.detection.confidence") > SUCCESS_VALUE;
  }

  bool IsLeftEye(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye1.detection.confidence") > SUCCESS_VALUE;
  }

  bool IsLeftEyeOpened(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye1.state.probOpen") > SUCCESS_VALUE;
  }

  bool IsLeftEyeClosed(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye1.state.probClosed") > SUCCESS_VALUE;
  }

  bool IsLeftEyeUnknown(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye1.state.probUnknown") > SUCCESS_VALUE;
  }

  bool IsRightEye(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye2.detection.confidence") > SUCCESS_VALUE;
  }

  bool IsRightEyeOpened(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye2.state.probOpen") > SUCCESS_VALUE;
  }

  bool IsRightEyeClosed(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye2.state.probClosed") > SUCCESS_VALUE;
  }

  bool IsRightEyeUnknown(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Eye2.state.probUnknown") > SUCCESS_VALUE;
  }
};

// Проверка на то, смотрит ли человек в сторону
class LookingAwayCondition : public Condition
{
public:
  LookingAwayCondition()
      : Condition("Looking away")
  {}

  bool ShouldNotify() override { return this->GetDuration() > 2.0f; }

  ConditionState CheckCondition(std::shared_ptr<InputData> data,
                                float time) override;
  ~LookingAwayCondition() override = default;

private:
  bool prevState = false;
  bool wasLookingAway = false;

  uint32_t counter = 0;

  const float SUCCESS_VALUE = 0.5f;
  bool IsPerson(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Person.confidence") > SUCCESS_VALUE;
  }

  bool IsHead(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Head.detection.confidence") > SUCCESS_VALUE;
  }

  const float THRESHOLD_ANGLE_YAW = 0.35f; // 0.35 radians = 20 degrees
  const float THRESHOLD_ANGLE_PITCH = 0.5f;// 0.5 radians ~ 28 degrees
  bool IsHeadLookingAway(std::shared_ptr<InputData> data)
  {
    return data->TryGetFloat("Head.orientation.yaw") > THRESHOLD_ANGLE_YAW
        || data->TryGetFloat("Head.gaze.yaw") > THRESHOLD_ANGLE_YAW
        || data->TryGetFloat("Head.orientation.pitch") > THRESHOLD_ANGLE_PITCH
        || data->TryGetFloat("Head.gaze.pitch") > THRESHOLD_ANGLE_PITCH
        || data->TryGetFloat("Head.orientation.yaw") < -THRESHOLD_ANGLE_YAW
        || data->TryGetFloat("Head.gaze.yaw") < -THRESHOLD_ANGLE_YAW
        || data->TryGetFloat("Head.orientation.pitch") < -THRESHOLD_ANGLE_PITCH
        || data->TryGetFloat("Head.gaze.pitch") < -THRESHOLD_ANGLE_PITCH;
  }
};

#endif// CUSTOMCONDITIONS_H
