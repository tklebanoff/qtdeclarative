/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVETYPELOADER_P_H
#define QDECLARATIVETYPELOADER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qobject.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptprogram.h>
#include <QtDeclarative/qdeclarativeerror.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <private/qdeclarativecleanup_p.h>
#include <private/qdeclarativescriptparser_p.h>
#include <private/qdeclarativedirparser_p.h>
#include <private/qdeclarativeimport_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeScriptData;
class QDeclarativeScriptBlob;
class QDeclarativeQmldirData;
class QDeclarativeTypeLoader;
class QDeclarativeCompiledData;
class QDeclarativeComponentPrivate;
class QDeclarativeTypeData;
class QDeclarativeDataLoader;

class Q_AUTOTEST_EXPORT QDeclarativeDataBlob : public QDeclarativeRefCount
{
public:
    enum Status {
        Null,                    // Prior to QDeclarativeDataLoader::load()
        Loading,                 // Prior to data being received and dataReceived() being called
        WaitingForDependencies,  // While there are outstanding addDependency()s
        Complete,                // Finished
        Error                    // Error
    };

    enum Type {
        QmlFile,
        JavaScriptFile,
        QmldirFile
    };

    QDeclarativeDataBlob(const QUrl &, Type);
    virtual ~QDeclarativeDataBlob();

    Type type() const;

    Status status() const;
    bool isNull() const;
    bool isLoading() const;
    bool isWaiting() const;
    bool isComplete() const;
    bool isError() const;
    bool isCompleteOrError() const;

    qreal progress() const;

    QUrl url() const;
    QUrl finalUrl() const;

    QList<QDeclarativeError> errors() const;

    void setError(const QDeclarativeError &);
    void setError(const QList<QDeclarativeError> &errors);

    void addDependency(QDeclarativeDataBlob *);

protected:
    virtual void dataReceived(const QByteArray &) = 0;

    virtual void done();
    virtual void networkError(QNetworkReply::NetworkError);

    virtual void dependencyError(QDeclarativeDataBlob *);
    virtual void dependencyComplete(QDeclarativeDataBlob *);
    virtual void allDependenciesDone();
    
    virtual void downloadProgressChanged(qreal);

private:
    friend class QDeclarativeDataLoader;
    void tryDone();
    void cancelAllWaitingFor();
    void notifyAllWaitingOnMe();
    void notifyComplete(QDeclarativeDataBlob *);

    Type m_type;
    Status m_status;
    qreal m_progress;

    QUrl m_url;
    QUrl m_finalUrl;

    // List of QDeclarativeDataBlob's that are waiting for me to complete.
    QList<QDeclarativeDataBlob *> m_waitingOnMe;

    // List of QDeclarativeDataBlob's that I am waiting for to complete.
    QList<QDeclarativeDataBlob *> m_waitingFor;

    // Manager that is currently fetching data for me
    QDeclarativeDataLoader *m_manager;
    int m_redirectCount:30;
    bool m_inCallback:1;
    bool m_isDone:1;

    QList<QDeclarativeError> m_errors;
};

class Q_AUTOTEST_EXPORT QDeclarativeDataLoader : public QObject
{
    Q_OBJECT
public:
    QDeclarativeDataLoader(QDeclarativeEngine *);
    ~QDeclarativeDataLoader();

    void load(QDeclarativeDataBlob *);
    void loadWithStaticData(QDeclarativeDataBlob *, const QByteArray &);

    QDeclarativeEngine *engine() const;

private slots:
    void networkReplyFinished();
    void networkReplyProgress(qint64,qint64);

private:
    void setData(QDeclarativeDataBlob *, const QByteArray &);

    QDeclarativeEngine *m_engine;
    typedef QHash<QNetworkReply *, QDeclarativeDataBlob *> NetworkReplies;
    NetworkReplies m_networkReplies;
};

class Q_AUTOTEST_EXPORT QDeclarativeTypeLoader : public QDeclarativeDataLoader
{
    Q_OBJECT
public:
    QDeclarativeTypeLoader(QDeclarativeEngine *);
    ~QDeclarativeTypeLoader();

    enum Option {
        None,
        PreserveParser
    };
    Q_DECLARE_FLAGS(Options, Option)

    QDeclarativeTypeData *get(const QUrl &url);
    QDeclarativeTypeData *get(const QByteArray &, const QUrl &url, Options = None);
    void clearCache();

    QDeclarativeScriptBlob *getScript(const QUrl &);
    QDeclarativeQmldirData *getQmldir(const QUrl &);
private:
    typedef QHash<QUrl, QDeclarativeTypeData *> TypeCache;
    typedef QHash<QUrl, QDeclarativeScriptBlob *> ScriptCache;
    typedef QHash<QUrl, QDeclarativeQmldirData *> QmldirCache;

    TypeCache m_typeCache;
    ScriptCache m_scriptCache;
    QmldirCache m_qmldirCache;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeTypeLoader::Options)

class Q_AUTOTEST_EXPORT QDeclarativeTypeData : public QDeclarativeDataBlob
{
public:
    struct TypeReference
    {
        TypeReference() : type(0), majorVersion(0), minorVersion(0), typeData(0) {}

        QDeclarativeParser::Location location;
        QDeclarativeType *type;
        int majorVersion;
        int minorVersion;
        QDeclarativeTypeData *typeData;
    };

    struct ScriptReference
    {
        ScriptReference() : script(0) {}

        QDeclarativeParser::Location location;
        QString qualifier;
        QDeclarativeScriptBlob *script;
    };

    QDeclarativeTypeData(const QUrl &, QDeclarativeTypeLoader::Options, QDeclarativeTypeLoader *);
    ~QDeclarativeTypeData();

    QDeclarativeTypeLoader *typeLoader() const;

    const QDeclarativeImports &imports() const;
    const QDeclarativeScriptParser &parser() const;

    const QList<TypeReference> &resolvedTypes() const;
    const QList<ScriptReference> &resolvedScripts() const;

    QDeclarativeCompiledData *compiledData() const;

    // Used by QDeclarativeComponent to get notifications
    struct TypeDataCallback {
        ~TypeDataCallback() {}
        virtual void typeDataProgress(QDeclarativeTypeData *, qreal) {}
        virtual void typeDataReady(QDeclarativeTypeData *) {}
    };
    void registerCallback(TypeDataCallback *);
    void unregisterCallback(TypeDataCallback *);

protected:
    virtual void done();
    virtual void dataReceived(const QByteArray &);
    virtual void allDependenciesDone();
    virtual void downloadProgressChanged(qreal);

private:
    void resolveTypes();
    void compile();

    QDeclarativeTypeLoader::Options m_options;

    QDeclarativeQmldirData *qmldirForUrl(const QUrl &);

    QDeclarativeScriptParser scriptParser;
    QDeclarativeImports m_imports;

    QList<ScriptReference> m_scripts;
    QList<QDeclarativeQmldirData *> m_qmldirs;

    QList<TypeReference> m_types;
    bool m_typesResolved:1;

    QDeclarativeCompiledData *m_compiledData;

    QList<TypeDataCallback *> m_callbacks;
   
    QDeclarativeTypeLoader *m_typeLoader;
};

class Q_AUTOTEST_EXPORT QDeclarativeScriptData : public QDeclarativeRefCount, public QDeclarativeCleanup
{
public:
    QDeclarativeScriptData(QDeclarativeEngine *);
    ~QDeclarativeScriptData();

    QUrl url;
    QDeclarativeTypeNameCache *importCache;
    QList<QDeclarativeScriptBlob *> scripts;
    QDeclarativeParser::Object::ScriptBlock::Pragmas pragmas;

protected:
    virtual void clear(); // From QDeclarativeCleanup

private:
    friend class QDeclarativeVME;
    friend class QDeclarativeScriptBlob;

    bool m_loaded;
    QScriptProgram m_program;
    QScriptValue m_value;
};

class Q_AUTOTEST_EXPORT QDeclarativeScriptBlob : public QDeclarativeDataBlob
{
public:
    QDeclarativeScriptBlob(const QUrl &, QDeclarativeTypeLoader *);
    ~QDeclarativeScriptBlob();

    struct ScriptReference
    {
        ScriptReference() : script(0) {}

        QDeclarativeParser::Location location;
        QString qualifier;
        QDeclarativeScriptBlob *script;
    };

    QDeclarativeParser::Object::ScriptBlock::Pragmas pragmas() const;
    QString scriptSource() const;

    QDeclarativeTypeLoader *typeLoader() const;
    const QDeclarativeImports &imports() const;

    QDeclarativeScriptData *scriptData() const;

protected:
    virtual void dataReceived(const QByteArray &);
    virtual void done();

private:
    QDeclarativeParser::Object::ScriptBlock::Pragmas m_pragmas;
    QString m_source;

    QDeclarativeImports m_imports;
    QList<ScriptReference> m_scripts;
    QDeclarativeScriptData *m_scriptData;

    QDeclarativeTypeLoader *m_typeLoader;
};

class Q_AUTOTEST_EXPORT QDeclarativeQmldirData : public QDeclarativeDataBlob
{
public:
    QDeclarativeQmldirData(const QUrl &);

    const QDeclarativeDirComponents &dirComponents() const;

protected:
    virtual void dataReceived(const QByteArray &);

private:
    QDeclarativeDirComponents m_components;

};

QT_END_NAMESPACE

#endif // QDECLARATIVETYPELOADER_P_H
