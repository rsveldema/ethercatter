#ifndef SELECT__H______
#define SELECT__H______

#include <functional>
#include <vector>

class Select
{
public:
    typedef std::function<void()> callback_t;

    Select()
    {
    }

    void add(int fd, callback_t&& callback);

    void wait();

private:
    std::vector<std::pair<int, callback_t>> fd_vals;
    int max_fd = 0;
};


#endif