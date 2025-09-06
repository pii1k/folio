#pragma once

#include <chrono>
#include <deque>
#include <functional>

namespace folio::concurrency
{
class JobSystem
{
public:
    using Job = std::function<void()>;

    explicit JobSystem(size_t workers) {}
    ~JobSystem() = default;

    void submit(Job j)
    {
        queue_.emplace_back(std::move(j));
    }

    void drain(double budget_sec = 0.0)
    {
        using clock = std::chrono::steady_clock;
        const auto start = clock::now();

        while (!queue_.empty())
        {
            auto job = std::move(queue_.front());
            queue_.pop_front();
            job();

            if (budget_sec > 0.0)
            {
                const double elapsed = std::chrono::duration<double>(clock::now() - start).count();
                if (elapsed >= budget_sec)
                {
                    break;
                }
            }
        }
    }

    size_t pending() const { return queue_.size(); }

private:
    std::deque<Job> queue_;
};
} // namespace folio::concurrency