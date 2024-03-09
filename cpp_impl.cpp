#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <iomanip>
#include <openssl/sha.h>

using namespace std;

mutex result_mutex;

string calculateSHA1(const string &input)
{
    SHA_CTX sha1Context;
    SHA1_Init(&sha1Context);
    SHA1_Update(&sha1Context, input.c_str(), input.length());

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1_Final(hash, &sha1Context);

    stringstream sha1Stream;
    sha1Stream << hex << setfill('0');
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        sha1Stream << setw(2) << static_cast<unsigned>(hash[i]);
    }

    return sha1Stream.str();
}

template <typename Function, typename... Args>
void timingDecorator(Function func, Args &&...args)
{
    auto start_time = chrono::high_resolution_clock::now();
    func(forward<Args>(args)...);
    auto end_time = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end_time - start_time);
    cout << "Execution time: " << duration.count() / 1e6 << " seconds" << endl;
}

string mergeHashes(vector<pair<thread::id, string>> &&result_objs)
{
    string result;
    sort(result_objs.begin(), result_objs.end());
    for (const auto &item : result_objs)
    {
        result += item.second;
    }
    return result;
}

void newHashCalculator(int threadId, const string &input_value, queue<pair<thread::id, string>> &result_q)
{
    double counter = 0;
    string last_SHA1;

    while (counter < (1 << 20))
    {
        stringstream result_stream;
        result_stream << threadId << last_SHA1 << counter << input_value;
        string result = result_stream.str();
        last_SHA1 = calculateSHA1(result);
        counter++;
    }

    lock_guard<mutex> lock(result_mutex);
    result_q.push({this_thread::get_id(), move(last_SHA1)});
}

string sha1hash(double thread_counts, const string &input_value)
{
    vector<pair<thread::id, string>> result_objs;
    queue<pair<thread::id, string>> result_queue;

    vector<thread> thread_pool;
    for (double i = 0; i < thread_counts; ++i)
    {
        thread_pool.emplace_back(newHashCalculator, ref(i), ref(input_value), ref(result_queue));
    }

    for (auto &thread : thread_pool)
    {
        thread.join();
    }

    while (!result_queue.empty())
    {
        result_objs.push_back(move(result_queue.front()));
        result_queue.pop();
    }

    return mergeHashes(move(result_objs));
}

int main()
{
    int thread_count=1;
    cout << sha1hash(thread_count, "ali") << endl;
    timingDecorator(sha1hash, thread_count, "ali");
    return 0;
}
