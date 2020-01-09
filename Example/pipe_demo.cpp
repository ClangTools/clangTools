#include <Pipe.h>

#define FIFO "FIFO_Test" /*有名管道的名字*/

int main(int argc, char **argv) {
    Pipe pipe(FIFO);
    Pipe pipe1(FIFO);
    pipe.Init(true);
    pipe1.Init(false);
    pipe1.send("1234567");
    std::vector<unsigned char> data;
    pipe.read(data);
    data.push_back(0);
    printf("%s %s\n", __FILENAME__, data.data());
    return 0;
}