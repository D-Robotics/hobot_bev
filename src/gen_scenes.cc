// Copyright (c) 2024，D-Robotics.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <map>

void ParseScenes() {
  std::ifstream f1("1.txt");
  std::ifstream f2("2.txt");
  std::ifstream f3("3.txt");

  if (!f1 || !f2 || !f3) {
    printf("open file error\n");
  }

  std::map<std::string, std::string> map1, map2, map3;

  std::string line, contents[4];
  while (getline(f1, line)) {
    std::stringstream ss(line);
    ss >> contents[0] >> contents[1] >> contents[2] >> contents[3];
    map1[contents[1]] = contents[3];
  }
  while (getline(f2, line)) {
    std::stringstream ss(line);
    ss >> contents[0] >> contents[1] >> contents[2] >> contents[3];
    map2[contents[1]] = contents[3];
  }
  while (getline(f3, line)) {
    std::stringstream ss(line);
    ss >> contents[0] >> contents[1] >> contents[2] >> contents[3];
    map3[contents[1]] = contents[3];
  }

  printf(
    "size of map1: %ld, map2: %ld, map3: %ld\n",
    map1.size(), map2.size(), map3.size());

  std::map<std::string, std::string> out_map;
  for (const auto &item : map1) {
    if (map2.find(item.second) != map2.end()) {
      std::string key = map2[item.second];
      if (map3.find(key) != map3.end()) {
        out_map[item.first] = map3[key];
      } else {
        printf(
          "key %s not found in map3\n", key.c_str());
      }
    } else {
      printf(
        "key %s not found in map2\n", item.second.c_str());
    }
  }

  printf( "size of out_map: %ld\n", out_map.size());

  // "95024749968b4c5db7c4830da4d2d475": "boston-seaport",
  std::string out_fname = "scenes.txt";
  std::ofstream ofs(out_fname);
  for (auto &item : out_map) {
    ofs << item.first << ": " << item.second << "," << std::endl;
  }
  printf( "write to %s\n", out_fname.c_str());

}

int main(int argc, char **argv) {
  ParseScenes();
  return 0;
}
