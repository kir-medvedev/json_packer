#include <fstream>
#include "jsonpacker.h"

int main(int argc, char* argv[])
{
    using namespace std;

    if (argc == 3) {
        const string inpFilename(argv[1]);
        const string outFilename(argv[2]);
        ifstream inpStream(inpFilename);

        if (inpStream.is_open()) {
            JsonPacker packer(outFilename);
            string line;
            while (getline(inpStream, line)) {
                if (line.size() > 0)
                    packer << line;
            }
            inpStream.close();
        }
    }

    return EXIT_SUCCESS;
}

