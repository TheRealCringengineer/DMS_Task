#include "CustomConditions.h"

ConditionState
EyesClosedCondition::CheckCondition(std::shared_ptr<InputData> data, float time)
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

ConditionState
LookingAwayCondition::CheckCondition(std::shared_ptr<InputData> data,
                                     float time)
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
