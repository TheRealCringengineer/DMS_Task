#include <fstream>
#include <iostream>

#include "Conditions/Condition.h"
#include "Events/EventBus.h"
#include "Input/Decoder.h"


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

class StdoutDecoder : public Decoder
{
public:
  StdoutDecoder(InputData<float>* data,
                uint32_t frameRate,
                EventBus<std::string_view, float, float>* e)
      : Decoder(data, frameRate)
      , eventBus(e)
  {}

  void CheckConditions(float time) override
  {
    for(auto condition: conditions) {

      ConditionState state = condition->CheckCondition(input, time);
      switch(state) {
      case ConditionState::Start:
        break;
      case ConditionState::End:
        // Event
        if(condition->ShouldNotify())
          eventBus->FireEvent(condition->GetName(),
                              condition->GetStartTime(),
                              condition->GetEndTime());
        break;
      case ConditionState::NoChange:
        break;
      }
    }
  }

private:
  EventBus<std::string_view, float, float>* eventBus;
};

int main()
{
  auto d =
      new InputDataStream<float>(new std::ifstream("perception_results.txt"),
                                 {"Camera.probBlocked",
                                  "Person.confidence",
                                  "Person.bbox.x",
                                  "Person.bbox.y",
                                  "Person.bbox.width",
                                  "Person.bbox.height",
                                  "Head.detection.confidence",
                                  "Head.detection.bbox.x",
                                  "Head.detection.bbox.y",
                                  "Head.detection.bbox.width",
                                  "Head.detection.bbox.height",
                                  "Head.position.x",
                                  "Head.position.y",
                                  "Head.position.z",
                                  "Head.orientation.yaw",
                                  "Head.orientation.pitch",
                                  "Head.orientation.roll",
                                  "Head.gaze.yaw",
                                  "Head.gaze.pitch",
                                  "Hand1.confidence",
                                  "Hand1.bbox.x",
                                  "Hand1.bbox.y",
                                  "Hand1.bbox.width",
                                  "Hand1.bbox.height",
                                  "Hand2.confidence",
                                  "Hand2.bbox.x",
                                  "Hand2.bbox.y",
                                  "Hand2.bbox.width",
                                  "Hand2.bbox.height",
                                  "Eye1.detection.confidence",
                                  "Eye1.detection.bbox.x",
                                  "Eye1.detection.bbox.y",
                                  "Eye1.detection.bbox.width",
                                  "Eye1.detection.bbox.height",
                                  "Eye1.state.probClosed",
                                  "Eye1.state.probOpen",
                                  "Eye1.state.probUnknown",
                                  "Eye2.detection.confidence",
                                  "Eye2.detection.bbox.x",
                                  "Eye2.detection.bbox.y",
                                  "Eye2.detection.bbox.width",
                                  "Eye2.detection.bbox.height",
                                  "Eye2.state.probClosed",
                                  "Eye2.state.probOpen",
                                  "Eye2.state.probUnknown"},
                                 ";");

  EventBus<std::string_view, float, float> eventBus;

  // Выглядит ужасно, но пойдёт
  auto sh = std::make_shared<
      std::function<bool(std::string_view name, float start, float end)>>(
      [](std::string_view name, float start, float end) -> bool {
        std::cout << "Event : " << name << " started at " << start
                  << " ended at " << end << "\n";

        return false;
      });

  eventBus.Register(sh);

  const uint32_t VIDEO_FRAMERATE = 20;
  StdoutDecoder a(d, VIDEO_FRAMERATE, &eventBus);
  a.RegisterCondition(new EyesClosedCondition<float>());
  a.Run();

  return 0;
}
