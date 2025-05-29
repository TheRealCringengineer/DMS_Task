#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Начнём с обработки входного файла.
// unoredered_map не очень подходит, потому что хотелось бы итерировать по строчкам
// А хранить массив std::vector<std::unoredered_map> как-будто не хочется
//
// Что-нибудь вроде std::vector<std::vector>> пойдёт
// А вот map заюзаем для полей с их индексами. Норм
//
// Ну и вообще по хорошему должен быть интерфейс на случай, если будет другой формат
// Но полноценный интерфейс разрабатывать пока не хочется, поэтому так

class InputDataStream
{
public:
  InputDataStream(std::basic_istream<char, std::char_traits<char>>* inp,
                  std::vector<std::string> ind)
      : input(inp)
      , delimiter(";")
  {
    for(size_t i = 0; i < ind.size(); i++) indexes[ind[i]] = i;
  }

  void ReadLine()
  {
    std::getline(*input, line);
    while(line[0] == '#') { std::getline(*input, line); }// Skipping comments
    Split();
  }

  float GetField(const std::string& name) { return tokens[indexes[name]]; }

private:
  std::unordered_map<std::string, size_t> indexes;
  std::string line;
  std::basic_istream<char, std::char_traits<char>>* input;

  // Не лучший способ, ну да не суть
  std::vector<float> tokens;
  size_t pos = 0;
  std::string token;
  std::string delimiter;
  void Split()
  {
    tokens.clear();
    while((pos = line.find(delimiter)) != std::string::npos) {
      token = line.substr(0, pos);
      tokens.push_back(std::stof(token));
      line.erase(0, pos + delimiter.length());
    }
    tokens.push_back(std::stof(line));
  }
};

int main()
{
  InputDataStream data(new std::ifstream("perception_results.txt"),
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
                        "Eye2.state.probUnknown"});

  for(size_t i = 0; i < 5; i++) {
    data.ReadLine();

    std::cout << data.GetField("Person.confidence") << std::endl;
  }

  return 0;
}
