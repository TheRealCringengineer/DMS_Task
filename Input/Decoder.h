#ifndef DECODER_H
#define DECODER_H

#include "../Conditions/Condition.h"
#include "Input.h"

// Скорее даже какой-то TimeBasedDecoder получается
// Фактически пока у нас есть набор данных, разбитый по дискретным интервалам (InputData)
// Декодер позволит нормально читать их определять примреное время событий

class Decoder
{
public:
  Decoder(std::shared_ptr<InputData> data, uint32_t dataRate)
      : input(data)
      , readsPerSecond(dataRate)
  {}

  void RegisterCondition(std::shared_ptr<Condition> c)
  {
    conditions.push_back(std::move(c));
  }
  void DeleteCondition(std::shared_ptr<Condition> c)
  {
    auto it = std::find(conditions.begin(), conditions.end(), c);
    if(it != conditions.end()) conditions.erase(it);
  }

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
    return static_cast<float>(frame) / readsPerSecond;
  }

protected:
  std::shared_ptr<InputData> input;
  std::vector<std::shared_ptr<Condition>> conditions;

  uint32_t readsPerSecond = 24;
};

#endif// DECODER_H
