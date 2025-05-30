#ifndef INPUT_H
#define INPUT_H

#include <istream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Первая версия с шаблоном мне не нравилась
// Приходилось везде передавать конкретный тип данных
// Такой вариант с несколькими `TryType` методами выглядит чище (на мой взгляд)

class InputData
{
public:
  virtual void ReadFields() = 0;

  virtual bool IsFinished() = 0;

  virtual uint32_t GetCurrentFrame() { return currentFrame; }

  virtual float TryGetFloat(std::string_view name)
  {
    ThrowNotImplemented();
    return {};// To get rid of warnings
  }
  virtual double TryGetDouble(std::string_view name)
  {
    ThrowNotImplemented();
    return {};// To get rid of warnings
  }
  virtual std::string_view TryGetString(std::string_view name)
  {
    ThrowNotImplemented();
    return {};// To get rid of warnings
  }

  virtual ~InputData() = default;

protected:
  uint32_t currentFrame = 0;

private:
  void ThrowNotImplemented()
  {
    throw std::runtime_error("This method is not implemented");
  }
};

class InputDataStream : public InputData
{
public:
  InputDataStream(
      std::shared_ptr<std::basic_istream<char, std::char_traits<char>>> inp,
      std::vector<std::string> ind,
      const std::string& del)
      : input(inp)
      , delimiter(del)
  {
    for(size_t i = 0; i < ind.size(); i++) indexes[ind[i]] = i;
  }

  ~InputDataStream() override;

  void ReadFields() override;

  bool IsFinished() override { return input->eof(); }

  float TryGetFloat(std::string_view name) override;
  double TryGetDouble(std::string_view name) override;
  std::string_view TryGetString(std::string_view name) override;

private:
  const char COMMENT_SYMBOL = '#';

  std::unordered_map<std::string, size_t> indexes;
  std::string line;
  std::shared_ptr<std::basic_istream<char, std::char_traits<char>>> input;

  std::vector<std::string> tokens;
  std::string token;
  std::string delimiter;

  size_t splitStart, splitEnd;

  void CheckFieldName(std::string_view name);

  void Split();
};

#endif// INPUT_H
