#include <QtCore/QCoreApplication>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    std::cout << "Dummy test passed!" << std::endl;
    return 0; // success
}
