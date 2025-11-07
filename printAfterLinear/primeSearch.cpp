#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <algorithm>

class PrimeFinder {
private:
  std::mutex mtx;
  std::atomic<unsigned long long> current_number = 2;
  std::vector<unsigned long long> primes;
  unsigned long long max_range;

  bool isPrime(unsigned long long n) {
    if (n <= 1)
      return false;
    if (n <= 3)
      return true;
    if (n % 2 == 0 || n % 3 == 0)
      return false;

    for (unsigned long long i = 5; i * i <= n && i * i > i; i += 6) {
      if (n % i == 0 || n % (i + 2) == 0) {
        return false;
      }
    }

    return true;
  }

  void worker() {
    unsigned long long number;
    while (true) {
      number = current_number.fetch_add(1);
      
      if (number > max_range) {
        break;
      }
      
      if (isPrime(number)) {
        std::lock_guard<std::mutex> lock(mtx);
        primes.push_back(number);
        // std::cout << "Thread " << std::this_thread::get_id() << " found prime: " << number << std::endl; // comment out to remove live printing
      }

    }
  }

public:
  PrimeFinder(unsigned long long max) : max_range(max) {}

  std::vector<unsigned long long> findPrimes(int num_threads) {
    primes.clear();

    std::vector<std::thread> threads;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_threads; ++i) {
      threads.emplace_back(&PrimeFinder::worker, this);
    }

    for (auto &t : threads) {
      t.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);

    std::sort(primes.begin(), primes.end());

    std::cout << "Found " << primes.size() << " primes in " << duration.count()
              << " ms using " << num_threads << " threads" << std::endl;
    std::cout << "Searched up to: " << max_range << std::endl;

    return primes;
  }
};

void printPrimes(const std::vector<unsigned long long> &primes, int primes_per_line = 10) {
  std::cout << "\nPrime numbers found:\n";
  for (size_t i = 0; i < primes.size(); ++i) {
    std::cout << primes[i];
    
    if ((i + 1) % primes_per_line == 0) {
      std::cout << std::endl;
    } else if (i != primes.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << std::endl;
}

int main() {
    std::string max_range_str;
    std::string line;
    int num_threads;
    int line_number = 0;
    bool max_range_read = false;
    bool num_threads_read = false;

    std::ifstream configFile("config.txt");
    
    if (!configFile.is_open()) {
        std::cout << "Could not open config.txt file." << std::endl;
        return 1;
    }

    while (std::getline(configFile, line)) {
        line_number++;
        
        // Skip empty lines and comments
        if (line.empty()) {
            continue;
        }

        std::istringstream iss(line);
        std::string leftover;
        
        switch (line_number) {
            case 1:
                if (!(iss >> num_threads) || (iss >> leftover)) {
                    std::cout << "Error: Invalid input for threads value in config.txt (line 1)" << std::endl;
                    return 1;
                }
                num_threads_read = true;
                break;
            case 2:
                if (iss >> max_range_str) {
                    max_range_read = true;
                }
                break;
                
            default:
                break;
        }
    }

    configFile.close();
    
    if (!max_range_read) {
        std::cout << "Error: Maximum range not found in config.txt" << std::endl;
        return 1;
    }
    
    if (!num_threads_read) {
        std::cout << "Error: Number of threads not found in config.txt" << std::endl;
        return 1;
    }

    char* endPtr;
    unsigned long long max_range = std::strtoull(max_range_str.c_str(), &endPtr, 10);
    if (*endPtr != '\0') {
        std::cout << "Error: Invalid maximum range number: " << max_range_str << std::endl;
        return 1;
    }

    if (max_range < 2) {
        std::cout << "Maximum range must be at least 2." << std::endl;
        return 1;
    }

    if (num_threads < 1) {
        std::cout << "Number of threads must be at least 1." << std::endl;
        return 1;
    }

    std::cout << "Configuration loaded from config.txt:" << std::endl;
    std::cout << "Number of threads: " << num_threads << std::endl;
    std::cout << "Maximum range: " << max_range << std::endl;
    std::cout << "==================================" << std::endl;

    PrimeFinder finder(max_range);
    auto primes = finder.findPrimes(num_threads);

    printPrimes(primes); // comment out to remove printing after

    return 0;
}