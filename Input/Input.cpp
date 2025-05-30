#include "Input.h"

void InputDataStream::ReadFields()
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

float InputDataStream::TryGetFloat(std::string_view name)
{
  CheckFieldName(name);
  return std::stof(tokens[indexes[name.data()]]);
}
double InputDataStream::TryGetDouble(std::string_view name)
{
  CheckFieldName(name);
  return std::stod(tokens[indexes[name.data()]]);
}
std::string_view InputDataStream::TryGetString(std::string_view name)
{
  CheckFieldName(name);
  return tokens[indexes[name.data()]];
}

// Private
void InputDataStream::CheckFieldName(std::string_view name)
{
  if(indexes.count(name.data()) == 0)
    throw std::runtime_error("Incorrect field name");
}

void InputDataStream::Split()
{
  tokens.clear();
  splitStart = 0;

  while((splitEnd = line.find(delimiter, splitStart)) != std::string::npos) {
    token = line.substr(splitStart, splitEnd - splitStart);
    splitStart = splitEnd + delimiter.size();
    tokens.push_back(token);
  }

  if(!line.substr(splitStart).empty())
    tokens.push_back(line.substr(splitStart));
}
