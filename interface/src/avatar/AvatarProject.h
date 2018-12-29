//
//  AvatarProject.h
//
//
//  Created by Thijs Wenker on 12/7/2018
//  Copyright 2018 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#pragma once
#ifndef hifi_AvatarProject_h
#define hifi_AvatarProject_h

#include "MarketplaceItemUploader.h"
#include "ProjectFile.h"
#include "FST.h"

#include <QObject>
#include <QDir>
#include <QVariantHash>
#include <QStandardPaths>

class AvatarProject : public QObject {
    Q_OBJECT
    Q_PROPERTY(FST* fst READ getFST CONSTANT)

    Q_PROPERTY(QStringList projectFiles READ getProjectFiles NOTIFY projectFilesChanged)

    Q_PROPERTY(QString projectFolderPath READ getProjectPath CONSTANT)
    Q_PROPERTY(QString projectFSTPath READ getFSTPath CONSTANT)
    Q_PROPERTY(QString projectFBXPath READ getFBXPath CONSTANT)
    Q_PROPERTY(QString name READ getProjectName WRITE setProjectName NOTIFY nameChanged)

public:
    Q_INVOKABLE MarketplaceItemUploader* upload(bool updateExisting);
    Q_INVOKABLE void openInInventory();
    Q_INVOKABLE QStringList getProjectFiles() const;

    Q_INVOKABLE QString getProjectName() const { return _fst->getName(); }
    Q_INVOKABLE void setProjectName(const QString& newProjectName) {
        if (newProjectName.trimmed().length() > 0) {
            _fst->setName(newProjectName);
            _fst->write();
            emit nameChanged();
        }
    }
    Q_INVOKABLE QString getProjectPath() const { return _projectPath; }
    Q_INVOKABLE QString getFSTPath() const { return _fst->getPath(); }
    Q_INVOKABLE QString getFBXPath() const { return _fst->getModelPath(); }

    /**
     * returns the AvatarProject or a nullptr on failure.
     */
    static AvatarProject* openAvatarProject(const QString& path);
    static AvatarProject* createAvatarProject(const QString& projectsFolder,
                                              const QString& avatarProjectName,
                                              const QString& avatarModelPath,
                                              const QString& textureFolder);

    static bool isValidNewProjectName(const QString& projectPath, const QString& projectName);

    static QString getDefaultProjectsPath() {
        return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/High Fidelity Projects";
    }

signals:
    void nameChanged();
    void projectFilesChanged();

private:
    AvatarProject(const QString& fstPath, const QByteArray& data);
    AvatarProject(FST* fst);

    ~AvatarProject() { _fst->deleteLater(); }

    FST* getFST() { return _fst; }

    void refreshProjectFiles();
    void appendDirectory(QString prefix, QDir dir);
    QStringList getScriptPaths(const QDir& scriptsDir);

    FST* _fst;

    QDir _directory;
    QList<ProjectFilePath> _projectFiles{};
    QString _projectPath;
};

#endif  // hifi_AvatarProject_h
