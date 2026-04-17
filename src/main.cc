#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <ranges>
#include <vector>

int main(int argc, char *argv[]) {
    // Example of using C++20 features in the plugin code
    std::vector<int> v{1, 2, 3, 4, 5};
    for (int i : v | std::views::filter([](int n) { return n % 2 == 0; })) {
        std::cout << i << " ";
    }
    std::cout << std::endl;

    {
        std::vector<int> v2 = {10, 3, 8, 6, 2, 7, 5, 1, 9, 4};

        std::ranges::nth_element(v2, v2.begin() + 4);
        std::cout << "Index of v2.begin() + 4: " << std::distance(v2.begin(), v2.begin() + 4)
                  << std::endl;
        // std::nth_element(v2.begin(), v2.begin() + 4, v2.end());

        std::cout << "5th smallest element: " << v2[4] << std::endl;
        std::cout << "Vector after nth_element: ";
        std::ranges::copy(v2, std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;

        std::cout << "Sorted vector: ";
        std::ranges::sort(v2);
        std::ranges::copy(v2, std::ostream_iterator<int>(std::cout, " "));
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
