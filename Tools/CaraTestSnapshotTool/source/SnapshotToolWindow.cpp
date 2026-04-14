#include "SnapshotToolWindow.h"
#include "DiffHighlighter.h"
#include "FileHelper.h"

#include <CaraTest/Diff.h>
#include <CaraTest/File.h>

#include <QtCore/QFileSystemWatcher>
#include <QtCore/QStandardPaths>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QDirIterator>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMessageBox>

#include <QtGui/QFileSystemModel>
#include <QtGui/QShowEvent>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dwmapi.h>
#endif

#include <filesystem>

namespace
{
    const auto ToolName = QString("CaraTest Snapshot Tool");
    const auto EmptyFileTitle = QString("...");
    const auto SnapshotFileExtensionMask = QString("*.snapshot");
    const auto ConsolasFont = QFont{ "Consolas", 12, QFont::Normal };
    const auto AcceptColor = QColor("#8AE28A");
    const auto RejectColor = QColor("#F44B56");
    const int MaxRecentDirectories = 10;

#ifdef _WIN32
    static void SetDarkModeTitleBar(const QWidget& window)
    {
        const auto useDarkMode = TRUE;
        DwmSetWindowAttribute(reinterpret_cast<HWND>(window.winId()), DWMWA_USE_IMMERSIVE_DARK_MODE, &useDarkMode, sizeof(useDarkMode));
    }
#else
    static void SetDarkModeTitleBar(const QWidget& window) {}
#endif

    static std::filesystem::path GetOriginFileInfoForSnapshot(const std::filesystem::path& filePath)
    {
        const auto directory = filePath.parent_path();
        const auto completeBaseName = filePath.stem(); // Gets the base name without extension
        const auto originPath = directory / completeBaseName; // Constructs the origin path
        return originPath;
    }

    static std::filesystem::path GetSettingsFilePath()
    {
        const auto appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString();
        std::filesystem::create_directories(appDataDir); // Ensure the directory exists

        const auto filePath = std::filesystem::path(appDataDir) / "snapshot_tool_settings.json";
        return filePath;
    }

    static QStringList LoadRecentDirectories()
    {
        const auto optionalFileContent = CaraTest::File::readContent(GetSettingsFilePath());
        if (!optionalFileContent.has_value())
            return {};

        const auto fileContent = QString::fromStdString(optionalFileContent.value());
        const auto jsonDocument = QJsonDocument::fromJson(fileContent.toUtf8());
        if (!jsonDocument.isObject())
            return {};

        const auto jsonObject = jsonDocument.object();
        const auto jsonValue = jsonObject.value("recentDirectories");
        if (!jsonValue.isArray())
            return {};

        const auto jsonArray = jsonValue.toArray();
        QStringList result;
        for (const auto& value : jsonArray)
        {
            if (value.isString())
                result.append(value.toString());
        }
        return result;
    }

    static void SaveRecentDirectories(const QStringList& dirs)
    {
        QJsonObject jsonObject{};
        QJsonArray jsonArray{};
        for (const auto& dir : dirs)
        {
            jsonArray.append(dir);
        }
        jsonObject["recentDirectories"] = jsonArray;

        QJsonDocument jsonDocument(jsonObject);

        const auto settingsFilePath = GetSettingsFilePath();
        const auto jsonContent = jsonDocument.toJson(QJsonDocument::Indented);
        CaraTest::File::writeContent(settingsFilePath, jsonContent.toStdString());
    }

    static QString GetCurrentRootPath(const QFileSystemModel* fileSystemModel, const QTreeView* snapshotFileTree)
    {
        return fileSystemModel->filePath(snapshotFileTree->rootIndex());
    }

    static bool HasSnapshotFiles(const QString& rootPath)
    {
        if (rootPath.isEmpty() || !QDir(rootPath).exists())
            return false;

        QDirIterator iterator(rootPath, QStringList() << SnapshotFileExtensionMask, QDir::Files, QDirIterator::Subdirectories);
        return iterator.hasNext();
    }
}

void SnapshotToolWindow::showWarningDialog(const QString& message) const
{
    QMessageBox messageBox;
    messageBox.setStyle(style());
    messageBox.setPalette(palette());
    messageBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    messageBox.setText(message);
    messageBox.setIcon(QMessageBox::Information);
    messageBox.setStandardButtons(QMessageBox::Ok);
    SetDarkModeTitleBar(messageBox);

    if (auto* okButton = messageBox.button(QMessageBox::Ok))
        okButton->setMinimumWidth(150);

    messageBox.exec();
}

bool SnapshotToolWindow::showConfirmationDialog(const QString& title, const QString& message) const
{
    QMessageBox messageBox;
    messageBox.setStyle(style());
    messageBox.setPalette(palette());
    messageBox.setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
    messageBox.setWindowTitle(title);
    messageBox.setText(message);
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::No);
    SetDarkModeTitleBar(messageBox);

    if (auto* yesButton = messageBox.button(QMessageBox::Yes))
        yesButton->setMinimumWidth(150);
    if (auto* noButton = messageBox.button(QMessageBox::No))
        noButton->setMinimumWidth(150);

    return messageBox.exec() == QMessageBox::Yes;
}

SnapshotToolWindow::SnapshotToolWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_selectedSnapshotWatcher(new QFileSystemWatcher(this))
    , m_recentMenu(nullptr)
{
    setWindowTitle(ToolName);

    m_menuBar = new QMenuBar(this);
    setMenuBar(m_menuBar);
    m_menuBar->addAction(QString("Select Directory"), this, &SnapshotToolWindow::selectDirectoryToWatch);

    m_recentMenu = new QMenu("Recent Directories", m_menuBar);
    m_menuBar->addMenu(m_recentMenu);

    m_recentDirectories = LoadRecentDirectories();
    rebuildRecentMenu();

    m_snapshotFileTree = new QTreeView(this);
    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    m_fileSystemModel->setNameFilters(QStringList() << SnapshotFileExtensionMask);
    m_fileSystemModel->setNameFilterDisables(false);
    m_snapshotFileTree->setModel(m_fileSystemModel);

    m_snapshotFileTree->setMinimumWidth(220);
    m_snapshotFileTree->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_snapshotFileTree->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    m_snapshotFileTree->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    m_snapshotFileTree->setFocusPolicy(Qt::NoFocus);

    m_snapshotFileTree->header()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
    m_snapshotFileTree->header()->setSectionHidden(1, true);
    m_snapshotFileTree->header()->setSectionHidden(2, true);
    m_snapshotFileTree->header()->setSectionHidden(3, true);
    m_snapshotFileTree->header()->setStretchLastSection(true);

    auto fileTreeWidget = new QWidget(this);
    auto fileTreeLayout = new QVBoxLayout(fileTreeWidget);
    auto fileTreeContentMargins = fileTreeLayout->contentsMargins();
    fileTreeContentMargins.setRight(4);
    fileTreeLayout->setContentsMargins(fileTreeContentMargins);
    fileTreeLayout->addWidget(m_snapshotFileTree);

    m_originFileName = new QLabel(EmptyFileTitle, this);
    m_originFileContent = new QTextEdit(this);
    m_originFileContent->setReadOnly(true);
    m_originFileContent->setFont(ConsolasFont);
    m_originFileContent->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    m_originFileDiffHighlighter = new DiffHighlighter(CaraTest::DiffChange::Deletion, RejectColor, m_originFileContent->document());
    m_rejectSnapshotButton = new QPushButton("Reject", this);
    m_rejectSnapshotButton->setEnabled(false);
    auto rejectStyleSheet = ReadStyleContent(":/styles/rejectbutton.qss");
    m_rejectSnapshotButton->setStyleSheet(rejectStyleSheet);
    m_rejectAllSnapshotsButton = new QPushButton("All", this);
    m_rejectAllSnapshotsButton->setToolTip("Reject all snapshots");
    m_rejectAllSnapshotsButton->setMaximumWidth(48);
    m_rejectAllSnapshotsButton->setFixedHeight(m_rejectSnapshotButton->sizeHint().height());
    m_rejectAllSnapshotsButton->setStyleSheet(rejectStyleSheet);

    auto originWidget = new QWidget(this);
    auto originLayout = new QVBoxLayout(originWidget);
    auto originContentMargins = originLayout->contentsMargins();
    originContentMargins.setLeft(4);
    originContentMargins.setRight(4);
    originLayout->setContentsMargins(originContentMargins);
    originLayout->addWidget(m_originFileName);
    originLayout->addWidget(m_originFileContent);
    auto rejectButtonLayout = new QHBoxLayout();
    rejectButtonLayout->addWidget(m_rejectSnapshotButton);
    rejectButtonLayout->addWidget(m_rejectAllSnapshotsButton);
    originLayout->addLayout(rejectButtonLayout);

    m_snapshotFileName = new QLabel(EmptyFileTitle, this);
    m_snapshotFileContent = new QTextEdit(this);
    m_snapshotFileContent->setReadOnly(true);
    m_snapshotFileContent->setFont(ConsolasFont);
    m_snapshotFileContent->setLineWrapMode(QTextEdit::LineWrapMode::NoWrap);
    m_snapshotFileDiffHighlighter = new DiffHighlighter(CaraTest::DiffChange::Addition, AcceptColor, m_snapshotFileContent->document());
    m_acceptSnapshotButton = new QPushButton("Accept", this);
    m_acceptSnapshotButton->setEnabled(false);
    auto acceptStyleSheet = ReadStyleContent(":/styles/acceptbutton.qss");
    m_acceptSnapshotButton->setStyleSheet(acceptStyleSheet);
    m_acceptAllSnapshotsButton = new QPushButton("All", this);
    m_acceptAllSnapshotsButton->setToolTip("Accept all snapshots");
    m_acceptAllSnapshotsButton->setMaximumWidth(48);
    m_acceptAllSnapshotsButton->setFixedHeight(m_acceptSnapshotButton->sizeHint().height());
    m_acceptAllSnapshotsButton->setStyleSheet(acceptStyleSheet);

    auto snapshotWidget = new QWidget(this);
    auto snapshotLayout = new QVBoxLayout(snapshotWidget);
    auto snapshotContentMargins = snapshotLayout->contentsMargins();
    snapshotContentMargins.setLeft(4);
    snapshotLayout->setContentsMargins(snapshotContentMargins);
    snapshotLayout->addWidget(m_snapshotFileName);
    snapshotLayout->addWidget(m_snapshotFileContent);
    auto acceptButtonLayout = new QHBoxLayout();
    acceptButtonLayout->addWidget(m_acceptSnapshotButton);
    acceptButtonLayout->addWidget(m_acceptAllSnapshotsButton);
    snapshotLayout->addLayout(acceptButtonLayout);

    auto splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    splitter->addWidget(fileTreeWidget);
    splitter->addWidget(originWidget);
    splitter->addWidget(snapshotWidget);
    splitter->setStretchFactor(1, 1);
    splitter->setStretchFactor(2, 1);

    setCentralWidget(splitter);
    resize(1000, 600);

    QObject::connect(m_fileSystemModel, &QFileSystemModel::directoryLoaded, m_snapshotFileTree, &QTreeView::expandAll);
    QObject::connect(m_snapshotFileTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SnapshotToolWindow::onSnapshotClicked);
    QObject::connect(m_snapshotFileTree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SnapshotToolWindow::onUpdateButtonState);
    QObject::connect(m_rejectSnapshotButton, &QPushButton::clicked, this, &SnapshotToolWindow::onRejectClicked);
    QObject::connect(m_rejectAllSnapshotsButton, &QPushButton::clicked, this, &SnapshotToolWindow::onRejectAllClicked);
    QObject::connect(m_acceptSnapshotButton, &QPushButton::clicked, this, &SnapshotToolWindow::onAcceptClicked);
    QObject::connect(m_acceptAllSnapshotsButton, &QPushButton::clicked, this, &SnapshotToolWindow::onAcceptAllClicked);
    QObject::connect(m_selectedSnapshotWatcher, &QFileSystemWatcher::fileChanged, this, &SnapshotToolWindow::onCurrentDisplayedFileChanged);
}

void SnapshotToolWindow::showEvent(QShowEvent* event)
{
    static auto firstShowEvent = true;
    if (!firstShowEvent)
        return;

    firstShowEvent = false;

    if (!m_recentDirectories.isEmpty() && QDir(m_recentDirectories.first()).exists())
    {
        setDirectoryToWatch(m_recentDirectories.first());
    }
    else
    {
        selectDirectoryToWatch();
    }
}

void SnapshotToolWindow::selectDirectoryToWatch()
{
    const auto dirPath = QFileDialog::getExistingDirectory(this, "Select Directory");
    if (dirPath.isNull())
        return;

    setDirectoryToWatch(dirPath);
    addDirectoryToRecent(dirPath);
    SaveRecentDirectories(m_recentDirectories);
}

void SnapshotToolWindow::setDirectoryToWatch(const QString& dirPath)
{
    clearPreviews();
    m_snapshotFileTree->clearSelection();

    m_fileSystemModel->setRootPath(dirPath);
    m_snapshotFileTree->setRootIndex(m_fileSystemModel->index(dirPath));

    const auto windowTitle = QString("%1 - %2").arg(ToolName, dirPath);
    setWindowTitle(windowTitle);

    onUpdateButtonState();
}

void SnapshotToolWindow::clearPreviews()
{
    m_originFileName->setText(EmptyFileTitle);
    m_originFileContent->clear();
    m_originFileDiffHighlighter->clear();
    m_snapshotFileName->setText(EmptyFileTitle);
    m_snapshotFileContent->clear();
    m_snapshotFileDiffHighlighter->clear();

    clearSelectedSnapshotWatcher();
}

void SnapshotToolWindow::clearSelectedSnapshotWatcher()
{
    const auto watchedFiles = m_selectedSnapshotWatcher->files();
    if (!watchedFiles.isEmpty())
        m_selectedSnapshotWatcher->removePaths(watchedFiles);
}

void SnapshotToolWindow::updateHighlighters()
{
    const auto originFileContent = m_originFileContent->toPlainText();
    const auto snapshotFileContent = m_snapshotFileContent->toPlainText();
    const auto diffs = CaraTest::diff(originFileContent.toStdString(), snapshotFileContent.toStdString());

    m_originFileDiffHighlighter->highlightDiffs(diffs);
    m_snapshotFileDiffHighlighter->highlightDiffs(diffs);
}

void SnapshotToolWindow::onSnapshotClicked()
{
    const auto selectedIndexes = m_snapshotFileTree->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    clearPreviews();

    const auto& index = selectedIndexes.first();
    const auto snapshotFileInfo = m_fileSystemModel->fileInfo(index);
    const auto snapshotFilePath = std::filesystem::path(snapshotFileInfo.absoluteFilePath().toStdString());
    const auto isFile = snapshotFileInfo.isFile();
    if (!isFile)
        return;

    const auto originFileInfo = GetOriginFileInfoForSnapshot(snapshotFilePath);
    const auto originExists = std::filesystem::exists(originFileInfo);
    if (originExists)
    {
        const auto originFileName = QString::fromStdString(originFileInfo.filename().string());
        m_originFileName->setText(originFileName);
        const auto originFileContent = QString::fromStdString(CaraTest::File::readContent(originFileInfo).value_or(""));
        m_originFileContent->setText(originFileContent);
        const auto originFilePath = QString::fromStdString(originFileInfo.string());
        m_selectedSnapshotWatcher->addPath(originFilePath);
    }

    const auto snapshotFileName = snapshotFileInfo.fileName();
    m_snapshotFileName->setText(snapshotFileName);
    const auto optionalSnapshotFileContent = CaraTest::File::readContent(snapshotFilePath);
    if (optionalSnapshotFileContent.has_value())
    {
        const auto snapshotFileContent = QString::fromStdString(optionalSnapshotFileContent.value());
        m_snapshotFileContent->setText(snapshotFileContent);
    }
    else
    {
        m_snapshotFileContent->setText("");
    }
    m_selectedSnapshotWatcher->addPath(snapshotFileInfo.absoluteFilePath());

    updateHighlighters();
}

void SnapshotToolWindow::onUpdateButtonState()
{
    const auto selectedIndexes = m_snapshotFileTree->selectionModel()->selectedIndexes();
    const auto isFileSelected = (!selectedIndexes.isEmpty() && m_fileSystemModel->fileInfo(selectedIndexes.first()).isFile());
    const auto rootPath = GetCurrentRootPath(m_fileSystemModel, m_snapshotFileTree);
    const auto hasSnapshotFiles = HasSnapshotFiles(rootPath);

    m_acceptSnapshotButton->setEnabled(isFileSelected);
    m_rejectSnapshotButton->setEnabled(isFileSelected);
    m_acceptAllSnapshotsButton->setEnabled(hasSnapshotFiles);
    m_rejectAllSnapshotsButton->setEnabled(hasSnapshotFiles);
}

void SnapshotToolWindow::onRejectClicked()
{
    const auto selectedIndexes = m_snapshotFileTree->selectionModel()->selectedIndexes();
    const auto snapshotFileInfo = m_fileSystemModel->fileInfo(selectedIndexes.first());
    auto snapshotFile = QFile(snapshotFileInfo.absoluteFilePath());
    if (!snapshotFile.exists())
        return;

    clearSelectedSnapshotWatcher();

    const auto wasRemoved = snapshotFile.remove();
    if (wasRemoved)
        return;

    showWarningDialog(QString("Failed to delete the file: '%1'").arg(snapshotFile.fileName()));
}

void SnapshotToolWindow::onRejectAllClicked()
{
    const auto rootPath = GetCurrentRootPath(m_fileSystemModel, m_snapshotFileTree);
    if (!HasSnapshotFiles(rootPath))
        return;

    if (!showConfirmationDialog("Reject All Snapshots", "Do you really want to reject all snapshot files in the current directory and its subdirectories?"))
        return;

    clearSelectedSnapshotWatcher();

    QDirIterator iterator(rootPath, QStringList() << SnapshotFileExtensionMask, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        QFile snapshotFile(iterator.next());
        if (!snapshotFile.exists())
            continue;

        if (!snapshotFile.remove())
        {
            showWarningDialog(QString("Failed to delete the file: '%1'").arg(snapshotFile.fileName()));
            return;
        }
    }

    clearPreviews();
    onUpdateButtonState();
}

void SnapshotToolWindow::onAcceptClicked()
{
    const auto selectedIndexes = m_snapshotFileTree->selectionModel()->selectedIndexes();
    const auto snapshotFileInfo = m_fileSystemModel->fileInfo(selectedIndexes.first());
    const auto snapshotFilePath = std::filesystem::path(snapshotFileInfo.absoluteFilePath().toStdString());
    const auto snapshotFileName = snapshotFilePath.filename();
    if (!std::filesystem::exists(snapshotFilePath))
        return;

    clearSelectedSnapshotWatcher();

    const auto originFileInfo = GetOriginFileInfoForSnapshot(snapshotFilePath);
    const auto originFileName = originFileInfo.filename();
    const auto originFilePath = originFileInfo;

    if (std::filesystem::exists(originFilePath))
    {
        try
        {
            std::filesystem::remove(originFilePath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            showWarningDialog(QString("Failed to delete the file: '%1'").arg(QString::fromStdString(originFileName.string())));
            return;
        }
    }

    try
    {
        std::filesystem::rename(snapshotFilePath, originFilePath);
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        showWarningDialog(QString("Failed to rename the file: '%1' to '%2'")
            .arg(QString::fromStdString(snapshotFileName.string()), QString::fromStdString(originFileName.string())));
    }
}

void SnapshotToolWindow::onAcceptAllClicked()
{
    const auto rootPath = GetCurrentRootPath(m_fileSystemModel, m_snapshotFileTree);
    if (!HasSnapshotFiles(rootPath))
        return;

    if (!showConfirmationDialog("Accept All Snapshots", "Do you really want to accept all snapshot files in the current directory and its subdirectories?"))
        return;

    clearSelectedSnapshotWatcher();

    QDirIterator iterator(rootPath, QStringList() << SnapshotFileExtensionMask, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext())
    {
        const auto snapshotPathString = iterator.next();
        const auto snapshotFilePath = std::filesystem::path(snapshotPathString.toStdString());
        const auto originFilePath = GetOriginFileInfoForSnapshot(snapshotFilePath);

        if (std::filesystem::exists(originFilePath))
        {
            try
            {
                std::filesystem::remove(originFilePath);
            }
            catch (const std::filesystem::filesystem_error&)
            {
                showWarningDialog(QString("Failed to delete the file: '%1'").arg(QString::fromStdString(originFilePath.filename().string())));
                return;
            }
        }

        try
        {
            std::filesystem::rename(snapshotFilePath, originFilePath);
        }
        catch (const std::filesystem::filesystem_error&)
        {
            showWarningDialog(QString("Failed to rename the file: '%1' to '%2'")
                .arg(QString::fromStdString(snapshotFilePath.filename().string()), QString::fromStdString(originFilePath.filename().string())));
            return;
        }
    }

    clearPreviews();
    onUpdateButtonState();
}

void SnapshotToolWindow::onCurrentDisplayedFileChanged(const QString& filePath)
{
    // we might need to add the file again because some programs delete files before saving
    // https://doc.qt.io/qt-6/qfilesystemwatcher.html#fileChanged

    const auto watchedFiles = m_selectedSnapshotWatcher->files();
    if (!watchedFiles.contains(filePath))
        m_selectedSnapshotWatcher->addPath(filePath);

    const auto changedFilePath = std::filesystem::path(filePath.toStdString());
    const auto fileExtension = changedFilePath.extension().string();
    const auto snapshotExtension = SnapshotFileExtensionMask.mid(1).toStdString(); // remove the leading "*"
    const auto isSnapshot = (fileExtension == snapshotExtension);
    if (isSnapshot)
    {
        const auto optionalSnapshotFileContent = CaraTest::File::readContent(changedFilePath);
        if (optionalSnapshotFileContent.has_value())
        {
            const auto snapshotFileContent = QString::fromStdString(optionalSnapshotFileContent.value());
            m_snapshotFileContent->setText(snapshotFileContent);
        }
        else
        {
            m_snapshotFileContent->setText("");
        }
    }
    else
    {
        const auto optionalOriginFileContent = CaraTest::File::readContent(changedFilePath);
        if (optionalOriginFileContent.has_value())
        {
            const auto originFileContent = QString::fromStdString(optionalOriginFileContent.value());
            m_originFileContent->setText(originFileContent);
        }
        else
        {
            m_originFileContent->setText("");
        }
    }

    updateHighlighters();
}

void SnapshotToolWindow::rebuildRecentMenu()
{
    m_recentMenu->clear();
    for (const auto dir : m_recentDirectories)
    {
        auto action = m_recentMenu->addAction(dir);
        QObject::connect(action, &QAction::triggered, this,
            [this, dir]() {
                if (!QDir(dir).exists())
                {
                    showWarningDialog(QString("Directory does not exist: %1").arg(dir));
                    return;
                }
                setDirectoryToWatch(dir);
                addDirectoryToRecent(dir);
                SaveRecentDirectories(m_recentDirectories);
            });
    }
}

void SnapshotToolWindow::addDirectoryToRecent(const QString& dirPath)
{
    m_recentDirectories.removeAll(dirPath);
    m_recentDirectories.insert(0, dirPath);
    while (m_recentDirectories.size() > MaxRecentDirectories)
    {
        m_recentDirectories.removeLast();
    }

    rebuildRecentMenu();
}
