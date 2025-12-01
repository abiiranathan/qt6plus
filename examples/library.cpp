// Test installed shared library
#include <QApplication>
#include <iostream>
#include <qt6plus/httpclient.hpp>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    HttpClient client{};
    auto response = client.get_sync("https://google.com");

    if (response.OK) {
        std::cout << response.data.toStdString() << "\n";
    } else {
        std::cout << "Error: " << response.errorString.toStdString() << "\n";
    }
    app.exec();
}

// g++ library.cpp `pkg-config --cflags --libs qt6plus`
