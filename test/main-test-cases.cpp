#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/filesystem.hpp>

int main(int argc, char *argv[])
{
    (void)argc;
    using namespace std;
    string cases("[MotionBuffer]");
    //string cases("[MotionBuffer],[TearDown]");

    const int ac = 2; // # of cli arguments for catch app
    const char* av[ac];
    av[0] = argv[0];
    av[1] = cases.c_str();

    int result = Catch::Session().run(ac, av);
    //int result = Catch::Session().run(argc, argv);

    /*
    cout << "Press <enter> to continue" << endl;
    string str;
    getline(cin, str);
    */
    return result;
}
