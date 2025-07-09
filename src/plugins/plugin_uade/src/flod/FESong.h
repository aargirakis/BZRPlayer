#ifndef FESong_H
#define FESong_H
#include <vector>

using namespace std;

class FESong
{
    friend class FEPlayer;

public:
    FESong();

private:
    int speed;
    int length;
    vector<vector<int>> tracks;
};

#endif // FESong_H
