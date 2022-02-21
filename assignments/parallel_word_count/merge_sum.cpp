#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <map>

int main()
{
    std::map<std::string, int> counts;
    int count;
    std::string word;
    while (std::cin >> count >> word)
    {
        counts[word] += count;
    }

    int max = 0;
    std::for_each(counts.begin(),
                  counts.end(),
                  [&max](const auto &pair) {
                      max = std::max(max, pair.second);
                  });

    int ndigits = static_cast<int>(std::ceil(std::log10(max)));

    for (const auto &pair : counts)
    {
        std::cout << "  " << std::setw(ndigits) << pair.second << " ";
        std::cout << pair.first << '\n';
    }

    return 0;
}