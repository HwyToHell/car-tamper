#define CATCH_CONFIG_RUNNER
#include <catch.hpp>

int main(int argc, char *argv[])
{
    using namespace std;

    int result = Catch::Session().run(argc, argv);

    /*
    cout << "Press <enter> to continue" << endl;
    string str;
    getline(cin, str);
    */
    return result;
}
