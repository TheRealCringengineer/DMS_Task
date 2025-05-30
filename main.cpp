#include <cstdint>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Events/EventBus.h"

// Начнём с обработки входного файла.
// unoredered_map не очень подходит, потому что хотелось бы итерировать по строчкам
// А хранить массив std::vector<std::unoredered_map> как-будто не хочется
//
// Что-нибудь вроде std::vector<std::vector>> пойдёт
// А вот map заюзаем для полей с их индексами. Норм
//
// Ну и вообще по хорошему должен быть интерфейс на случай, если будет другой формат
// Но полноценный интерфейс разрабатывать пока не хочется, поэтому так

template<typename D,
         typename =
             typename std::enable_if<std::is_floating_point<D>::value, D>::type>
class InputData
{
public:
  virtual void ReadFields() = 0;

  virtual bool IsFinished() = 0;

  virtual uint32_t GetCurrentFrame() { return currentFrame; }

  virtual D GetField(std::string_view name) = 0;

  virtual ~InputData() = default;

protected:
  uint32_t currentFrame = 0;
};

template<typename D>
class InputDataStream : public InputData<D>
{
public:
  InputDataStream(std::basic_istream<char, std::char_traits<char>>* inp,
                  std::vector<std::string> ind,
                  const std::string& del)
      : input(inp)
      , delimiter(del)
  {
    for(size_t i = 0; i < ind.size(); i++) indexes[ind[i]] = i;
  }

  void ReadFields() override
  {
    std::getline(*input, line);

    // Skipping comments
    while(line.empty() || line[0] == COMMENT_SYMBOL) {
      if(input->eof()) return;
      std::getline(*input, line);
    }

    this->currentFrame++;
    Split();
  }

  bool IsFinished() override { return input->eof(); }

  D GetField(std::string_view name) override
  {
    return tokens[indexes[name.data()]];
  }

private:
  const char COMMENT_SYMBOL = '#';

  std::unordered_map<std::string, size_t> indexes;
  std::string line;
  std::basic_istream<char, std::char_traits<char>>* input;

  std::vector<D> tokens;
  std::string token;
  std::string delimiter;

  size_t splitStart, splitEnd;

  void Split()
  {
    tokens.clear();
    splitStart = 0;

    while((splitEnd = line.find(delimiter, splitStart)) != std::string::npos) {
      token = line.substr(splitStart, splitEnd - splitStart);
      splitStart = splitEnd + delimiter.size();
      if constexpr(std::is_same_v<D, float>) tokens.push_back(std::stof(token));

      if constexpr(std::is_same_v<D, double>)
        tokens.push_back(std::stod(token));
    }

    if(!line.substr(splitStart).empty()) {
      if constexpr(std::is_same_v<D, float>) {
        tokens.push_back(std::stof(line.substr(splitStart)));
      }

      if constexpr(std::is_same_v<D, double>)
        tokens.push_back(std::stod(line.substr(splitStart)));
    }
  }
};

// Идея по проверки непосредственно определенных условий
// Возьмём "сон". Это значит
// 1) Есть person
// 2) Есть оба глаза
// 3) Оба глаза закрыты
// На этапе конкретного промежутка времени мы можем задать сложные условия формата
// {
//   "Person.confidence" > X
// }
// and
// {
//   "Eye1.detection.confidence" > Y
// }
// Вроде такого
// Фактически - набор из name + bool function(float value)
//

enum class ConditionState : uint8_t
{
  Start = 0,
  NoChange,
  End
};

template<typename D>
class Condition
{
public:
  Condition(const std::string& n)
      : name(n)
  {}

  virtual ConditionState CheckCondition(InputData<D>* data, float time) = 0;

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

template<typename D>
class EyesClosedCondition : public Condition<D>
{
public:
  EyesClosedCondition()
      : Condition<D>("Eyes closed")
  {}

  // В данных нет длительности больше 2-х секунд даже)
  // Поэтому сделал вот так, чтобы была видна работа
  bool ShouldNotify() override { return this->GetDuration() > 1.0f; }

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

// Мы можем узнать время старта и конца определенного условия
// Теперь вопрос в том, как непосредственно определить, достаточн ли времени оно было активно
// Важно что "реальное" время зависит от того, сколько записей приходит на единицу времени (секунду)
// В таком случае, возможно стоит конвертировать кадры в мс/нс/с внутри Analyzer'а и уведомлят об этом
// какой-то дополнтиельный класс?
//
// Wrapper над Condition?
// Что-то вроде
// class Event {
// publiic:
//   void Update() => cond.Check();
//
//   ...
// private:
//  std::vector<Condition *> cond;
// };
//
// И последнее - система уведомления. Какая-то шина событий. Хммм

class Analyzer
{
public:
  Analyzer(InputData<float>* data, uint32_t frameRate)
      : input(data)
      , framesPerSecond(frameRate)
  {}

  void RegisterCondition(Condition<float>* c) { conditions.push_back(c); }

  void Run()
  {
    uint32_t frame = 0;
    float realTime = .0f;

    while(!input->IsFinished()) {
      input->ReadFields();

      realTime = GetTime(frame);

      this->CheckConditions(realTime);

      frame++;
    }
  }

  virtual void CheckConditions(float time) = 0;

private:
  float GetTime(uint32_t frame)
  {
    return static_cast<float>(frame) / framesPerSecond;
  }

protected:
  InputData<float>* input;
  std::vector<Condition<float>*> conditions;

  uint32_t framesPerSecond = 24;
};

class StdoutAnalyzer : public Analyzer
{
public:
  StdoutAnalyzer(InputData<float>* data,
                 uint32_t frameRate,
                 EventBus<std::string_view, float, float>* e)
      : Analyzer(data, frameRate)
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
  StdoutAnalyzer a(d, VIDEO_FRAMERATE, &eventBus);
  a.RegisterCondition(new EyesClosedCondition<float>());
  a.Run();

  return 0;
}
