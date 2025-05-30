#include <fstream>
#include <iostream>
#include <memory>

#include "Conditions/Condition.h"
#include "Events/EventBus.h"
#include "Input/Decoder.h"

#include "CustomConditions.h"

class StdoutDecoder : public Decoder
{
public:
  StdoutDecoder(std::shared_ptr<InputData> data,
                uint32_t frameRate,
                std::shared_ptr<EventBus<std::string_view, float, float>> e)
      : Decoder(data, frameRate)
      , eventBus(e)
  {}

  void CheckConditions(float time) override
  {
    for(auto& condition: conditions) {

      ConditionState state = condition->CheckCondition(input, time);
      switch(state) {
      case ConditionState::End:
        if(condition->ShouldNotify())
          eventBus->FireEvent(condition->GetName(),
                              condition->GetStartTime(),
                              condition->GetEndTime());
        break;

      case ConditionState::Start:
      case ConditionState::NoChange:
      default:
        break;
      }
    }
  }

private:
  std::shared_ptr<EventBus<std::string_view, float, float>> eventBus;
};

int main()
{
  const std::string delimiter = ";";
  auto fields = std::vector<std::string>{"Camera.probBlocked",
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
                                         "Eye2.state.probUnknown"};

  auto file = std::make_shared<std::ifstream>("perception_results.txt");

  auto dataStream = std::make_shared<InputDataStream>(file, fields, delimiter);

  using EventHandler = bool(std::string_view, float, float);
  auto handler = std::make_shared<std::function<EventHandler>>(

      [](std::string_view name, float start, float end) -> bool {
        std::cout << "Event : " << name << " started at " << start
                  << " ended at " << end << "\n";

        return false;
      });

  auto eventBus = std::make_shared<EventBus<std::string_view, float, float>>();
  eventBus->Register(handler);

  const uint32_t VIDEO_FRAMERATE = 20;
  StdoutDecoder decoder(dataStream, VIDEO_FRAMERATE, eventBus);

  decoder.RegisterCondition(std::make_shared<EyesClosedCondition>());
  decoder.RegisterCondition(std::make_shared<LookingAwayCondition>());

  decoder.Run();

  return 0;
}
