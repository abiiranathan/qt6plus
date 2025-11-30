#include <QAction>
#include <QApplication>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>

#include <QMenu>
#include <QStandardItemModel>
#include "EnhancedTreeView.hpp"

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    // Create the custom data structure for the music player
    struct Music {
        QString title;
        QString artist;
        QString album;
        QString filePath;
    };

    // Create a list of music tracks
    QList<Music> musicList;
    musicList.append({
        .title = "Highs and lows",
        .artist = "Artist 1",
        .album = "Album 1",
        .filePath = "/home/nabiizy/Downloads/music.mp3",
    });

    // Create the model and set the custom data
    QStandardItemModel model;
    for (const Music& music : musicList) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(music.title));
        items.append(new QStandardItem(music.artist));
        items.append(new QStandardItem(music.album));
        items.append(new QStandardItem(music.filePath));
        model.appendRow(items);
    }

    // Create the EnhancedTreeView and set the model
    EnhancedTreeView treeView;
    treeView.setModel(&model);
    treeView.setHeaders(QStringList() << "Title"
                                      << "Artist"
                                      << "Album");

    // Create the context menu actions
    auto* playAction = new QAction("Play", &treeView);
    auto* stopAction = new QAction("Stop", &treeView);

    // Create the media player
    QMediaPlayer mediaPlayer;
    auto* audioOutput = new QAudioOutput;
    mediaPlayer.setAudioOutput(audioOutput);

    // Connect the actions to their respective slots
    QObject::connect(playAction, &QAction::triggered, nullptr, [&]() {
        QModelIndex currentIndex = treeView.currentIndex();
        QString filePath = model.item(currentIndex.row(), 3)->text();

        mediaPlayer.setSource(QUrl::fromLocalFile(filePath));
        mediaPlayer.play();
    });

    QObject::connect(stopAction, &QAction::triggered, nullptr, [&]() { mediaPlayer.stop(); });

    // Set the context menu policy to CustomContextMenu
    treeView.setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the customContextMenuRequested signal to show the context menu
    QObject::connect(&treeView, &QWidget::customContextMenuRequested, [&](const QPoint& pos) {
        QMenu contextMenu(&treeView);
        contextMenu.addAction(playAction);
        contextMenu.addAction(stopAction);
        contextMenu.exec(treeView.mapToGlobal(pos));
    });

    treeView.show();

    return a.exec();
}
