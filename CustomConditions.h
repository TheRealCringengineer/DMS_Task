#ifndef CUSTOMCONDITIONS_H
#define CUSTOMCONDITIONS_H

#include "Conditions/Condition.h"
#include <cstdint>

// Сюда я вынес кастомные события (условия), которые проверяются в рамках обработки входных данных
// Аналогичным образом можно, например, делать специальные `DebugConditions` и т.д.
//

// Проверка на то, закрыты ли глаза
template<typename D>
class EyesClosedCondition : public Condition<D>
{
public:
  EyesClosedCondition()
      : Condition<D>("Eyes closed")
  {}

  // В данных нет длительности больше 3-х секунд даже)
  // Поэтому сделал вот так, чтобы была видна работа
  bool ShouldNotify() override { return this->GetDuration() > 2.0f; }

  ConditionState CheckCondition(InputData<D>* data, float time) override
  {
    prevState = wasLeftEyeClosed && wasRightEyeClosed;

    if(IsPerson(data)) {
      if(IsHead(data)) {
        if(IsLeftEye(data) && IsRightEye(data)) {
          if(IsLeftEyeClosed(data)) {
            wasLeftEyeClosed = true;
          } else if(IsLeftEyeOpened(data)) {
            wasLeftEyeClosed = false;
          }

          if(IsRightEyeClosed(data)) {
            wasRightEyeClosed = true;
          } else if(IsRightEyeOpened(data)) {
            wasRightEyeClosed = false;
          }
        }
      }
    } else {
      // Если мы не можем найти человека => всё нормально
      wasRightEyeClosed = false;
      wasLeftEyeClosed = false;
    }

    // Если человек есть, а головы, глаз не видно => считаем что состояние
    // закрытия/открытия глаз не менялось
    if(!prevState && (wasRightEyeClosed && wasLeftEyeClosed)) {
      this->startTime = time;
      return ConditionState::Start;
    }

    if(prevState && !(wasRightEyeClosed && wasLeftEyeClosed)) {
      this->endTime = time;
      return ConditionState::End;
    }

    return ConditionState::NoChange;
  };

  ~EyesClosedCondition() override = default;

private:
  bool prevState = false;

  // Храним последнее состояние глаза. В случае
  // неопредленного сотояния (probUnknown или вероятности у всех состояний < 0.5)
  // мы учитываем его как текущее состояние
  bool wasLeftEyeClosed = false;
  bool wasRightEyeClosed = false;

  const float SUCCESS_VALUE = 0.5f;
  bool IsPerson(InputData<D>* data)
  {
    return data->GetField("Person.confidence") > SUCCESS_VALUE;
  }

  bool IsHead(InputData<D>* data)
  {
    return data->GetField("Head.detection.confidence") > SUCCESS_VALUE;
  }

  bool IsLeftEye(InputData<D>* data)
  {
    return data->GetField("Eye1.detection.confidence") > SUCCESS_VALUE;
  }

  bool IsLeftEyeOpened(InputData<D>* data)
  {
    return data->GetField("Eye1.state.probOpen") > SUCCESS_VALUE;
  }

  bool IsLeftEyeClosed(InputData<D>* data)
  {
    return data->GetField("Eye1.state.probClosed") > SUCCESS_VALUE;
  }

  bool IsLeftEyeUnknown(InputData<D>* data)
  {
    return data->GetField("Eye1.state.probUnknown") > SUCCESS_VALUE;
  }

  bool IsRightEye(InputData<D>* data)
  {
    return data->GetField("Eye2.detection.confidence") > SUCCESS_VALUE;
  }

  bool IsRightEyeOpened(InputData<D>* data)
  {
    return data->GetField("Eye2.state.probOpen") > SUCCESS_VALUE;
  }

  bool IsRightEyeClosed(InputData<D>* data)
  {
    return data->GetField("Eye2.state.probClosed") > SUCCESS_VALUE;
  }

  bool IsRightEyeUnknown(InputData<D>* data)
  {
    return data->GetField("Eye2.state.probUnknown") > SUCCESS_VALUE;
  }
};

// Проверка на то, смотрит ли человек в сторону
template<typename D>
class LookingAwayCondition : public Condition<D>
{
public:
  LookingAwayCondition()
      : Condition<D>("Looking away")
  {}

  bool ShouldNotify() override { return this->GetDuration() > 2.0f; }

  ConditionState CheckCondition(InputData<D>* data, float time) override
  {
    prevState = wasLookingAway;

    if(IsPerson(data)) {
      if(IsHead(data)) {
        if(IsHeadLookingAway(data)) {
          wasLookingAway = true;
          counter = 0;
        } else {
          counter++;
          if(counter > 2) {
            // Исключаем моменты когда водитель глянул в камеру на пару кадров
            wasLookingAway = false;
          }
        }
      }
    }

    if(!prevState && wasLookingAway) {
      this->startTime = time;
      return ConditionState::Start;
    }

    if(prevState && !wasLookingAway) {
      this->endTime = time;
      return ConditionState::End;
    }

    return ConditionState::NoChange;
  }

  ~LookingAwayCondition() override = default;

private:
  bool prevState = false;
  bool wasLookingAway = false;

  uint32_t counter = 0;

  const float SUCCESS_VALUE = 0.5f;
  bool IsPerson(InputData<D>* data)
  {
    return data->GetField("Person.confidence") > SUCCESS_VALUE;
  }

  bool IsHead(InputData<D>* data)
  {
    return data->GetField("Head.detection.confidence") > SUCCESS_VALUE;
  }

  const float THRESHOLD_ANGLE_YAW = 0.35f; // 0.35 radians = 20 degrees
  const float THRESHOLD_ANGLE_PITCH = 0.5f;// 0.5 radians ~ 28 degrees
  bool IsHeadLookingAway(InputData<D>* data)
  {
    return data->GetField("Head.orientation.yaw") > THRESHOLD_ANGLE_YAW
        || data->GetField("Head.gaze.yaw") > THRESHOLD_ANGLE_YAW
        || data->GetField("Head.orientation.pitch") > THRESHOLD_ANGLE_PITCH
        || data->GetField("Head.gaze.pitch") > THRESHOLD_ANGLE_PITCH
        || data->GetField("Head.orientation.yaw") < -THRESHOLD_ANGLE_YAW
        || data->GetField("Head.gaze.yaw") < -THRESHOLD_ANGLE_YAW
        || data->GetField("Head.orientation.pitch") < -THRESHOLD_ANGLE_PITCH
        || data->GetField("Head.gaze.pitch") < -THRESHOLD_ANGLE_PITCH;
    
  }
};

#endif// CUSTOMCONDITIONS_H
