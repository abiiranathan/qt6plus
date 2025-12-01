#include <QAction>
#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QSlider>
#include <QStandardItemModel>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include "EnhancedTreeView.hpp"

// Create the custom data structure for the music player
struct Music {
    QString title;
    QString artist;
    QString album;
    QString filePath;
};

int main(int argc, char* argv[]) {  // NOLINT
    QApplication a(argc, argv);

    // Create a list of music tracks
    QList<Music> musicList;
    musicList.append({
        .title = "Highs and lows",
        .artist = "Artist 1",
        .album = "Album 1",
        .filePath = "/home/nabiizy/Downloads/09. Taylor Swift - Wood.mp3",
    });

    // Create the model and set the custom data
    auto* model = new QStandardItemModel(nullptr);
    for (const Music& music : musicList) {
        QList<QStandardItem*> items;
        items.append(new QStandardItem(music.title));
        items.append(new QStandardItem(music.artist));
        items.append(new QStandardItem(music.album));
        items.append(new QStandardItem(music.filePath));
        model->appendRow(items);
    }

    // Create main window widget
    auto* mainWindow = new QWidget(nullptr);
    mainWindow->setWindowTitle("Mini Music Player");
    mainWindow->resize(800, 600);

    // Create the EnhancedTreeView and set the model
    auto* treeView = new EnhancedTreeView(mainWindow);
    treeView->setModel(model);
    treeView->setHeaders(QStringList() << "Title"
                                       << "Artist"
                                       << "Album");

    treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView->hideColumn(3);  // Hide the file path column

    // Create the context menu actions
    auto* playAction = new QAction("Play", treeView);
    auto* pauseAction = new QAction("Pause", treeView);
    auto* stopAction = new QAction("Stop", treeView);
    auto* removeAction = new QAction("Remove from playlist", treeView);

    // Create the media player on the heap with mainWindow as parent
    auto* mediaPlayer = new QMediaPlayer(mainWindow);
    auto* audioOutput = new QAudioOutput(mediaPlayer);
    mediaPlayer->setAudioOutput(audioOutput);

    // Create playback controls
    auto* controlsWidget = new QWidget(mainWindow);
    auto* controlsLayout = new QHBoxLayout(controlsWidget);

    auto* playPauseButton = new QPushButton("▶", controlsWidget);
    playPauseButton->setFixedSize(40, 40);
    auto* stopButton = new QPushButton("■", controlsWidget);
    stopButton->setFixedSize(40, 40);
    auto* prevButton = new QPushButton("⏮", controlsWidget);
    prevButton->setFixedSize(40, 40);
    auto* nextButton = new QPushButton("⏭", controlsWidget);
    nextButton->setFixedSize(40, 40);

    auto* volumeSlider = new QSlider(Qt::Horizontal, controlsWidget);
    volumeSlider->setRange(0, 100);
    volumeSlider->setValue(50);
    volumeSlider->setFixedWidth(100);
    audioOutput->setVolume(0.5);

    auto* volumeLabel = new QLabel("Volume: 50%", controlsWidget);
    volumeLabel->setFixedWidth(80);

    controlsLayout->addWidget(prevButton);
    controlsLayout->addWidget(playPauseButton);
    controlsLayout->addWidget(stopButton);
    controlsLayout->addWidget(nextButton);
    controlsLayout->addStretch();
    controlsLayout->addWidget(volumeLabel);
    controlsLayout->addWidget(volumeSlider);

    // Create progress controls
    auto* progressWidget = new QWidget(mainWindow);
    auto* progressLayout = new QHBoxLayout(progressWidget);

    auto* currentTimeLabel = new QLabel("00:00", progressWidget);
    currentTimeLabel->setFixedWidth(45);
    auto* progressSlider = new QSlider(Qt::Horizontal, progressWidget);
    progressSlider->setRange(0, 0);
    auto* totalTimeLabel = new QLabel("00:00", progressWidget);
    totalTimeLabel->setFixedWidth(45);

    progressLayout->addWidget(currentTimeLabel);
    progressLayout->addWidget(progressSlider);
    progressLayout->addWidget(totalTimeLabel);

    // Create now playing label
    auto* nowPlayingLabel = new QLabel("No track playing", mainWindow);
    nowPlayingLabel->setStyleSheet("QLabel { font-weight: bold; padding: 5px; }");

    // Create toolbar
    auto* toolbarWidget = new QWidget(mainWindow);
    auto* toolbarLayout = new QHBoxLayout(toolbarWidget);
    auto* addFilesButton = new QPushButton("Add Files", toolbarWidget);
    auto* clearPlaylistButton = new QPushButton("Clear Playlist", toolbarWidget);

    toolbarLayout->addWidget(addFilesButton);
    toolbarLayout->addWidget(clearPlaylistButton);
    toolbarLayout->addStretch();

    // Main layout
    auto* mainLayout = new QVBoxLayout(mainWindow);
    mainLayout->addWidget(toolbarWidget);
    mainLayout->addWidget(nowPlayingLabel);
    mainLayout->addWidget(treeView);
    mainLayout->addWidget(progressWidget);
    mainLayout->addWidget(controlsWidget);

    // Helper function to format time
    auto formatTime = [](qint64 milliseconds) -> QString {
        int seconds = static_cast<int>(milliseconds / 1000);
        int minutes = seconds / 60;
        seconds = seconds % 60;
        return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    };

    // Helper function to play a track by index
    auto playTrack = [mediaPlayer, model, nowPlayingLabel, treeView](const QModelIndex& index) {
        if (!index.isValid()) {
            return;
        }

        QStandardItem* filePathItem = model->item(index.row(), 3);
        QStandardItem* titleItem = model->item(index.row(), 0);
        QStandardItem* artistItem = model->item(index.row(), 1);

        if (filePathItem == nullptr || titleItem == nullptr || artistItem == nullptr) {
            return;
        }

        QString filePath = filePathItem->text();
        QString title = titleItem->text();
        QString artist = artistItem->text();

        mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        mediaPlayer->play();

        nowPlayingLabel->setText(QString("Now Playing: %1 - %2").arg(artist).arg(title));
        treeView->setCurrentIndex(index);
    };

    // Connect play/pause button
    QObject::connect(playPauseButton, &QPushButton::clicked, mainWindow,
                     [mediaPlayer, playPauseButton, treeView, playTrack]() {
                         if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
                             mediaPlayer->pause();
                             playPauseButton->setText("▶");
                         } else {
                             if (mediaPlayer->playbackState() == QMediaPlayer::StoppedState) {
                                 // If stopped, play the current selection or first track
                                 QModelIndex currentIndex = treeView->currentIndex();
                                 if (!currentIndex.isValid() && treeView->model()->rowCount() > 0) {
                                     currentIndex = treeView->model()->index(0, 0);
                                 }
                                 playTrack(currentIndex);
                             } else {
                                 mediaPlayer->play();
                             }
                             playPauseButton->setText("⏸");
                         }
                     });

    // Connect stop button
    QObject::connect(stopButton, &QPushButton::clicked, mainWindow,
                     [mediaPlayer, playPauseButton, nowPlayingLabel]() {
                         mediaPlayer->stop();
                         playPauseButton->setText("▶");
                         nowPlayingLabel->setText("Stopped");
                     });

    // Connect previous button
    QObject::connect(prevButton, &QPushButton::clicked, mainWindow, [treeView, playTrack]() {
        QModelIndex currentIndex = treeView->currentIndex();
        if (currentIndex.row() > 0) {
            QModelIndex prevIndex = treeView->model()->index(currentIndex.row() - 1, 0);
            playTrack(prevIndex);
        }
    });

    // Connect next button
    QObject::connect(nextButton, &QPushButton::clicked, mainWindow, [treeView, playTrack, model]() {
        QModelIndex currentIndex = treeView->currentIndex();
        if (currentIndex.row() < model->rowCount() - 1) {
            QModelIndex nextIndex = treeView->model()->index(currentIndex.row() + 1, 0);
            playTrack(nextIndex);
        }
    });

    // Auto-play next track when current finishes
    QObject::connect(
        mediaPlayer, &QMediaPlayer::playbackStateChanged, mainWindow,
        [mediaPlayer, treeView, playTrack, model](QMediaPlayer::PlaybackState state) {
            if (state == QMediaPlayer::StoppedState &&
                mediaPlayer->mediaStatus() == QMediaPlayer::EndOfMedia) {
                QModelIndex currentIndex = treeView->currentIndex();
                if (currentIndex.isValid() && currentIndex.row() < model->rowCount() - 1) {
                    QModelIndex nextIndex = treeView->model()->index(currentIndex.row() + 1, 0);
                    playTrack(nextIndex);
                }
            }
        });

    // Connect volume slider
    QObject::connect(volumeSlider, &QSlider::valueChanged, mainWindow,
                     [audioOutput, volumeLabel](int value) {
                         audioOutput->setVolume((float)value / 100.0F);
                         volumeLabel->setText(QString("Volume: %1%").arg(value));
                     });

    // Connect progress slider for seeking
    QObject::connect(progressSlider, &QSlider::sliderMoved, mainWindow,
                     [mediaPlayer](int position) { mediaPlayer->setPosition(position); });

    // Update progress slider and time labels
    QObject::connect(mediaPlayer, &QMediaPlayer::positionChanged, mainWindow,
                     [progressSlider, currentTimeLabel, formatTime](qint64 position) {
                         if (!progressSlider->isSliderDown()) {
                             progressSlider->setValue(static_cast<int>(position));
                         }
                         currentTimeLabel->setText(formatTime(position));
                     });

    QObject::connect(mediaPlayer, &QMediaPlayer::durationChanged, mainWindow,
                     [progressSlider, totalTimeLabel, formatTime](qint64 duration) {
                         progressSlider->setRange(0, static_cast<int>(duration));
                         totalTimeLabel->setText(formatTime(duration));
                     });

    // Connect double-click to play
    QObject::connect(treeView, &QAbstractItemView::doubleClicked, mainWindow, playTrack);

    // Connect context menu actions
    QObject::connect(playAction, &QAction::triggered, mainWindow,
                     [treeView, playTrack]() { playTrack(treeView->currentIndex()); });

    QObject::connect(pauseAction, &QAction::triggered, mainWindow,
                     [mediaPlayer]() { mediaPlayer->pause(); });

    QObject::connect(stopAction, &QAction::triggered, mainWindow, [mediaPlayer, nowPlayingLabel]() {
        mediaPlayer->stop();
        nowPlayingLabel->setText("Stopped");
    });

    QObject::connect(removeAction, &QAction::triggered, mainWindow, [treeView, model]() {
        QModelIndex currentIndex = treeView->currentIndex();
        if (currentIndex.isValid()) {
            model->removeRow(currentIndex.row());
        }
    });

    // Add files functionality
    QObject::connect(addFilesButton, &QPushButton::clicked, mainWindow, [model, mainWindow]() {
        QStringList files = QFileDialog::getOpenFileNames(
            mainWindow, "Select Music Files", QDir::homePath(),
            "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a);;All Files (*.*)");

        for (const QString& filePath : files) {
            QFileInfo fileInfo(filePath);
            QString title = fileInfo.completeBaseName();
            QString artist = "Unknown Artist";
            QString album = "Unknown Album";

            QList<QStandardItem*> items;
            items.append(new QStandardItem(title));
            items.append(new QStandardItem(artist));
            items.append(new QStandardItem(album));
            items.append(new QStandardItem(filePath));
            model->appendRow(items);
        }
    });

    // Clear playlist functionality
    QObject::connect(clearPlaylistButton, &QPushButton::clicked, mainWindow,
                     [model, mediaPlayer, nowPlayingLabel]() {
                         mediaPlayer->stop();
                         model->removeRows(0, model->rowCount());
                         nowPlayingLabel->setText("Playlist cleared");
                     });

    // Set the context menu policy to CustomContextMenu
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    // Connect the customContextMenuRequested signal to show the context menu
    QObject::connect(
        treeView, &QWidget::customContextMenuRequested, mainWindow,
        [playAction, pauseAction, stopAction, removeAction, treeView](const QPoint& pos) {
            QMenu contextMenu(treeView);
            contextMenu.addAction(playAction);
            contextMenu.addAction(pauseAction);
            contextMenu.addAction(stopAction);
            contextMenu.addSeparator();
            contextMenu.addAction(removeAction);
            contextMenu.exec(treeView->mapToGlobal(pos));
        });

    mainWindow->show();

    return a.exec();
}
