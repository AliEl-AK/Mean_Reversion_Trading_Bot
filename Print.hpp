#include <iostream>
#include <vector>
#include <map>

template <typename T>

class Print {
    public:

        static void printVector(const std::string &name, const std::vector<T> &vec)
        {
            std::cout << "Vector name: " << name << std::endl;
            for (const auto &element : vec)
            {
                std::cout << element << std::endl;
            }
        }

        static void printMap(const std::string &name, const std::map<std::string, T> &map)
        {
            std::cout << "Map name: " << name << std::endl;
            for (const auto &pair : map)
            {
                std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
            }
        }

        static void printSpecialMap(const std::string &name, const std::map<std::string, std::vector<std::pair<std::string, float>>> &map)
        {
            std::cout << "Map name: " << name << std::endl;
            for (const auto &pair : map)
            {
                std::cout << "Key: " << pair.first << std::endl;
                for (const auto &innerPair : pair.second)
                {
                    std::cout << "Indicator: " << innerPair.first << ", Average: " << innerPair.second << std::endl;
                }
            }
        }

        static void displayProgressBar(int progress, int total, int width = 20) {
            float percentage = static_cast<float>(progress) / total;
            int barWidth = static_cast<int>(percentage * width);
            std::cout << "[";
            for (int i = 0; i < width; ++i) {
                if (i < barWidth)
                    std::cout << "=";
                else
                    std::cout << " ";
            }
            std::cout << "] " << int(percentage * 100.0) << "%\r";
            std::cout.flush();
            if (progress == total) {
                std::cout << std::endl;
            }
        }
};