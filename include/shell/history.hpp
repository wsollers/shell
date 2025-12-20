#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <cstddef>
#include <utility>


constexpr size_t HISTORY_DEFAULT_SIZE = 1000;
namespace wshell {

class History {
    using value_type = std::string;

    explicit History(std::size_t max_items = HISTORY_DEFAULT_SIZE)
        : max_items_(max_items), history_(max_items){}

    void set_max(std::size_t new_max) {
        if ( new_max <= 0 ) {
            new_max = HISTORY_DEFAULT_SIZE;
        }
        
        max_items_ = new_max;

        const std::size_t n = std::min(max_items_, history_.size());

        auto last_n_view = history_ | std::views::drop(history_.size() - n);

        std::vector<value_type> tmp;
        tmp.reserve(max_items_);                 // capacity for future growth
        tmp.assign(last_n_view.begin(), last_n_view.end());

        history_.swap(tmp);                  // trims + releases old buffer
    }

    [[nodiscard]] std::size_t max()  const noexcept { return max_items_; }
    [[nodiscard]] std::size_t size() const noexcept { return history_.size(); }
    [[nodiscard]] bool empty() const noexcept { return history_.empty(); }

    [[nodiscard]] const std::vector<value_type>& items() const noexcept { return history_; }

    void push(value_type line) {
        if(history_.capacity() == history_.size()) {
            std::vector<value_type> tmp (max_items_);
            std::move(history_.begin() + 1, history_.end(), tmp.begin());
            history_.swap (tmp);
            history_.push_back(line);
        } else {
            history_.push_back(std::move(line));
        }
    }

private:


    std::vector<value_type> history_;
    std::size_t max_items_{HISTORY_DEFAULT_SIZE};
};

}