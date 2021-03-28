#include "select.h"

void Select::add(int fd, callback_t &&callback)
{
    max_fd = std::max(fd, max_fd);
    fd_vals.push_back(std::make_pair(fd, std::move(callback)));
}

void Select::wait()
{
    fd_set rfds;
    FD_ZERO(&rfds);
    for (int i = 0; i < fd_vals.size(); i++)
    {
        int fd = fd_vals[i].first;
        FD_SET(fd, &rfds);
    }
    fprintf(stderr, "start-select: %d\n", max_fd + 1);
    int ret = ::select(max_fd + 1, &rfds, NULL, NULL, NULL);
    if (ret < 0)
    {
        if (errno == EINTR)
        {
            fprintf(stderr, "EINTR\n");
            return;
        }

        perror("select()");
        return;
    }

    fprintf(stderr, "start-select-ret: %d\n", ret);
    for (int i = 0; i < fd_vals.size(); i++)
    {
        int fd = fd_vals[i].first;
        if (FD_ISSET(fd, &rfds))
        {
            fd_vals[i].second();
        }
    }
}