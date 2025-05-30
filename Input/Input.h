#ifndef INPUT_H
#define INPUT_H

#include <type_traits>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <istream>
#include <string>

// Объективно самая слабая часть реализации
// Из-за класса InputData приходится таскать везде этот шаблонный параметр
// D, чтобы указать тип полей
// По хорошему нужна нормальная абстракция, которая позволила бы получать
// поля любого типа :(
//
// С другой стороны - пока у нас есть набор данных одного типа в любом виде
// Всё должно быть ОК
//

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
    // Assuming comments are always fiirst symbol on the line
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

#endif// INPUT_H
